#include "pytes_driver.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cctype>
#include <cstdio>
#include <cstring>
#include <regex>

namespace esphome {
namespace pytes_e_box {

static const char *const TAG = "pytes_e_box";

static const uint8_t STR_MIN_LEN = 8;
static const uint8_t STR_MAX_LEN = 60;

static const std::regex TRIM_PATTERN("(^\\s|\\s{2,})");

void PytesDriver::add_command_(std::vector<PollingCommand> &queue, const char *command, int index, BmsCommandId id) {
  PollingCommand cmd;
  cmd.command = command;
  cmd.errors = 0;
  cmd.index = index;
  cmd.identifier = id;
  cmd.length = strlen(command);
  queue.push_back(cmd);
}

void PytesDriver::build_commands(std::vector<PollingCommand> &queue, int battery_count) {
  this->add_command_(queue, "pwr", 0, PYTES_PWR);
  std::string cmd;
  for (int i = 1; i <= battery_count; i++) {
    cmd = "pwr " + to_string(i);
    this->add_command_(queue, cmd.c_str(), i, PYTES_PWR_INDEX);
    cmd = "bat " + to_string(i);
    this->add_command_(queue, cmd.c_str(), i, PYTES_BAT_INDEX);
  }
}

BmsCommandId PytesDriver::read_command(const std::string &line) {
  char cmd_[4];
  int no_ = -1;
  const int parsed = sscanf(line.c_str(), "PYTES>%s %d", cmd_, &no_);
  if (parsed <= 0) {
    if (sscanf(line.c_str(), "%s %d", cmd_, &no_) <= 0) {
      ESP_LOGE(TAG, "Line dont have 'Pytes>' tag: %s", line.c_str());
      return CMD_ERROR;
    }
  }
  std::string command = cmd_;
  if ((command == "pwr") && (no_ == -1)) {
    return PYTES_PWR;
  }
  if ((command == "pwr") && (no_ >= 1)) {
    return PYTES_PWR_INDEX;
  }
  if ((command == "bat") && (no_ == -1)) {
    return PYTES_BAT;
  }
  if ((command == "bat") && (no_ >= 1)) {
    return PYTES_BAT_INDEX;
  }
  return CMD_NIL;
}

int PytesDriver::is_line_complete(const std::string &line) {
  if (line == "$$") {
    return 1;
  }
  if (line == "Command completed successfully ") {
    return 2;
  }
  return 0;
}

bool PytesDriver::needs_line_preprocess(BmsCommandId cmd) { return cmd == PYTES_PWR_INDEX; }

void PytesDriver::preprocess_line(BmsCommandId cmd, std::string &line) {
  line = std::regex_replace(line, TRIM_PATTERN, "");
}

std::string PytesDriver::command_to_string(BmsCommandId cmd) {
  switch (cmd) {
    case CMD_NIL:
      return "Not in List";
    case PYTES_PWR:
      return "Power Data";
    case PYTES_PWR_INDEX:
      return "Power Data Index";
    case PYTES_BAT_INDEX:
      return "Battery Data Index";
    case PYTES_BAT:
      return "Battery Data";
    case CMD_ERROR:
      return "ERROR?!";
    default:
      return "No Command found!";
  }
}

void PytesDriver::reset() {
  this->bat_ = {};
  this->cell_ = {};
}

BmsEmit PytesDriver::parse_line(BmsCommandId cmd, const std::string &line, int index) {
  switch (cmd) {
    case PYTES_PWR:
      return this->parse_pwr_line_(line) ? BmsEmit::BATTERY : BmsEmit::NONE;
    case PYTES_PWR_INDEX:
      this->parse_pwr_index_(line, index);
      return BmsEmit::NONE;
    case PYTES_BAT_INDEX:
      return this->parse_bat_index_(line, index) ? BmsEmit::CELL : BmsEmit::NONE;
    default:
      return BmsEmit::NONE;
  }
}

BmsEmit PytesDriver::command_complete(BmsCommandId cmd, int index) {
  if (cmd == PYTES_PWR_INDEX) {
    this->bat_.bat_num = index;
    return BmsEmit::BATTERY;
  }
  return BmsEmit::NONE;
}

bool PytesDriver::parse_pwr_line_(const std::string &line) {
  if (!isdigit(line[0]) || (line.find("Absent") != std::string::npos)) {
    return false;
  }

  int bat_num = 0, voltage, current, temperature, tlow, thigh, vlow, vhigh, coulomb;
  int day, month, year, hour, minute, sec;
  char base_st[STR_MIN_LEN], volt_st[STR_MIN_LEN], curr_st[STR_MIN_LEN], temp_st[STR_MIN_LEN];
  char bv_st[STR_MIN_LEN], bt_st[STR_MIN_LEN], serial_st[STR_MAX_LEN], devtype_st[STR_MAX_LEN];

  const int parsed = sscanf(                                                                       // NOLINT
      line.c_str(), "%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %d-%d-%d %d:%d:%d %s %s %s %s",   // NOLINT
      &bat_num, &voltage, &current, &temperature, &tlow, &thigh, &vlow, &vhigh,                    // NOLINT
      base_st, volt_st, curr_st, temp_st, &coulomb, &day, &month, &year, &hour,                    // NOLINT
      &minute, &sec, bv_st, bt_st, serial_st, devtype_st);                                         // NOLINT

  if (parsed != 23) {
    ESP_LOGE(TAG, "invalid line: found only %d, should be 23 items. in line %d\n: %s", parsed, bat_num,
             line.substr(0, line.size() - 2).c_str());
    return false;
  }
  if (bat_num <= 0) {
    ESP_LOGE(TAG, "invalid bat_num in line %s", line.substr(0, line.size() - 2).c_str());
    return false;
  }

  this->bat_ = {};
  this->bat_.bat_num = bat_num;
  this->bat_.voltage = (float) voltage;
  this->bat_.current = (float) current;
  this->bat_.temperature = (float) temperature;
  this->bat_.temperature_low = (float) tlow;
  this->bat_.temperature_high = (float) thigh;
  this->bat_.voltage_low = (float) vlow;
  this->bat_.voltage_high = (float) vhigh;
  this->bat_.coulomb = (float) coulomb;
  this->bat_.base_state = base_st;
  this->bat_.voltage_state = volt_st;
  this->bat_.current_state = curr_st;
  this->bat_.temperature_state = temp_st;
  return true;
}

bool PytesDriver::parse_bat_index_(const std::string &line, int index) {
  if (!isdigit(line[0])) {
    return false;
  }

  int cell_num = 0, cell_volt, cell_tempr, cell_coulomb, cell_curr;
  char base_st[STR_MAX_LEN], volt_st[STR_MAX_LEN], curr_st[STR_MAX_LEN], temp_st[STR_MAX_LEN];

  const int parsed = sscanf(                                            // NOLINT
      line.c_str(), "%d %d %d %7s %7s %7s %7s %d%% %d",                 // NOLINT
      &cell_num, &cell_volt, &cell_tempr, base_st, volt_st,            // NOLINT
      curr_st, temp_st, &cell_coulomb, &cell_curr);                     // NOLINT

  if (parsed != 9) {
    ESP_LOGE(TAG, "invalid line: found only %d, should be 9 items. in line %d\n: %s", parsed, cell_num,
             line.substr(0, line.size() - 2).c_str());
    return false;
  }

  this->cell_ = {};
  this->cell_.bat_num = index;
  this->cell_.cell_num = cell_num;
  this->cell_.voltage = (float) cell_volt;
  this->cell_.temperature = (float) cell_tempr;
  this->cell_.coulomb = (float) cell_coulomb;
  this->cell_.current = (float) cell_curr;
  this->cell_.base_state = base_st;
  this->cell_.voltage_state = volt_st;
  this->cell_.current_state = curr_st;
  this->cell_.temperature_state = temp_st;
  return true;
}

void PytesDriver::parse_pwr_index_(const std::string &line, int index) {
  this->bat_.bat_num = index;
  int v;
  char buf[STR_MAX_LEN];

  if (line.rfind("SOC Voltage:", 0) == 0) {
    if (sscanf(line.c_str(), "SOC Voltage: %dmV", &v) == 1)
      this->bat_.soc_voltage = (float) v;
  }
  if (line.rfind("Total Coulomb:", 0) == 0) {
    if (sscanf(line.c_str(), "Total Coulomb: %dmAS", &v) == 1)
      this->bat_.total_coulomb = (float) v;
  }
  if (line.rfind("Real Coulomb:", 0) == 0) {
    if (sscanf(line.c_str(), "Real Coulomb: %dmAH", &v) == 1)
      this->bat_.real_coulomb = (float) v;
  }
  if (line.rfind("Total Power In:", 0) == 0) {
    if (sscanf(line.c_str(), "Total Power In: %dAS", &v) == 1)
      this->bat_.total_power_in = (float) v;
  }
  if (line.rfind("Work Status:", 0) == 0) {
    if (sscanf(line.c_str(), "Work Status: %d", &v) == 1)
      this->bat_.work_status = (float) v;
  }
  if (line.rfind("Bat Num:", 0) == 0) {
    if (sscanf(line.c_str(), " Bat Num: %d", &v) == 1)
      this->bat_.cell_count = (float) v;
  }
  if (line.rfind("Total Power Out :", 0) == 0) {
    if (sscanf(line.c_str(), "Total Power Out : %dAS ", &v) == 1)
      this->bat_.total_power_out = (float) v;
  }
  if (line.rfind("Barcode:", 0) == 0) {
    if (sscanf(line.c_str(), " Barcode: %[^\n]", buf) == 1)
      this->bat_.barcode = buf;
  }
  if (line.rfind("Firm Version:", 0) == 0) {
    if (sscanf(line.c_str(), "Firm Version: %[^\n\r]", buf) == 1)
      this->bat_.firm_version = buf;
  }
  if (line.rfind("Coul. Status:", 0) == 0) {
    if (sscanf(line.c_str(), "Coul. Status: %[^\n]", buf) == 1)
      this->bat_.coulomb_status = buf;
  }
  if (line.rfind("Bat Status:", 0) == 0) {
    if (sscanf(line.c_str(), "Bat Status: %[^\n]", buf) == 1)
      this->bat_.bat_status = buf;
  }
  if (line.rfind("CMOS Status:", 0) == 0) {
    if (sscanf(line.c_str(), " CMOS Status: %[^\n]", buf) == 1)
      this->bat_.cmos_status = buf;
  }
  if (line.rfind("DMOS Status:", 0) == 0) {
    if (sscanf(line.c_str(), " DMOS Status: %[^\n]", buf) == 1)
      this->bat_.dmos_status = buf;
  }
  if (line.rfind("Bat Protect ENA :", 0) == 0) {
    if (sscanf(line.c_str(), " Bat Protect ENA : %[^\n]", buf) == 1)
      this->bat_.bat_protect_ena = buf;
  }
  if (line.rfind("Pwr Protect ENA :", 0) == 0) {
    if (sscanf(line.c_str(), " Pwr Protect ENA : %[^\n]", buf) == 1)
      this->bat_.pwr_protect_ena = buf;
  }
  if (line.rfind("Bat Events:", 0) == 0) {
    if (sscanf(line.c_str(), " Bat Events: %[^\n]", buf) == 1)
      this->bat_.bat_events = buf;
  }
  if (line.rfind("Power Events:", 0) == 0) {
    if (sscanf(line.c_str(), " Power Events: %[^\n]", buf) == 1)
      this->bat_.power_events = buf;
  }
  if (line.rfind("System Fault:", 0) == 0) {
    if (sscanf(line.c_str(), " System Fault: %[^\n]", buf) == 1)
      this->bat_.system_fault = buf;
  }
  if (line.rfind("DevType:", 0) == 0) {
    if (sscanf(line.c_str(), " DevType: %[^\n]", buf) == 1)
      this->bat_.dev_type = buf;
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
