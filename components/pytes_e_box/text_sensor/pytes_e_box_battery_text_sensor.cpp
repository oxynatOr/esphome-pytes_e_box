#include "pytes_e_box_battery_text_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pytes_e_box {

PytesEBoxBatteryTextSensor::PytesEBoxBatteryTextSensor(int bat_num) { this->bat_num_ = bat_num; }

void PytesEBoxBatteryTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox Battery Text Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);

  LOG_TEXT_SENSOR("  ", "Base state", this->base_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Voltage state", this->voltage_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Current state", this->current_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Temperature state", this->temperature_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Barcode", this->barcode_text_sensor_);
  LOG_TEXT_SENSOR("  ", "DevType", this->dev_type_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Firm Version", this->firm_version_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Coulomb Status", this->coulomb_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Bat Status", this->bat_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "CMOS Status", this->cmos_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "DMOS Status", this->dmos_status_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Bat Protect ENA", this->bat_protect_ena_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Pwr Protect ENA", this->pwr_protect_ena_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Bat Events", this->bat_events_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Power Events", this->power_events_text_sensor_);
  LOG_TEXT_SENSOR("  ", "System Fault", this->system_fault_text_sensor_);
}

void PytesEBoxBatteryTextSensor::on_battery_data(const BmsBatteryData *line) {
  if (this->bat_num_ != line->bat_num) {
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

  if (this->barcode_text_sensor_ != nullptr && !line->barcode.empty()) {
    this->barcode_text_sensor_->publish_state(line->barcode);
  }
  if (this->dev_type_text_sensor_ != nullptr && !line->dev_type.empty()) {
    this->dev_type_text_sensor_->publish_state(line->dev_type);
  }
  if (this->firm_version_text_sensor_ != nullptr && !line->firm_version.empty()) {
    this->firm_version_text_sensor_->publish_state(line->firm_version);
  }
  if (this->coulomb_status_text_sensor_ != nullptr && !line->coulomb_status.empty()) {
    this->coulomb_status_text_sensor_->publish_state(line->coulomb_status);
  }
  if (this->bat_status_text_sensor_ != nullptr && !line->bat_status.empty()) {
    this->bat_status_text_sensor_->publish_state(line->bat_status);
  }
  if (this->cmos_status_text_sensor_ != nullptr && !line->cmos_status.empty()) {
    this->cmos_status_text_sensor_->publish_state(line->cmos_status);
  }
  if (this->dmos_status_text_sensor_ != nullptr && !line->dmos_status.empty()) {
    this->dmos_status_text_sensor_->publish_state(line->dmos_status);
  }
  if (this->bat_protect_ena_text_sensor_ != nullptr && !line->bat_protect_ena.empty()) {
    this->bat_protect_ena_text_sensor_->publish_state(line->bat_protect_ena);
  }
  if (this->pwr_protect_ena_text_sensor_ != nullptr && !line->pwr_protect_ena.empty()) {
    this->pwr_protect_ena_text_sensor_->publish_state(line->pwr_protect_ena);
  }
  if (this->bat_events_text_sensor_ != nullptr && !line->bat_events.empty()) {
    this->bat_events_text_sensor_->publish_state(line->bat_events);
  }
  if (this->power_events_text_sensor_ != nullptr && !line->power_events.empty()) {
    this->power_events_text_sensor_->publish_state(line->power_events);
  }
  if (this->system_fault_text_sensor_ != nullptr && !line->system_fault.empty()) {
    this->system_fault_text_sensor_->publish_state(line->system_fault);
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
