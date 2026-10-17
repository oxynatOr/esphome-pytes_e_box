#include "pytes_e_box.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include <algorithm>
#include <cinttypes>
#include <cmath>

namespace esphome {
namespace pytes_e_box {

PytesEBoxComponent::PytesEBoxComponent() {}

void PytesEBoxComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox:");
  ESP_LOGCONFIG(TAG, "  Batteries: %d", this->batteries_in_system_);
  ESP_LOGCONFIG(TAG, "  Poll Timeout: %" PRIu32 " ms", this->polling_timeout_);
  ESP_LOGCONFIG(TAG, "  Command Idle Time: %" PRIu32 " ms", this->command_idle_time_);
  ESP_LOGCONFIG(TAG, "  Max Retries: %u", this->max_retries_);
  ESP_LOGCONFIG(TAG, "  Commands in Queue: %u", (unsigned) this->cmd_queue_.size());
  LOG_UPDATE_INTERVAL(this);
  LOG_BINARY_SENSOR("  ", "Connected", this->connected_binary_sensor_);
  LOG_SENSOR("  ", "Error Count", this->error_count_sensor_);
  LOG_SENSOR("  ", "Total Current", this->total_current_sensor_);
  LOG_SENSOR("  ", "Total Power", this->total_power_sensor_);
  LOG_SENSOR("  ", "Average SOC", this->average_soc_sensor_);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Connection with PytesEBox failed!");
  }
}

void PytesEBoxComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PytesEBox...");
  this->state_ = STATE_WAIT;
  this->clear_uart_buffer();
  this->reset_aggregates_();

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
  this->driver_->build_commands(this->cmd_queue_, this->batteries_in_system_);

  if (this->error_count_sensor_ != nullptr) {
    this->error_count_sensor_->publish_state(0);
  }

  if (this->is_ready()) {
    this->state_ = STATE_IDLE;
    ESP_LOGCONFIG(TAG, "Device is ready to receive commands");
  }
}

void PytesEBoxComponent::write_command_() {
  this->buffer_index_read_ = 0;
  this->buffer_index_write_ = 0;
  this->clear_uart_buffer();
  this->driver_->reset();
  this->write_str(this->cmd_queue_[this->command_queue_position_].command.c_str());
  this->write_str("\n");
  this->last_poll_ = millis();
  this->rx_bytes_ = 0;
  this->state_ = STATE_POLL;
}

void PytesEBoxComponent::send_command_again_() {
  this->command_retries_++;
  ESP_LOGI(TAG, "Retrying command '%s' (%" PRIu32 "/%u)",
           this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_retries_,
           this->max_retries_);
  this->write_command_();
}

bool PytesEBoxComponent::send_next_command_() {
  if (this->command_queue_position_ >= this->cmd_queue_.size() ||
      this->cmd_queue_[this->command_queue_position_].command.empty()) {
    return false;
  }
  this->command_retries_ = 0;
  this->write_command_();
  ESP_LOGD(TAG, "Sending command from queue: %s from index: %d",
           this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_queue_position_);
  return true;
}

void PytesEBoxComponent::retry_or_advance_() {
  if (this->command_retries_ < this->max_retries_) {
    this->send_command_again_();
    return;
  }
  if (this->max_retries_ > 0) {
    ESP_LOGW(TAG, "Giving up on command '%s' after %" PRIu32 " retries",
             this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_retries_);
  }
  this->clear_uart_buffer();
  this->last_poll_ = millis();
  this->state_ = STATE_SEND_NEXT_COMMAND;
}

void PytesEBoxComponent::record_error_() {
  this->errors_++;
  if (this->error_count_sensor_ != nullptr) {
    this->error_count_sensor_->publish_state(this->errors_);
  }
}

void PytesEBoxComponent::mark_success_() {
  if (!this->cycle_ok_ && this->connected_binary_sensor_ != nullptr) {
    this->connected_binary_sensor_->publish_state(true);
  }
  this->cycle_ok_ = true;
}

