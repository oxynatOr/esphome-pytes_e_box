#include "pytes_e_box_battery_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cmath>

namespace esphome {
namespace pytes_e_box {

PytesEBoxBatterySensor::PytesEBoxBatterySensor(int bat_num) { this->bat_num_ = bat_num; }

void PytesEBoxBatterySensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox Battery Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);

  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Current", this->current_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Temperature low", this->temperature_low_sensor_);
  LOG_SENSOR("  ", "Temperature high", this->temperature_high_sensor_);
  LOG_SENSOR("  ", "MOS Temperature", this->mos_temperature_sensor_);
  LOG_SENSOR("  ", "Voltage low", this->voltage_low_sensor_);
  LOG_SENSOR("  ", "Voltage high", this->voltage_high_sensor_);
  LOG_SENSOR("  ", "Coulomb", this->coulomb_sensor_);

  LOG_SENSOR("  ", "SOC Voltage", this->soc_voltage_sensor_);
  LOG_SENSOR("  ", "Total Coulomb", this->total_coulomb_sensor_);
  LOG_SENSOR("  ", "Real Coulomb", this->real_coulomb_sensor_);
  LOG_SENSOR("  ", "Total Power In", this->total_power_in_sensor_);
  LOG_SENSOR("  ", "Total Power Out", this->total_power_out_sensor_);
  LOG_SENSOR("  ", "Work Status", this->work_status_sensor_);
  LOG_SENSOR("  ", "Cell Count", this->cell_count_sensor_);
  LOG_SENSOR("  ", "Power", this->power_sensor_);
  LOG_SENSOR("  ", "Cell Voltage Min", this->cell_voltage_min_sensor_);
  LOG_SENSOR("  ", "Cell Voltage Max", this->cell_voltage_max_sensor_);
  LOG_SENSOR("  ", "Cell Voltage Delta", this->cell_voltage_delta_sensor_);
}

void PytesEBoxBatterySensor::on_battery_data(const BmsBatteryData *line) {
  if (this->bat_num_ != line->bat_num) {
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
  if (this->temperature_low_sensor_ != nullptr && !std::isnan(line->temperature_low)) {
    this->temperature_low_sensor_->publish_state(line->temperature_low / 1000.0f);
  }
  if (this->temperature_high_sensor_ != nullptr && !std::isnan(line->temperature_high)) {
    this->temperature_high_sensor_->publish_state(line->temperature_high / 1000.0f);
  }
  if (this->mos_temperature_sensor_ != nullptr && !std::isnan(line->mos_temperature)) {
    this->mos_temperature_sensor_->publish_state(line->mos_temperature / 1000.0f);
  }
  if (this->voltage_low_sensor_ != nullptr && !std::isnan(line->voltage_low)) {
    this->voltage_low_sensor_->publish_state(line->voltage_low / 1000.0f);
  }
  if (this->voltage_high_sensor_ != nullptr && !std::isnan(line->voltage_high)) {
    this->voltage_high_sensor_->publish_state(line->voltage_high / 1000.0f);
  }
  if (this->coulomb_sensor_ != nullptr && !std::isnan(line->coulomb)) {
    this->coulomb_sensor_->publish_state(line->coulomb);
  }

  if (this->soc_voltage_sensor_ != nullptr && !std::isnan(line->soc_voltage)) {
    this->soc_voltage_sensor_->publish_state(line->soc_voltage / 1000.0f);
  }
  if (this->total_coulomb_sensor_ != nullptr && !std::isnan(line->total_coulomb)) {
    this->total_coulomb_sensor_->publish_state(line->total_coulomb / 3600000.0f);
  }
  if (this->real_coulomb_sensor_ != nullptr && !std::isnan(line->real_coulomb)) {
    this->real_coulomb_sensor_->publish_state(line->real_coulomb / 1000.0f);
  }
  if (this->total_power_in_sensor_ != nullptr && !std::isnan(line->total_power_in)) {
    this->total_power_in_sensor_->publish_state(line->total_power_in / 3600.0f);
  }
  if (this->total_power_out_sensor_ != nullptr && !std::isnan(line->total_power_out)) {
    this->total_power_out_sensor_->publish_state(line->total_power_out / 3600.0f);
  }
  if (this->work_status_sensor_ != nullptr && !std::isnan(line->work_status)) {
    this->work_status_sensor_->publish_state(line->work_status);
  }
  if (this->cell_count_sensor_ != nullptr && !std::isnan(line->cell_count)) {
    this->cell_count_sensor_->publish_state(line->cell_count);
  }
  if (this->power_sensor_ != nullptr && !std::isnan(line->voltage) && !std::isnan(line->current)) {
    this->power_sensor_->publish_state((line->voltage / 1000.0f) * (line->current / 1000.0f));
  }
}

void PytesEBoxBatterySensor::on_cell_data(const BmsCellData *line) {
  if (this->bat_num_ != line->bat_num || std::isnan(line->voltage)) {
    return;
  }
  if (std::isnan(this->cell_min_mv_) || line->voltage < this->cell_min_mv_) {
    this->cell_min_mv_ = line->voltage;
  }
  if (std::isnan(this->cell_max_mv_) || line->voltage > this->cell_max_mv_) {
    this->cell_max_mv_ = line->voltage;
  }
}

void PytesEBoxBatterySensor::on_cell_batch_complete(int bat_num) {
  if (this->bat_num_ != bat_num) {
    return;
  }
  if (!std::isnan(this->cell_min_mv_) && !std::isnan(this->cell_max_mv_)) {
    if (this->cell_voltage_min_sensor_ != nullptr) {
      this->cell_voltage_min_sensor_->publish_state(this->cell_min_mv_ / 1000.0f);
    }
    if (this->cell_voltage_max_sensor_ != nullptr) {
      this->cell_voltage_max_sensor_->publish_state(this->cell_max_mv_ / 1000.0f);
    }
    if (this->cell_voltage_delta_sensor_ != nullptr) {
      this->cell_voltage_delta_sensor_->publish_state((this->cell_max_mv_ - this->cell_min_mv_) / 1000.0f);
    }
  }
  this->cell_min_mv_ = NAN;
  this->cell_max_mv_ = NAN;
}

}  // namespace pytes_e_box
}  // namespace esphome
