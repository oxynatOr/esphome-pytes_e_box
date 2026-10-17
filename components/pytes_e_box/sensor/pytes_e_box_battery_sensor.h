#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatterySensor : public BmsListener, public Component {
 public:
  PytesEBoxBatterySensor(int bat_num);
  void dump_config() override;
  SUB_SENSOR(voltage)
  SUB_SENSOR(current)
  SUB_SENSOR(temperature)
  SUB_SENSOR(temperature_low)
  SUB_SENSOR(temperature_high)
  SUB_SENSOR(mos_temperature)
  SUB_SENSOR(voltage_low)
  SUB_SENSOR(voltage_high)
  SUB_SENSOR(coulomb)
  SUB_SENSOR(soc_voltage)
  SUB_SENSOR(total_coulomb)
  SUB_SENSOR(real_coulomb)
  SUB_SENSOR(total_power_in)
  SUB_SENSOR(total_power_out)
  SUB_SENSOR(work_status)
  SUB_SENSOR(cell_count)
  SUB_SENSOR(power)
  SUB_SENSOR(cell_voltage_min)
  SUB_SENSOR(cell_voltage_max)
  SUB_SENSOR(cell_voltage_delta)

  void on_battery_data(const BmsBatteryData *data) override;
  void on_cell_data(const BmsCellData *data) override;
  void on_cell_batch_complete(int bat_num) override;

 protected:
  int bat_num_;
  float cell_min_mv_{NAN};
  float cell_max_mv_{NAN};
};

}  // namespace pytes_e_box
}  // namespace esphome