void PytesEBoxComponent::reset_aggregates_() {
  std::fill(std::begin(this->agg_voltage_), std::end(this->agg_voltage_), NAN);
  std::fill(std::begin(this->agg_current_), std::end(this->agg_current_), NAN);
  std::fill(std::begin(this->agg_soc_), std::end(this->agg_soc_), NAN);
}

void PytesEBoxComponent::finish_cycle_() {
  if (this->connected_binary_sensor_ != nullptr) {
    this->connected_binary_sensor_->publish_state(this->cycle_ok_);
  }

  float current = 0, power = 0, soc = 0;
  int n_current = 0, n_power = 0, n_soc = 0;
  for (int i = 1; i <= MAX_BATTERIES; i++) {
    if (!std::isnan(this->agg_current_[i])) {
      current += this->agg_current_[i] / 1000.0f;
      n_current++;
      if (!std::isnan(this->agg_voltage_[i])) {
        power += (this->agg_voltage_[i] / 1000.0f) * (this->agg_current_[i] / 1000.0f);
        n_power++;
      }
    }
    if (!std::isnan(this->agg_soc_[i])) {
      soc += this->agg_soc_[i];
      n_soc++;
    }
  }
  if (this->total_current_sensor_ != nullptr && n_current > 0) {
    this->total_current_sensor_->publish_state(current);
  }
  if (this->total_power_sensor_ != nullptr && n_power > 0) {
    this->total_power_sensor_->publish_state(power);
  }
  if (this->average_soc_sensor_ != nullptr && n_soc > 0) {
    this->average_soc_sensor_->publish_state(soc / n_soc);
  }
  this->reset_aggregates_();
  this->cycle_ok_ = false;
}

void PytesEBoxComponent::emit_(BmsEmit emit, int index) {
  if (emit == BmsEmit::BATTERY) {
    const BmsBatteryData *data = &this->driver_->battery_data();
    if (data->bat_num >= 1 && data->bat_num <= MAX_BATTERIES) {
      if (!std::isnan(data->voltage))
        this->agg_voltage_[data->bat_num] = data->voltage;
      if (!std::isnan(data->current))
        this->agg_current_[data->bat_num] = data->current;
      if (!std::isnan(data->coulomb))
        this->agg_soc_[data->bat_num] = data->coulomb;
    }
    this->mark_success_();
    for (BmsListener *listener : this->listeners_) {
      listener->on_battery_data(data);
    }
  } else if (emit == BmsEmit::CELL) {
    const BmsCellData *data = &this->driver_->cell_data();
    for (BmsListener *listener : this->listeners_) {
      listener->on_cell_data(data);
    }
  } else if (emit == BmsEmit::CELL_BATCH_END) {
    for (BmsListener *listener : this->listeners_) {
      listener->on_cell_batch_complete(index);
    }
  }
}

