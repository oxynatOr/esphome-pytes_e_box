#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatteryTextSensor : public BmsListener, public Component {
 public:
  PytesEBoxBatteryTextSensor(int bat_num);

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

  void on_battery_data(const BmsBatteryData *data) override;

 protected:
  int bat_num_;

};

}  // namespace pytes_e_box
}  // namespace esphome
