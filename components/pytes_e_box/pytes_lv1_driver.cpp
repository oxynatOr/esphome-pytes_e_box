#include "pytes_lv1_driver.h"
#include "esphome/core/log.h"

#include <cctype>
#include <cstdio>

namespace esphome {
namespace pytes_e_box {

static const char *const TAG = "pytes_e_box";
static const size_t LV1_STR_LEN = 60;

// LV1 cell line: cell_num cell_volt cell_curr cell_tempr base volt curr temp coulomb%
// (current is the 3rd column, between voltage and temperature).
bool PytesLv1Driver::parse_bat_index_(const std::string &line, int index) {
  if (!isdigit(line[0])) {
    return false;
  }

  int cell_num = 0, cell_volt, cell_curr, cell_tempr, cell_coulomb;
  char base_st[LV1_STR_LEN], volt_st[LV1_STR_LEN], curr_st[LV1_STR_LEN], temp_st[LV1_STR_LEN];

  const int parsed = sscanf(                                            // NOLINT
      line.c_str(), "%d %d %d %d %7s %7s %7s %7s %d%%",                 // NOLINT
      &cell_num, &cell_volt, &cell_curr, &cell_tempr, base_st,         // NOLINT
      volt_st, curr_st, temp_st, &cell_coulomb);                        // NOLINT

  if (parsed != 9) {
    ESP_LOGE(TAG, "invalid LV1 cell line: found only %d, should be 9 items: %s", parsed,
             line.substr(0, line.size() - 2).c_str());
    return false;
  }

  this->cell_ = {};
  this->cell_.bat_num = index;
  this->cell_.cell_num = cell_num;
  this->cell_.voltage = (float) cell_volt;
  this->cell_.current = (float) cell_curr;
  this->cell_.temperature = (float) cell_tempr;
  this->cell_.coulomb = (float) cell_coulomb;
  this->cell_.base_state = base_st;
  this->cell_.voltage_state = volt_st;
  this->cell_.current_state = curr_st;
  this->cell_.temperature_state = temp_st;
  return true;
}

}  // namespace pytes_e_box
}  // namespace esphome
