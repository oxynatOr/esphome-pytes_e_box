#pragma once

#include "../pytes_e_box.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace pytes_e_box {

class PytesEBoxBatteryCellTextSensor : public PytesEBoxListener, public Component {
 public:
  PytesEBoxBatteryCellTextSensor(int8_t bat_num,int8_t cell_num); 

  void dump_config() override;

  SUB_TEXT_SENSOR(base_state)
  SUB_TEXT_SENSOR(voltage_state)
  SUB_TEXT_SENSOR(current_state)
  SUB_TEXT_SENSOR(temperature_state)

  virtual void on_pwr_line_read(pwr_LineContents *line);
  virtual void on_pwrn_line_read(pwr_data_LineContents *line);
  virtual void on_batn_line_read(bat_index_LineContents *line);

 protected:
  int8_t bat_num_;
  int8_t cell_num_;
};

}  // namespace pytes_e_box
}  // namespace esphome
