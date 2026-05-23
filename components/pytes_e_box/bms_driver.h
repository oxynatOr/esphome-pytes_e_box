#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "bms_data.h"

namespace esphome {
namespace pytes_e_box {

// Command identifiers are driver-defined. Only these two values carry universal
// meaning to the host; everything >= 1 is interpreted by the active driver.
using BmsCommandId = int;
static const BmsCommandId CMD_ERROR = -1;
static const BmsCommandId CMD_NIL = 0;

struct PollingCommand {
  std::string command;
  int index;
  uint8_t length = 0;
  uint8_t errors = 0;
  BmsCommandId identifier;
};

// Selectable BMS protocol. Extend this enum (and the Python mapping) to add a
// new driver.
enum class BmsType { PYTES_E_BOX, EXAMPLE };

// What the host should publish after a parse step.
enum class BmsEmit { NONE, BATTERY, CELL };

// A protocol driver owns everything BMS-specific: the command set, how to
// recognize a response, how to detect its end, and how to turn response lines
// into the normalized data model. The host owns only UART transport and the
// generic polling/retry state machine.
class BmsDriver {
 public:
  virtual ~BmsDriver() = default;

  // Build the polling command queue for the given number of batteries.
  virtual void build_commands(std::vector<PollingCommand> &queue, int battery_count) = 0;

  // Identify which command an echoed prompt/header line belongs to. Return
  // CMD_NIL/CMD_ERROR when the line is not a recognizable command echo.
  virtual BmsCommandId read_command(const std::string &line) = 0;

  // Response terminator detection (0 = not complete, >0 = complete).
  virtual int is_line_complete(const std::string &line) = 0;

  // Whether response lines for this command need per-line preprocessing before
  // parsing (e.g. whitespace trimming).
  virtual bool needs_line_preprocess(BmsCommandId cmd) { return false; }
  virtual void preprocess_line(BmsCommandId cmd, std::string &line) {}

  // Parse a single response line into the driver's working data. Return whether
  // the host should emit battery or cell data now (per-line protocols) or wait
  // (accumulating protocols).
  virtual BmsEmit parse_line(BmsCommandId cmd, const std::string &line, int index) = 0;

  // Called once when the full response has arrived. Accumulating protocols emit
  // here.
  virtual BmsEmit command_complete(BmsCommandId cmd, int index) { return BmsEmit::NONE; }

  // Clear working buffers; called by the host before each command.
  virtual void reset() = 0;

  // Most recent parsed data, valid right after parse_line/command_complete
  // returns BATTERY/CELL.
  virtual const BmsBatteryData &battery_data() const = 0;
  virtual const BmsCellData &cell_data() const = 0;

  // Human-readable command name, for logging only.
  virtual std::string command_to_string(BmsCommandId cmd) { return "command"; }
};

}  // namespace pytes_e_box
}  // namespace esphome
