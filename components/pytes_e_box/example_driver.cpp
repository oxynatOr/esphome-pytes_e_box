#include "example_driver.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <cstdio>
#include <cstring>

namespace esphome {
namespace pytes_e_box {

static const char *const TAG = "pytes_e_box";

void ExampleDriver::build_commands(std::vector<PollingCommand> &queue, int battery_count) {
  // One "stat <n>" command per battery. The index travels with the command so
  // parse_line() knows which battery a response belongs to.
  for (int i = 1; i <= battery_count; i++) {
    std::string cmd = "stat " + to_string(i);
    PollingCommand pc;
    pc.command = cmd;
    pc.index = i;
    pc.identifier = EX_STAT;
    pc.length = cmd.size();
    pc.errors = 0;
    queue.push_back(pc);
  }
}

BmsCommandId ExampleDriver::read_command(const std::string &line) {
  // Recognize the echoed command, with or without the "EXAMPLE>" prompt.
  char cmd[8] = {0};
  int no = -1;
  int n = sscanf(line.c_str(), "EXAMPLE>%7s %d", cmd, &no);
  if (n < 1) {
    n = sscanf(line.c_str(), "%7s %d", cmd, &no);
  }
  if (n < 1) {
    return CMD_NIL;
  }
  if (std::string(cmd) == "stat") {
    return EX_STAT;
  }
  return CMD_NIL;
}

int ExampleDriver::is_line_complete(const std::string &line) { return (line == "$$") ? 1 : 0; }

BmsEmit ExampleDriver::parse_line(BmsCommandId cmd, const std::string &line, int index) {
  if (cmd != EX_STAT) {
    return BmsEmit::NONE;
  }
  // Only the data line starts with "V:"; skip headers, the prompt and "$$".
  if (line.rfind("V:", 0) != 0) {
    return BmsEmit::NONE;
  }

  int voltage_mv, current_ma, temp_mc, soc;
  const int parsed = sscanf(line.c_str(), "V:%d I:%d T:%d SOC:%d", &voltage_mv, &current_ma, &temp_mc, &soc);
  if (parsed != 4) {
    ESP_LOGE(TAG, "example: expected 4 fields, got %d in: %s", parsed, line.c_str());
    return BmsEmit::NONE;
  }

  // Map onto the normalized model. Scaling to engineering units happens in the
  // sensor classes (mV/m°C divided by 1000), exactly like the Pytes driver, so
  // the existing sensor platform works unchanged.
  this->bat_ = {};
  this->bat_.bat_num = index;
  this->bat_.voltage = (float) voltage_mv;
  this->bat_.current = (float) current_ma;
  this->bat_.temperature = (float) temp_mc;
  this->bat_.coulomb = (float) soc;
  return BmsEmit::BATTERY;
}

void ExampleDriver::reset() {
  this->bat_ = {};
  this->cell_ = {};
}

std::string ExampleDriver::command_to_string(BmsCommandId cmd) {
  switch (cmd) {
    case EX_STAT:
      return "Example Status";
    case CMD_NIL:
      return "Not in List";
    case CMD_ERROR:
      return "ERROR?!";
    default:
      return "No Command found!";
  }
}

}  // namespace pytes_e_box
}  // namespace esphome
