#pragma once

#include "pytes_driver.h"

namespace esphome {
namespace pytes_e_box {

// Driver for the "Pytes LV1 Stack Battery".
//
// The LV1 console is identical to the Pytes E-Box (same pwr / pwr N / bat N
// commands, same "PYTES>" prompt and terminators, same pwr table) EXCEPT for the
// per-cell line of the "bat N" command: the LV1 reports a real per-cell current
// column right after the voltage (num volt curr tempr ...), whereas the E-Box
// puts current at the end (num volt tempr ... coulomb% curr).
//
// We therefore reuse PytesDriver and only override the cell-line parser.
class PytesLv1Driver : public PytesDriver {
 protected:
  bool parse_bat_index_(const std::string &line, int index) override;
};

}  // namespace pytes_e_box
}  // namespace esphome
