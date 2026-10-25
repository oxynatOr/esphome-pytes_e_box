#pragma once

#include "bms_driver.h"

namespace esphome {
namespace pytes_e_box {

// Driver for the Pylontech console protocol (US2000/US3000 and compatibles).
//
// Ported from the upstream ESPHome `pylontech` component. Unlike Pytes, the
// Pylontech console is fire-and-forget: we send a single "pwr" per update and
// then parse every line that arrives. There is no command-echo handshake and no
// "$$" terminator, so this driver runs in the host's streaming mode
// (is_streaming() == true).
//
// One "pwr" returns one data line per battery (bat_num is in the line). A header
// line (first token "Power") announces the column layout; newer firmware adds
// Tlow.Id/Thigh.Id/Vlow.Id/Vhigh.Id columns, detected via the "Tlow.Id" token.
class PylontechDriver : public BmsDriver {
 public:
  bool is_streaming() const override { return true; }
  void build_commands(std::vector<PollingCommand> &queue, int battery_count) override;
  BmsCommandId read_command(const std::string &line) override;
  int is_line_complete(const std::string &line) override;
  BmsEmit parse_line(BmsCommandId cmd, const std::string &line, int index) override;
  void reset() override;
  const BmsBatteryData &battery_data() const override { return this->bat_; }
  const BmsCellData &cell_data() const override { return this->cell_; }
  std::string command_to_string(BmsCommandId cmd) override;

 protected:
  enum PylontechCommand : BmsCommandId {
    PYLON_PWR = 1,
  };

  bool has_tlow_id_ = false;
  BmsBatteryData bat_{};
  BmsCellData cell_{};
};

}  // namespace pytes_e_box
}  // namespace esphome