void PytesEBoxComponent::update() {
  // Streaming drivers (e.g. Pylontech): fire the command once and let
  // loop_streaming_() parse whatever arrives, line by line.
  if (this->driver_ != nullptr && this->driver_->is_streaming()) {
    if (this->cycle_started_) {
      this->finish_cycle_();
    }
    this->cycle_started_ = true;
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

  if (this->state_ != STATE_IDLE) {
    ESP_LOGD(TAG, "Previous polling cycle still running, skipping update");
    return;
  }
  this->command_queue_position_ = 0;
  this->cycle_ok_ = false;
  if (!this->send_next_command_()) {
    ESP_LOGE(TAG, "Command queue is empty");
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
    this->emit_(emit, 0);
    this->buffer_[this->buffer_index_read_].clear();
    this->buffer_index_read_ = (this->buffer_index_read_ + 1) % NUM_BUFFERS;
  }
}

int PytesEBoxComponent::readline_(int readch) {
  if (readch <= 0) {
    return -1;
  }
  switch (readch) {
    case '\n':
      break;
    case '\r': {
      int len = this->line_pos_;
      this->line_pos_ = 0;
      return len;
    }
    default:
      if (this->line_pos_ < (int) sizeof(this->line_buf_) - 1) {
        this->line_buf_[this->line_pos_++] = (char) readch;
        this->line_buf_[this->line_pos_] = 0;
      }
  }
  return -1;
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

  /** response did not arrive in time */
  if (this->state_ != STATE_SEND_NEXT_COMMAND && millis() - this->last_poll_ > this->polling_timeout_) {
    const char *command = this->cmd_queue_[this->command_queue_position_].command.c_str();
    uint32_t elapsed = millis() - this->last_poll_;
    this->record_error_();
    if (this->rx_bytes_ == 0) {
      ESP_LOGW(TAG, "Timeout on command '%s' after %" PRIu32 " ms: no bytes received - check RX wiring/pin/GND",
               command, elapsed);
    } else {
      ESP_LOGW(TAG, "Timeout on command '%s' after %" PRIu32 " ms: %" PRIu32 " bytes received but no terminator",
               command, elapsed, this->rx_bytes_);
    }
    this->retry_or_advance_();
    return;
  }

  /** command queue */
  if (this->state_ == STATE_SEND_NEXT_COMMAND) {
    if (millis() - this->last_poll_ <= this->command_idle_time_) {
      return;
    }

    this->command_queue_position_++;
    if (this->command_queue_position_ >= this->cmd_queue_.size()) {
      this->state_ = STATE_IDLE;
      ESP_LOGI(TAG, "PytesEBox command queue done.");
      this->finish_cycle_();
      return;
    }
    if (!this->send_next_command_()) {
      ESP_LOGE(TAG, "Command failed: %s", this->cmd_queue_[this->command_queue_position_].command.c_str());
    }
    return;
  }

  /** full response received: emit accumulated data (if any) */
  if (this->state_ == STATE_COMMAND_COMPLETE) {
    int index = this->cmd_queue_[this->command_queue_position_].index;
    BmsEmit emit = this->driver_->command_complete(this->last_cmd_, index);
    this->emit_(emit, index);
    this->mark_success_();
    this->state_ = STATE_SEND_NEXT_COMMAND;
    ESP_LOGVV(TAG, "Command Complete, switch to STATE_SEND_NEXT_COMMAND");
    return;
  }

  /** parse one response line and emit per-line data */
  if (this->state_ == STATE_COMMAND) {
    if (this->buffer_index_read_ != this->buffer_index_write_) {
      if (this->last_cmd_ == CMD_NIL || this->last_cmd_ == CMD_ERROR) {
        this->state_ = STATE_SEND_NEXT_COMMAND;
        return;
      }
      int index = this->cmd_queue_[this->command_queue_position_].index;
      BmsEmit emit = this->driver_->parse_line(this->last_cmd_, this->buffer_[this->buffer_index_read_], index);
      this->emit_(emit, index);

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
        this->record_error_();
        this->retry_or_advance_();
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
    while (this->available()) {
      int c = this->read();
      this->rx_bytes_++;
      if (this->readline_(c) > 0) {
        this->buffer_[this->buffer_index_write_] = this->line_buf_;
        ESP_LOGV(TAG, "(%d) %s", this->buffer_index_write_, this->line_buf_);
        if (this->driver_->is_line_complete(this->buffer_[this->buffer_index_write_]) > 0) {
          this->state_ = STATE_POLL_COMPLETE;
          ESP_LOGVV(TAG, "Command Complete, switch to STATE_POLL_COMPLETE");
        }
        this->buffer_index_write_ = (this->buffer_index_write_ + 1) % NUM_BUFFERS;
      }
    }
  }
}

void PytesEBoxComponent::clear_uart_buffer() {
  uint8_t byte;
  this->flush();
  while (this->available()) {
    this->read_byte(&byte);
  }
  this->line_pos_ = 0;
}

float PytesEBoxComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pytes_e_box
}  // namespace esphome
