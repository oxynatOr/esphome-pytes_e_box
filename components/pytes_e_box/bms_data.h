#pragma once

#include <cmath>
#include <string>

namespace esphome {
namespace pytes_e_box {

// Normalized, protocol-agnostic battery data model.
// Numeric fields default to NAN and text fields to empty: a value left at its
// default means "not provided by this message" and is not published. Drivers
// fill the subset they parse; sensors publish only present fields.

struct BmsBatteryData {
  int bat_num = 0;

  float voltage = NAN;
  float current = NAN;
  float temperature = NAN;
  float temperature_low = NAN;
  float temperature_high = NAN;
  float mos_temperature = NAN;
  float voltage_low = NAN;
  float voltage_high = NAN;
  float coulomb = NAN;
  float soc_voltage = NAN;
  float total_coulomb = NAN;
  float real_coulomb = NAN;
  float total_power_in = NAN;
  float total_power_out = NAN;
  float work_status = NAN;
  float cell_count = NAN;

  std::string base_state;
  std::string voltage_state;
  std::string current_state;
  std::string temperature_state;
  std::string barcode;
  std::string dev_type;
  std::string firm_version;
  std::string coulomb_status;
  std::string bat_status;
  std::string cmos_status;
  std::string dmos_status;
  std::string bat_protect_ena;
  std::string pwr_protect_ena;
  std::string bat_events;
  std::string power_events;
  std::string system_fault;
};

struct BmsCellData {
  int bat_num = 0;
  int cell_num = 0;

  float voltage = NAN;
  float current = NAN;
  float temperature = NAN;
  float coulomb = NAN;

  std::string base_state;
  std::string voltage_state;
  std::string current_state;
  std::string temperature_state;
};

}  // namespace pytes_e_box
}  // namespace esphome
