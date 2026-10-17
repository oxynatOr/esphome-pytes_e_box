#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

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
static const uint8_t MAX_BATTERIES = 16;
static const int MAX_DATA_LINE_LENGTH = 256;

// Receives normalized data from the active BMS driver. Sensors implement this.
class BmsListener {
 public:
  virtual void on_battery_data(const BmsBatteryData *data) {}
  virtual void on_cell_data(const BmsCellData *data) {}
  virtual void on_cell_batch_complete(int bat_num) {}
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

  void register_listener(BmsListener *listener) { this->listeners_.push_back(listener); }

  void set_cmd_idle_time(uint32_t cmd_idle_time) { this->command_idle_time_ = cmd_idle_time; }
  void set_polling_timeout(uint32_t poll_timeout) { this->polling_timeout_ = poll_timeout; }
  void set_system_battery_count(int8_t num_bats) { this->batteries_in_system_ = num_bats; }
  void set_driver_type(BmsType type) { this->driver_type_ = type; }
  void set_max_retries(uint8_t max_retries) { this->max_retries_ = max_retries; }

  SUB_BINARY_SENSOR(connected)
  SUB_SENSOR(error_count)
  SUB_SENSOR(total_current)
  SUB_SENSOR(total_power)
  SUB_SENSOR(average_soc)

  void clear_uart_buffer();

 protected:
  int batteries_in_system_;
  uint32_t polling_timeout_;
  uint32_t command_idle_time_;
  uint8_t max_retries_{2};
  uint32_t last_poll_ = 0;
  uint32_t command_retries_ = 0;

  std::string buffer_[NUM_BUFFERS];
  int buffer_index_write_ = 0;
  int buffer_index_read_ = 0;

  char line_buf_[MAX_DATA_LINE_LENGTH]{};
  int line_pos_ = 0;
  int readline_(int readch);

  uint32_t rx_bytes_ = 0;
  uint32_t errors_ = 0;
  bool cycle_ok_ = false;
  bool cycle_started_ = false;
  float agg_voltage_[MAX_BATTERIES + 1];
  float agg_current_[MAX_BATTERIES + 1];
  float agg_soc_[MAX_BATTERIES + 1];

  void record_error_();
  void mark_success_();
  void finish_cycle_();
  void reset_aggregates_();

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
  void write_command_();
  bool send_next_command_();
  void send_command_again_();
  // Retries the current command if allowed, otherwise moves on to the next one.
  void retry_or_advance_();

  void loop_streaming_();

  void emit_(BmsEmit emit, int index);

  std::vector<BmsListener *> listeners_{};
};

}  // namespace pytes_e_box
}  // namespace esphome
