#include "pytes_e_box.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include <algorithm>

namespace esphome {
namespace pytes_e_box {

static const int MAX_DATA_LENGTH_BYTES = 4096;
static const int MAX_DATA_LINE_LENGTH = 256;

PytesEBoxComponent::PytesEBoxComponent() {}

void PytesEBoxComponent::dump_config() {
  this->check_uart_settings(115200, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  ESP_LOGCONFIG(TAG, "PytesEBox:");
  ESP_LOGCONFIG(TAG, "  Batteries: %d", this->battaries_in_system_);
  ESP_LOGCONFIG(TAG, "  Poll Timeout: %d", this->polling_timeout_);
  ESP_LOGCONFIG(TAG, "  Commands in Queue: %d", this->cmd_queue_.size());

  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Connection with PytesEBox failed!");
  }
}

void PytesEBoxComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PytesEBox...");
  this->state_ = STATE_WAIT;
  this->clear_uart_buffer();
  this->last_poll_ = -1;

  switch (this->driver_type_) {
    case BmsType::EXAMPLE:
      this->driver_ = std::unique_ptr<BmsDriver>(new ExampleDriver());
      break;
    case BmsType::PYLONTECH:
      this->driver_ = std::unique_ptr<BmsDriver>(new PylontechDriver());
      break;
    case BmsType::PYTES_LV1:
      this->driver_ = std::unique_ptr<BmsDriver>(new PytesLv1Driver());
      break;
    case BmsType::PYTES_E_BOX:
    default:
      this->driver_ = std::unique_ptr<BmsDriver>(new PytesDriver());
      break;
  }
  this->driver_->build_commands(this->cmd_queue_, this->battaries_in_system_);
  this->command_queue_max_ = this->cmd_queue_.size();

