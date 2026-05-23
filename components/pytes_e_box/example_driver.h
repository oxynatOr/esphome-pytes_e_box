#pragma once

#include "bms_driver.h"

namespace esphome {
namespace pytes_e_box {

// =============================================================================
// TEMPLATE DRIVER
// -----------------------------------------------------------------------------
// Copy this file pair (example_driver.h / example_driver.cpp) as the starting
// point for a new BMS type. It speaks a small, made-up ASCII console protocol
// so every BmsDriver hook is shown with a concrete, compiling implementation.
//
// To add a real BMS:
//   1. Rename the class and files (e.g. SeplosDriver / seplos_driver.*).
//   2. Replace the command strings in build_commands().
//   3. Replace the prompt/echo recognition in read_command().
//   4. Replace the terminator(s) in is_line_complete().
//   5. Replace the line parsing in parse_line() and map fields onto the
//      normalized BmsBatteryData / BmsCellData model (see bms_data.h).
//   6. Register the type: add a value to BmsType (bms_driver.h), a case in
//      PytesEBoxComponent::setup() (pytes_e_box.cpp), and an entry in
//      BMS_TYPES (__init__.py).
//
// Two response shapes are common; this template demonstrates the first:
//   * One self-contained line per record  -> parse_line() returns BATTERY/CELL
//     immediately (used here, and by Pytes' "pwr"/"bat N").
//   * Fields spread over many lines        -> parse_line() accumulates and
//     returns NONE; command_complete() emits once. See PytesDriver for that
//     pattern (its "pwr N" handling).
//
// The made-up protocol: for each battery we send "stat <n>". The device echoes
// the command behind an "EXAMPLE>" prompt and answers with a single line, then
// a "$$" terminator, e.g.:
//
//   EXAMPLE>stat 1
//   V:53200 I:-1200 T:21000 SOC:87
//   $$
//
// where V is mV, I is mA, T is m°C and SOC is percent.
// =============================================================================
class ExampleDriver : public BmsDriver {
 public:
  void build_commands(std::vector<PollingCommand> &queue, int battery_count) override;
  BmsCommandId read_command(const std::string &line) override;
  int is_line_complete(const std::string &line) override;
  BmsEmit parse_line(BmsCommandId cmd, const std::string &line, int index) override;
  void reset() override;
  const BmsBatteryData &battery_data() const override { return this->bat_; }
  const BmsCellData &cell_data() const override { return this->cell_; }
  std::string command_to_string(BmsCommandId cmd) override;

 protected:
  enum ExampleCommand : BmsCommandId {
    EX_STAT = 1,
  };

  BmsBatteryData bat_{};
  BmsCellData cell_{};
};

}  // namespace pytes_e_box
}  // namespace esphome
