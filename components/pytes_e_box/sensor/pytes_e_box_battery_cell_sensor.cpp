#include "pytes_e_box_battery_cell_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome {
namespace pytes_e_box {

PytesEBoxBatteryCellSensor::PytesEBoxBatteryCellSensor(int bat_num, int cell_num) {
  this->bat_num_ = bat_num;
  this->cell_num_ = cell_num;
}

void PytesEBoxBatteryCellSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox Battery Cell Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);
  ESP_LOGCONFIG(TAG, " Cell %d", this->cell_num_);
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Coulomb", this->coulomb_sensor_);
}

void PytesEBoxBatteryCellSensor::on_cell_data(const BmsCellData *line) {
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->cell_num_ != line->cell_num) {
    return;
  }

  if (this->voltage_sensor_ != nullptr && !std::isnan(line->voltage)) {
    this->voltage_sensor_->publish_state(line->voltage / 1000.0f);
  }
  if (this->current_sensor_ != nullptr && !std::isnan(line->current)) {
    this->current_sensor_->publish_state(line->current / 1000.0f);
  }
  if (this->temperature_sensor_ != nullptr && !std::isnan(line->temperature)) {
    this->temperature_sensor_->publish_state(line->temperature / 1000.0f);
  }
  if (this->coulomb_sensor_ != nullptr && !std::isnan(line->coulomb)) {
    this->coulomb_sensor_->publish_state(line->coulomb);
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
