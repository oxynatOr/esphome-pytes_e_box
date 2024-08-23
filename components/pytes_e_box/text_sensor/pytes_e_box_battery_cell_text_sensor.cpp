#include "pytes_e_box_battery_cell_text_sensor.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace pytes_e_box {

//static const char *const TAG_BTS = "pytes_e_box.textsensor";

PytesEBoxBatteryCellTextSensor::PytesEBoxBatteryCellTextSensor(int8_t bat_num, int8_t cell_num) { this->bat_num_ = bat_num; this->cell_num_ = cell_num; }

void PytesEBoxBatteryCellTextSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "PytesEBox BatteryCell Text Sensor:");
  ESP_LOGCONFIG(TAG, " Battery %d", this->bat_num_);
  ESP_LOGCONFIG(TAG, " Cell %d", this->cell_num_);
  
  LOG_TEXT_SENSOR("  ","Cell Base State", this->base_state_text_sensor_);
  LOG_TEXT_SENSOR("  ","Cell Voltage State", this->voltage_state_text_sensor_);
  LOG_TEXT_SENSOR("  ","Cell Current State", this->current_state_text_sensor_);
  LOG_TEXT_SENSOR("  ","Cell Temperature State", this->temperature_state_text_sensor_);

}


void PytesEBoxBatteryCellTextSensor::on_pwrn_line_read(pwr_data_LineContents *line) { 
  return;
}
void PytesEBoxBatteryCellTextSensor::on_batn_line_read(bat_index_LineContents *line) { 
  if (this->bat_num_ != line->bat_num) {
    return;    
  }
  if (this->cell_num_ != line->cell_num) {
    return;    
  }  

   
  if (this->base_state_text_sensor_ != nullptr) {
  base_state_text_sensor_->publish_state(std::string(line->cell_baseState));
  }

  if (this->voltage_state_text_sensor_ != nullptr) {
  voltage_state_text_sensor_->publish_state(std::string(line->cell_voltState));
  }
  
  if (this->current_state_text_sensor_ != nullptr) {
  current_state_text_sensor_->publish_state(std::string(line->cell_currState));
  }

  if (this->temperature_state_text_sensor_ != nullptr) {
  temperature_state_text_sensor_->publish_state(std::string(line->cell_tempState));
  }

}


void PytesEBoxBatteryCellTextSensor::on_pwr_line_read(pwr_LineContents *line) {
    return;
}

}  // namespace pytes_e_box
}  // namespace esphome
