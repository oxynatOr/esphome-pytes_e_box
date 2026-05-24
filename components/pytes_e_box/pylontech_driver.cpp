#include "pylontech_driver.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <algorithm>
#include <cstring>

namespace esphome {
namespace pytes_e_box {

static const char *const TAG = "pytes_e_box";
static const size_t TOKEN_LEN = 16;

// Whitespace-delimited token extraction (skips leading spaces/tabs, stops at the
// next space/tab/CR or end of string).
static void get_token(const char *&cursor, char *out, size_t out_size) {
  while (*cursor == ' ' || *cursor == '\t') {
    cursor++;
  }
  if (*cursor == '\0') {
    out[0] = 0;
    return;
  }
  const char *start = cursor;
  while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\r') {
    cursor++;
  }
  size_t len = std::min(static_cast<size_t>(cursor - start), out_size - 1);
  memcpy(out, start, len);
  out[len] = 0;
}

void PylontechDriver::build_commands(std::vector<PollingCommand> &queue, int battery_count) {
  // A single "pwr" returns all batteries; battery_count is only used by the
  // sensors to filter on bat_num.
  PollingCommand pc;
  pc.command = "pwr";
  pc.index = 0;
  pc.identifier = PYLON_PWR;
  pc.length = 3;
  pc.errors = 0;
  queue.push_back(pc);
}

// Not used in streaming mode (the host splits lines on LF and never waits for a
// terminator), but the interface requires them.
BmsCommandId PylontechDriver::read_command(const std::string &line) { return CMD_NIL; }
int PylontechDriver::is_line_complete(const std::string &line) { return 0; }

void PylontechDriver::reset() {
  this->bat_ = {};
  this->cell_ = {};
  this->has_tlow_id_ = false;
}

BmsEmit PylontechDriver::parse_line(BmsCommandId cmd, const std::string &line, int index) {
  const char *cursor = line.c_str();
  char tok[TOKEN_LEN];

  // First token: battery number (data line), "Power" (header) or anything else.
  get_token(cursor, tok, sizeof(tok));
  auto first = parse_number<int>(std::string(tok));
  if (first.has_value() && first.value() > 0) {
    // data line -> fall through
  } else if (strcmp(tok, "Power") == 0) {
    this->has_tlow_id_ = line.find("Tlow.Id") != std::string::npos;
    ESP_LOGD(TAG, "pylontech header %s Tlow.Id", this->has_tlow_id_ ? "with" : "without");
    return BmsEmit::NONE;
  } else {
    return BmsEmit::NONE;
  }

  this->bat_ = {};
  this->bat_.bat_num = first.value();

  auto next_int = [&](float &target) -> bool {
    get_token(cursor, tok, sizeof(tok));
    auto v = parse_number<int>(std::string(tok));
    if (!v.has_value()) {
      return false;
    }
    target = (float) v.value();
    return true;
  };
  auto next_str = [&](std::string &target) -> bool {
    get_token(cursor, tok, sizeof(tok));
    if (strlen(tok) < 2) {
      return false;
    }
    target = tok;
    return true;
  };
  auto skip = [&]() { get_token(cursor, tok, sizeof(tok)); };

  if (!next_int(this->bat_.voltage))
    return BmsEmit::NONE;
  if (!next_int(this->bat_.current))
    return BmsEmit::NONE;
  if (!next_int(this->bat_.temperature))
    return BmsEmit::NONE;
  if (!next_int(this->bat_.temperature_low))
    return BmsEmit::NONE;
  if (this->has_tlow_id_)
    skip();
  if (!next_int(this->bat_.temperature_high))
    return BmsEmit::NONE;
  if (this->has_tlow_id_)
    skip();
  if (!next_int(this->bat_.voltage_low))
    return BmsEmit::NONE;
  if (this->has_tlow_id_)
    skip();
  if (!next_int(this->bat_.voltage_high))
    return BmsEmit::NONE;
  if (this->has_tlow_id_)
    skip();
  if (!next_str(this->bat_.base_state))
    return BmsEmit::NONE;
  if (!next_str(this->bat_.voltage_state))
    return BmsEmit::NONE;
  if (!next_str(this->bat_.current_state))
    return BmsEmit::NONE;
  if (!next_str(this->bat_.temperature_state))
    return BmsEmit::NONE;

  // Coulomb (SoC), trailing '%' stripped.
  get_token(cursor, tok, sizeof(tok));
  for (char *p = tok; *p != '\0'; ++p) {
    if (*p == '%') {
      *p = 0;
      break;
    }
  }
  auto coul = parse_number<int>(std::string(tok));
  if (!coul.has_value())
    return BmsEmit::NONE;
  this->bat_.coulomb = (float) coul.value();

  // Skip Date, Time, B.V.St, B.T.St.
  skip();
  skip();
  skip();
  skip();

  if (!next_int(this->bat_.mos_temperature))
    return BmsEmit::NONE;

  return BmsEmit::BATTERY;
}

std::string PylontechDriver::command_to_string(BmsCommandId cmd) {
  switch (cmd) {
    case PYLON_PWR:
      return "Pylontech Power";
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
