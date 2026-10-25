#include "pytes_e_box_battery_cell_text_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pytes_e_box {

PytesEBoxBatteryCellTextSensor::PytesEBoxBatteryCellTextSensor(int bat_num, int cell_num) {
  this->bat_num_ = bat_num;
  this->cell_num_ = cell_num;
}

void PytesEBoxBatteryCellTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox BatteryCell Text Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);
  ESP_LOGCONFIG(TAG, " Cell %d", this->cell_num_);

  LOG_TEXT_SENSOR("  ", "Cell Base State", this->base_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Cell Voltage State", this->voltage_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Cell Current State", this->current_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Cell Temperature State", this->temperature_state_text_sensor_);
}

void PytesEBoxBatteryCellTextSensor::on_cell_data(const BmsCellData *line) {
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->cell_num_ != line->cell_num) {
    return;
  }

  if (this->base_state_text_sensor_ != nullptr && !line->base_state.empty()) {
    this->base_state_text_sensor_->publish_state(line->base_state);
  }
  if (this->voltage_state_text_sensor_ != nullptr && !line->voltage_state.empty()) {
    this->voltage_state_text_sensor_->publish_state(line->voltage_state);
  }
  if (this->current_state_text_sensor_ != nullptr && !line->current_state.empty()) {
    this->current_state_text_sensor_->publish_state(line->current_state);
  }
  if (this->temperature_state_text_sensor_ != nullptr && !line->temperature_state.empty()) {
    this->temperature_state_text_sensor_->publish_state(line->temperature_state);
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
