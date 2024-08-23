#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatteryTextSensor : public PytesEBoxListener, public Component {
 public:
  PytesEBoxBatteryTextSensor(int8_t bat_num);

  void dump_config() override;

  SUB_TEXT_SENSOR(base_state)
  SUB_TEXT_SENSOR(voltage_state)
  SUB_TEXT_SENSOR(current_state)
  SUB_TEXT_SENSOR(temperature_state)
  SUB_TEXT_SENSOR(barcode)
  SUB_TEXT_SENSOR(dev_type)
  SUB_TEXT_SENSOR(firm_version)
  SUB_TEXT_SENSOR(coulomb_status)
  SUB_TEXT_SENSOR(bat_status)
  SUB_TEXT_SENSOR(cmos_status)
  SUB_TEXT_SENSOR(dmos_status)
  SUB_TEXT_SENSOR(bat_protect_ena)
  SUB_TEXT_SENSOR(pwr_protect_ena)
  SUB_TEXT_SENSOR(bat_events)
  SUB_TEXT_SENSOR(power_events)
  SUB_TEXT_SENSOR(system_fault)


  virtual void on_pwr_line_read(pwr_LineContents *line);
  virtual void on_pwrn_line_read(pwr_data_LineContents *line);
  virtual void on_batn_line_read(bat_index_LineContents *line);


 protected:
  int8_t bat_num_;

};

}  // namespace pytes_e_box
}  // namespace esphome
