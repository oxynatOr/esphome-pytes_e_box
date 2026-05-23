#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatteryCellSensor : public BmsListener, public Component {
 public:
  PytesEBoxBatteryCellSensor(int bat_num, int cell_num);
  void dump_config() override;

  SUB_SENSOR(voltage)
  SUB_SENSOR(current)
  SUB_SENSOR(temperature)
  SUB_SENSOR(coulomb)

  void on_cell_data(const BmsCellData *data) override;

 protected:
  int bat_num_;
  int cell_num_;
};

}  // namespace pytes_e_box
}  // namespace esphome