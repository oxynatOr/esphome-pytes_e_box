#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatteryCellTextSensor : public BmsListener, public Component {
 public:
  PytesEBoxBatteryCellTextSensor(int bat_num,int cell_num); 

  void dump_config() override;

  SUB_TEXT_SENSOR(base_state)
  SUB_TEXT_SENSOR(voltage_state)
  SUB_TEXT_SENSOR(current_state)
  SUB_TEXT_SENSOR(temperature_state)

  void on_cell_data(const BmsCellData *data) override;

 protected:
  int  bat_num_;
  int  cell_num_;
};

}  // namespace pytes_e_box
}  // namespace esphome