  if (this->is_ready()) {
    this->state_ = STATE_IDLE;
    ESP_LOGCONFIG(TAG, "Device is ready to receive commands");
  }
}

uint8_t PytesEBoxComponent::send_command_again() {
  if (this->state_ == STATE_WAIT) {
    this->buffer_index_read_ = 0;
    this->buffer_index_write_ = 0;
    this->clear_uart_buffer();
    this->write_str(this->cmd_queue_[this->command_queue_position_].command.c_str());
    this->write_str("\n");
    this->state_ = STATE_POLL;
    ESP_LOGI(TAG, "Retrying command '%s' from index: %d, retry count: %d, at time: %lu ms",
             this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_queue_position_,
             this->command_retries_, millis());
    this->command_retries_++;
    return 1;
  }
  return 0;
}

uint8_t PytesEBoxComponent::send_next_command_() {
  if (this->cmd_queue_[this->command_queue_position_].command != "") {
    this->buffer_index_read_ = 0;
    this->buffer_index_write_ = 0;
    this->command_retries_ = 0;
    this->clear_uart_buffer();
    if (this->driver_ != nullptr) {
      this->driver_->reset();
    }
    this->write_str(this->cmd_queue_[this->command_queue_position_].command.c_str());
    this->write_str("\n");
    this->last_poll_ = millis();
    this->state_ = STATE_POLL;
    ESP_LOGD(TAG, "Sending command from queue: %s from index: %d",
             this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_queue_position_);
    return 1;
  }
  return 0;
}

void PytesEBoxComponent::emit_(BmsEmit emit) {
  if (emit == BmsEmit::BATTERY) {
    const BmsBatteryData *data = &this->driver_->battery_data();
    for (BmsListener *listener : this->listeners_) {
      listener->on_battery_data(data);
    }
  } else if (emit == BmsEmit::CELL) {
    const BmsCellData *data = &this->driver_->cell_data();
    for (BmsListener *listener : this->listeners_) {
      listener->on_cell_data(data);
    }
  }
}

void PytesEBoxComponent::update() {
  // Streaming drivers (e.g. Pylontech): fire the command once and let
  // loop_streaming_() parse whatever arrives, line by line.
  if (this->driver_ != nullptr && this->driver_->is_streaming()) {
    this->buffer_index_read_ = 0;
    this->buffer_index_write_ = 0;
    for (auto &b : this->buffer_) {
      b.clear();
    }
    this->clear_uart_buffer();
    this->driver_->reset();
    if (!this->cmd_queue_.empty()) {
      this->write_str(this->cmd_queue_[0].command.c_str());
      this->write_str("\n");
    }
    return;
  }

  if (this->state_ == STATE_IDLE) {
    this->command_queue_position_ = 0;
    if (this->send_next_command_() == 0) {
      ESP_LOGE(TAG, "Command failed: %s", this->cmd_queue_[this->command_queue_position_].command.c_str());
      return;
    }
  }
}

void PytesEBoxComponent::loop_streaming_() {
  // Port of the upstream Pylontech read loop: collect bytes into the line ring
  // buffer (split on LF), and when the UART is drained, parse exactly one queued
  // line per loop() so we never block the component.
  size_t avail = this->available();
  if (avail > 0) {
    uint8_t buf[64];
    while (avail > 0) {
      size_t n = std::min(avail, sizeof(buf));
      if (!this->read_array(buf, n)) {
        break;
      }
      avail -= n;
      for (size_t i = 0; i < n; i++) {
        char c = (char) buf[i];
        this->buffer_[this->buffer_index_write_] += c;
        if (c == '\n' || this->buffer_[this->buffer_index_write_].length() >= (size_t) MAX_DATA_LINE_LENGTH) {
          this->buffer_index_write_ = (this->buffer_index_write_ + 1) % NUM_BUFFERS;
          this->buffer_[this->buffer_index_write_].clear();
        }
      }
    }
  } else if (this->buffer_index_read_ != this->buffer_index_write_) {
    BmsCommandId cmd = this->cmd_queue_.empty() ? CMD_NIL : this->cmd_queue_[0].identifier;
    BmsEmit emit = this->driver_->parse_line(cmd, this->buffer_[this->buffer_index_read_], 0);
    this->emit_(emit);
    this->buffer_[this->buffer_index_read_].clear();
    this->buffer_index_read_ = (this->buffer_index_read_ + 1) % NUM_BUFFERS;
  }
}

/* only 1-line per run, otherwise we will block the component. */
void PytesEBoxComponent::loop() {
  if (this->driver_ != nullptr && this->driver_->is_streaming()) {
    this->loop_streaming_();
    return;
  }

  /** nothing to do, keep chilling */
  if (this->state_ == STATE_IDLE || this->state_ == STATE_WAIT) {
    return;
  }

  /** check if we run into a "deathloop" or something is blocking */
  if (millis() - this->last_poll_ > this->polling_timeout_) {
    unsigned long elapsed = millis() - this->last_poll_;
    this->last_poll_ = millis();
    ESP_LOGE(TAG, "Timeout on command '%s': retry %d, elapsed %lu ms.",
             this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_retries_, elapsed);

    this->clear_uart_buffer();
    this->state_ = STATE_SEND_NEXT_COMMAND;
    if (this->command_queue_position_ == this->command_queue_max_) {
      this->state_ = STATE_IDLE;
    }
    return;
  }

  /** command queue */
  if (this->state_ == STATE_SEND_NEXT_COMMAND) {
    if (millis() - this->last_poll_ <= this->command_idle_time_) {
      return;
    }

    this->command_queue_position_ = (this->command_queue_position_ + 1) % COMMAND_QUEUE_LENGTH;
    if (this->command_queue_position_ == this->command_queue_max_) {
      this->state_ = STATE_IDLE;
      ESP_LOGI(TAG, "PytesEBox command queue done.");
      return;
    }
    if (this->send_next_command_() == 0) {
      ESP_LOGE(TAG, "Command failed: %s", this->cmd_queue_[this->command_queue_position_].command.c_str());
      return;
    }
  }

  /** full response received: emit accumulated data (if any) */
  if (this->state_ == STATE_COMMAND_COMPLETE) {
    BmsEmit emit = this->driver_->command_complete(this->last_cmd_, this->cmd_queue_[this->command_queue_position_].index);
    this->emit_(emit);
    this->state_ = STATE_SEND_NEXT_COMMAND;
    ESP_LOGVV(TAG, "Command Complete, switch to STATE_SEND_NEXT_COMMAND");
    if (this->command_queue_position_ == this->command_queue_max_) {
      this->state_ = STATE_IDLE;
      ESP_LOGVV(TAG, "Command Complete, switch to STATE_IDLE");
    }
    return;
  }

  /** parse one response line and emit per-line data */
  if (this->state_ == STATE_COMMAND) {
    if (this->buffer_index_read_ != this->buffer_index_write_) {
      if (this->last_cmd_ == CMD_NIL || this->last_cmd_ == CMD_ERROR) {
        this->state_ = STATE_SEND_NEXT_COMMAND;
        return;
      }
      BmsEmit emit = this->driver_->parse_line(this->last_cmd_, this->buffer_[this->buffer_index_read_],
                                               this->cmd_queue_[this->command_queue_position_].index);
      this->emit_(emit);

      if (this->driver_->is_line_complete(this->buffer_[this->buffer_index_read_]) > 0) {
        ESP_LOGVV(TAG, "Command Complete, switch to STATE_COMMAND_COMPLETE");
        this->state_ = STATE_COMMAND_COMPLETE;
        return;
      }
      this->buffer_index_read_ = (this->buffer_index_read_ + 1) % NUM_BUFFERS;
      return;
    }
  }

  /** optional per-line preprocessing before parsing */
  if (this->state_ == STATE_POLL_DECODED) {
    if (this->buffer_index_read_ != this->buffer_index_write_) {
      if (this->last_cmd_ == CMD_NIL || this->last_cmd_ == CMD_ERROR) {
        this->state_ = STATE_WAIT;
        this->send_command_again();
        return;
      }
      if (!this->driver_->needs_line_preprocess(this->last_cmd_)) {
        this->state_ = STATE_COMMAND;
        ESP_LOGVV(TAG, "switch to STATE_COMMAND");
        return;
      }
      this->driver_->preprocess_line(this->last_cmd_, this->buffer_[this->buffer_index_read_]);
      if (this->driver_->is_line_complete(this->buffer_[this->buffer_index_read_]) > 0) {
        this->state_ = STATE_COMMAND;
        this->buffer_index_read_ = (3) % NUM_BUFFERS;
        ESP_LOGVV(TAG, "switch to STATE_COMMAND");
        return;
      }
      this->buffer_index_read_ = (this->buffer_index_read_ + 1) % NUM_BUFFERS;
      return;
    }
  }

  /** identify the command we sent from the echoed prompt */
  if (this->state_ == STATE_POLL_COMPLETE) {
    this->last_cmd_ = this->driver_->read_command(this->buffer_[0]);
    this->state_ = STATE_POLL_DECODED;
    this->buffer_[0].clear();
    this->buffer_[1].clear();
    this->buffer_[2].clear();
    this->buffer_index_read_ = (3) % NUM_BUFFERS;
    ESP_LOGD(TAG, "parsed command -> %s [Battery: %i]", this->driver_->command_to_string(this->last_cmd_).c_str(),
             this->cmd_queue_[this->command_queue_position_].index);
    ESP_LOGVV(TAG, "switch to STATE_POLL_DECODED");
    return;
  }

  /** pull all the serial data from the buffer */
  if (this->state_ == STATE_POLL) {
    if (this->available() < 0) {
      this->state_ = STATE_WAIT;
      this->send_command_again();
      return;
    }
    while (this->available()) {
      static char buffer[MAX_DATA_LINE_LENGTH];
      if (readline(read(), buffer, MAX_DATA_LENGTH_BYTES) > 0) {
        this->buffer_[buffer_index_write_] = buffer;
        ESP_LOGV(TAG, "(%d) %s", this->buffer_index_write_, buffer);
        if (this->driver_->is_line_complete(this->buffer_[buffer_index_write_]) > 0) {
          this->state_ = STATE_POLL_COMPLETE;
          ESP_LOGVV(TAG, "Command Complete, switch to STATE_POLL_COMPLETE");
        }
        this->buffer_index_write_ = (this->buffer_index_write_ + 1) % NUM_BUFFERS;
      }  // readLine
    }  // while available
  }  /** Read UART Buffer End */
}

void PytesEBoxComponent::clear_uart_buffer() {
  uint8_t byte;
  this->flush();
  while (this->available()) {
    this->read_byte(&byte);
  }
}

float PytesEBoxComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pytes_e_box
}  // namespace esphome
