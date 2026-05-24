#pragma once

#include "bms_driver.h"

namespace esphome {
namespace pytes_e_box {

// Driver for the Pytes E-Box RS232 console protocol (ASCII tables behind a
// "PYTES>" prompt). Implements the original pwr / pwr N / bat N command set.
class PytesDriver : public BmsDriver {
 public:
  void build_commands(std::vector<PollingCommand> &queue, int battery_count) override;
  BmsCommandId read_command(const std::string &line) override;
  int is_line_complete(const std::string &line) override;
  bool needs_line_preprocess(BmsCommandId cmd) override;
  void preprocess_line(BmsCommandId cmd, std::string &line) override;
  BmsEmit parse_line(BmsCommandId cmd, const std::string &line, int index) override;
  BmsEmit command_complete(BmsCommandId cmd, int index) override;
  void reset() override;
  const BmsBatteryData &battery_data() const override { return this->bat_; }
  const BmsCellData &cell_data() const override { return this->cell_; }
  std::string command_to_string(BmsCommandId cmd) override;

 protected:
  enum PytesCommand : BmsCommandId {
    PYTES_PWR = 1,
    PYTES_PWR_INDEX = 2,
    PYTES_BAT = 3,
    PYTES_BAT_INDEX = 4,
  };

  void add_command_(std::vector<PollingCommand> &queue, const char *command, int index, BmsCommandId id);

  // pwr: one complete battery summary per line.
  bool parse_pwr_line_(const std::string &line);
  // bat N: one cell per line. virtual so variants (e.g. LV1) can override just
  // the cell-line column layout while reusing the rest of the protocol.
  virtual bool parse_bat_index_(const std::string &line, int index);
  // pwr N: key:value spread across many lines, accumulated into bat_.
  void parse_pwr_index_(const std::string &line, int index);

  BmsBatteryData bat_{};
  BmsCellData cell_{};
};

}  // namespace pytes_e_box
}  // namespace esphome
