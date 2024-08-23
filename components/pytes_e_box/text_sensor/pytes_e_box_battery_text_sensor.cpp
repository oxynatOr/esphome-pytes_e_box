#include "pytes_e_box_battery_text_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pytes_e_box {

//static const char *const TAG_BTS = "pytes_e_box.textsensor";

PytesEBoxBatteryTextSensor::PytesEBoxBatteryTextSensor(int8_t bat_num) { this->bat_num_ = bat_num; }

void PytesEBoxBatteryTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox Battery Text Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);

  LOG_TEXT_SENSOR("  ", "Base state", this->base_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Voltage state", this->voltage_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Current state", this->current_state_text_sensor_);
  LOG_TEXT_SENSOR("  ", "Temperature state", this->temperature_state_text_sensor_);
  LOG_TEXT_SENSOR("  ","Barcode", this->barcode_text_sensor_);
  LOG_TEXT_SENSOR("  ","DevType", this->dev_type_text_sensor_);
  LOG_TEXT_SENSOR("  ","Firm Version", this->firm_version_text_sensor_);
  LOG_TEXT_SENSOR("  ","Coulomb Status", this->coulomb_status_text_sensor_);
  LOG_TEXT_SENSOR("  ","Bat Status", this->bat_status_text_sensor_);
  LOG_TEXT_SENSOR("  ","CMOS Status", this->cmos_status_text_sensor_);
  LOG_TEXT_SENSOR("  ","DMOS Status", this->dmos_status_text_sensor_);
  LOG_TEXT_SENSOR("  ","Bat Protect ENA", this->bat_protect_ena_text_sensor_);
  LOG_TEXT_SENSOR("  ","Pwr Protect ENA", this->pwr_protect_ena_text_sensor_);
  LOG_TEXT_SENSOR("  ","Bat Events", this->bat_events_text_sensor_);
  LOG_TEXT_SENSOR("  ","Power Events", this->power_events_text_sensor_);
  LOG_TEXT_SENSOR("  ","System Fault", this->system_fault_text_sensor_);

}


void PytesEBoxBatteryTextSensor::on_pwrn_line_read(pwr_data_LineContents *line) { 
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->barcode_text_sensor_ != nullptr) {
    this->barcode_text_sensor_->publish_state(std::string(line->Barcode));
  }

  if (this->dev_type_text_sensor_ != nullptr) {
    this->dev_type_text_sensor_->publish_state(std::string(line->DevType));
  }

  if (this->firm_version_text_sensor_ != nullptr) {
    this->firm_version_text_sensor_->publish_state(std::string(line->FirmVersion));
  }

  if (this->coulomb_status_text_sensor_ != nullptr) {
    this->coulomb_status_text_sensor_->publish_state(std::string(line->CoulStatus));
  }

  if (this->bat_status_text_sensor_ != nullptr) {
    this->bat_status_text_sensor_->publish_state(std::string(line->BatStatus));
  }

  if (this->cmos_status_text_sensor_ != nullptr) {
    this->cmos_status_text_sensor_->publish_state(std::string(line->CMOSStatus));
  }

  if (this->dmos_status_text_sensor_ != nullptr) {
    this->dmos_status_text_sensor_->publish_state(std::string(line->DMOSStatus));
  }

  if (this->bat_protect_ena_text_sensor_ != nullptr) {
    this->bat_protect_ena_text_sensor_->publish_state(std::string(line->BatProtectENA));
  }

  if (this->pwr_protect_ena_text_sensor_ != nullptr) {
    this->pwr_protect_ena_text_sensor_->publish_state(std::string(line->PwrProtectENA));
  }

  if (this->bat_events_text_sensor_ != nullptr) {
    this->bat_events_text_sensor_->publish_state(std::string(line->BatEvents));
  }

  if (this->power_events_text_sensor_ != nullptr) {
    this->power_events_text_sensor_->publish_state(std::string(line->PowerEvents));
  }

  if (this->system_fault_text_sensor_ != nullptr) {
    this->system_fault_text_sensor_->publish_state(std::string(line->SystemFault));
  }

}
void PytesEBoxBatteryTextSensor::on_batn_line_read(bat_index_LineContents *line) {   
  return;    
}


void PytesEBoxBatteryTextSensor::on_pwr_line_read(pwr_LineContents *line) {
  if (this->bat_num_ != line->bat_num) {
    return;
  }
  if (this->base_state_text_sensor_ != nullptr) {
    this->base_state_text_sensor_->publish_state(std::string(line->base_st));
  }
  if (this->voltage_state_text_sensor_ != nullptr) {
    this->voltage_state_text_sensor_->publish_state(std::string(line->volt_st));
  }
  if (this->current_state_text_sensor_ != nullptr) {
    this->current_state_text_sensor_->publish_state(std::string(line->curr_st));
  }
  if (this->temperature_state_text_sensor_ != nullptr) {
    this->temperature_state_text_sensor_->publish_state(std::string(line->temp_st));
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
