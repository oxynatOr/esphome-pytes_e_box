#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"

#include <memory>
#include <string>
#include <vector>

#include "bms_data.h"
#include "bms_driver.h"
#include "pytes_driver.h"
#include "pytes_lv1_driver.h"
#include "example_driver.h"
#include "pylontech_driver.h"

namespace esphome {
namespace pytes_e_box {
static const char *const TAG = "pytes_e_box";
static const uint8_t NUM_BUFFERS = 128;

// Receives normalized data from the active BMS driver. Sensors implement this.
class BmsListener {
 public:
  virtual void on_battery_data(const BmsBatteryData *data) {}
  virtual void on_cell_data(const BmsCellData *data) {}
  virtual void dump_config() {}
};

class PytesEBoxComponent : public PollingComponent, public uart::UARTDevice {
 public:
  PytesEBoxComponent();

  void update() override;
  void loop() override;
  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override;

  int readline(int readch, char *buffer, int len) {
    static int pos = 0;
    int rpos;
    if (readch > 0) {
      switch (readch) {
        case '\n':  // Ignore new-lines
          break;
        case '\r':  // Return on CR
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len - 1) {
            buffer[pos++] = readch;
            buffer[pos] = 0;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  void register_listener(BmsListener *listener) { this->listeners_.push_back(listener); }

  void set_cmd_idle_time(uint32_t cmd_idle_time) { this->command_idle_time_ = cmd_idle_time; }
  void set_polling_timeout(uint32_t poll_timeout) { this->polling_timeout_ = poll_timeout; }
  void set_system_battery_count(int8_t num_bats) { this->battaries_in_system_ = num_bats; }
  void set_driver_type(BmsType type) { this->driver_type_ = type; }

  void clear_uart_buffer();

 protected:
  static const size_t COMMAND_QUEUE_LENGTH = 34;  // 16 boxes with 16 batteries and one index.
  int battaries_in_system_;
  uint32_t polling_timeout_;
  uint32_t command_idle_time_;
  uint32_t last_poll_ = 0;
  uint32_t command_retries_ = 0;

  std::string buffer_[NUM_BUFFERS];
  int buffer_index_write_ = 0;
  int buffer_index_read_ = 0;

  uint8_t state_ = 254;
  enum State {
    STATE_WAIT = 254,
    STATE_IDLE = 0,
    STATE_POLL = 1,
    STATE_COMMAND = 2,
    STATE_POLL_COMPLETE = 3,
    STATE_COMMAND_COMPLETE = 4,
    STATE_POLL_CHECKED = 5,
    STATE_POLL_DECODED = 6,
    STATE_SEND_NEXT_COMMAND = 7,
  };

  BmsType driver_type_{BmsType::PYTES_E_BOX};
  std::unique_ptr<BmsDriver> driver_;
  BmsCommandId last_cmd_ = CMD_NIL;

  std::vector<PollingCommand> cmd_queue_{};
  uint8_t command_queue_position_ = 0;
  uint8_t command_queue_max_ = 0;
  uint8_t send_next_command_();
  uint8_t send_command_again();

  void loop_streaming_();

  void emit_(BmsEmit emit);

  std::vector<BmsListener *> listeners_{};
};

}  // namespace pytes_e_box
}  // namespace esphome
