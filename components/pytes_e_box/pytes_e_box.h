#pragma once

#include "esphome/core/component.h"
#include "esphome/core/defines.h"
#include "esphome/components/uart/uart.h"
#include <regex>


namespace esphome {
namespace pytes_e_box {
static const char *const TAG = "pytes_e_box";
static const uint8_t NUM_BUFFERS = 128; 
static const uint8_t TEXT_SENSOR_MIN_LEN = 8;
static const uint8_t TEXT_SENSOR_BIG_LEN = 18;
static const uint8_t TEXT_SENSOR_MAX_LEN = 60;

enum ENUMCommand {
  CMD_ERROR = -1,
  CMD_NIL = 0,
  CMD_PWR = 1,
  CMD_PWR_INDEX = 2,
  CMD_BAT = 3,
  CMD_BAT_INDEX = 4,
};


struct PollingCommand {
  std::string command;
  int index;
  uint8_t length = 0;
  uint8_t errors = 0;
  ENUMCommand identifier;
}; 

class PytesEBoxListener { 
public:
  struct pwr_LineContents {
    uint32_t bat_num = 0, tlow, thigh, vlow, vhigh, day, month, year, hour, min, sec, coulomb, voltage, current, temperature;
    char base_st[TEXT_SENSOR_MIN_LEN], volt_st[TEXT_SENSOR_MIN_LEN], curr_st[TEXT_SENSOR_MIN_LEN],
        temp_st[TEXT_SENSOR_MIN_LEN], serial_st[TEXT_SENSOR_MAX_LEN], devtype_st[TEXT_SENSOR_MAX_LEN],
        bv_st[TEXT_SENSOR_MIN_LEN], bt_st[TEXT_SENSOR_MIN_LEN];
  };
  struct bat_index_LineContents {
    uint32_t bat_num = 0,cell_num = 0, cell_volt,cell_tempr, cell_coulomb, cell_curr;
    char cell_baseState[TEXT_SENSOR_MAX_LEN], cell_voltState[TEXT_SENSOR_MAX_LEN], cell_currState[TEXT_SENSOR_MAX_LEN], 
          cell_tempState[TEXT_SENSOR_MAX_LEN];
  };
  
  struct pwr_data_LineContents {
    uint32_t bat_num = 0, SOCVoltage,  coulomb, totalCoulomb, realCoulomb, totalPowerIn, totalPowerOut, 
        workStatus, batNum, nextDevice, cells = -1;
    char FirmVersion[TEXT_SENSOR_BIG_LEN], CoulStatus[TEXT_SENSOR_BIG_LEN], BatStatus[TEXT_SENSOR_BIG_LEN],
        CMOSStatus[TEXT_SENSOR_BIG_LEN], DMOSStatus[TEXT_SENSOR_BIG_LEN], BatProtectENA[TEXT_SENSOR_MAX_LEN], 
        PwrProtectENA[TEXT_SENSOR_MAX_LEN], BatEvents[TEXT_SENSOR_MIN_LEN], PowerEvents[TEXT_SENSOR_MIN_LEN], 
        SystemFault[TEXT_SENSOR_MIN_LEN], Barcode[TEXT_SENSOR_BIG_LEN], DevType[TEXT_SENSOR_BIG_LEN];
  };  

  virtual void on_pwr_line_read(pwr_LineContents *line);
  virtual void on_pwrn_line_read(pwr_data_LineContents *line);
  virtual void on_batn_line_read(bat_index_LineContents *line);
  virtual void dump_config();
  
};

class PytesEBoxComponent : public PollingComponent, public uart::UARTDevice {
public:
  PytesEBoxComponent();

  void update() override;
  void loop() override;
  void setup() override;
  void dump_config() override;

  float get_setup_priority() const override;

  ENUMCommand readCommand(std::string &buffer) {
  char cmd_[4];
  int no_ = -1;
  const int parsed = sscanf(buffer.c_str(),"PYTES>%s %d",cmd_,&no_);
  if (parsed <= 0) {
    if (sscanf(buffer.c_str(),"%s %d",cmd_,&no_) <= 0) {
    ESP_LOGE(TAG, "Line dont have 'Pytes>' tag: %s",buffer.c_str());
    return CMD_ERROR;
    }
  }
  std::string command = cmd_;
  if ((command == "pwr") && (no_ == -1)) { return CMD_PWR; }
  if ((command == "pwr") && (no_ >= 1)) { return CMD_PWR_INDEX; }
  if ((command == "bat") && (no_ == -1)) { return CMD_BAT; }
  if ((command == "bat") && (no_ >= 1)) { return CMD_BAT_INDEX; }
  return CMD_NIL;
  }

  std::string CommandtoString(ENUMCommand cmd){
    switch (cmd) {
    case CMD_NIL: {
      return "Not in List";
    }
    case CMD_PWR: {
      return "Power Data";
    }
    case CMD_PWR_INDEX: {
      return "Power Data Index";
    }
    case CMD_BAT_INDEX: {
      return "Battery Data Index";
    }
    case CMD_BAT: {
      return "Battery Data";
    }
    case CMD_ERROR: {
      return "ERROR?!";
    }
    default:
      return "No Command found!";
  }
}

  int isLineComplete(std::string &buffer) {
    if (buffer == "$$") { return 1; }
    if (buffer == "Command completed successfully ") { return 2; }
    return 0;
  }

  void register_listener(PytesEBoxListener *listener) { this->listeners_.push_back(listener); }

  void set_cmd_idle_time(uint32_t cmd_idle_time) { this->command_idle_time_ = cmd_idle_time; }
  void set_polling_timeout(uint32_t poll_timeout) { this->polling_timeout_ = poll_timeout; }
  void set_system_battery_count(uint32_t num_bats) { this->battaries_in_system_ = num_bats; }
  int readline(int readch, char *buffer, int len) {
  static int pos = 0;
  int rpos;
  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len-1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
  }  

  std::vector<std::string> splitData(std::string s){
    std::vector<std::string> res;
    int pos = 0;
    while(pos < s.size()){
        pos = s.find(":");
        res.push_back(s.substr(0,pos));
        s.erase(0,pos+1);
    }
    return res;
}
  void clear_uart_buffer(); 
protected:
  static const size_t COMMAND_QUEUE_LENGTH = 34; //16Boxes with 16 Batterys and one index. 
  uint32_t battaries_in_system_;
  uint32_t polling_timeout_;
  uint32_t command_idle_time_;
  uint32_t found_battaries_in_system_;
  uint32_t last_poll_ = 0;
  std::string buffer_[NUM_BUFFERS];
  int buffer_index_write_ = 0;
  int buffer_index_read_ = 0;

  uint8_t state_ = 254;
  enum State {
    STATE_WAIT                = 254,
    STATE_IDLE                = 0,
    STATE_POLL                = 1,
    STATE_COMMAND             = 2,
    STATE_POLL_COMPLETE       = 3,
    STATE_COMMAND_COMPLETE    = 4,
    STATE_POLL_CHECKED        = 5,
    STATE_POLL_DECODED        = 6,
    STATE_SEND_NEXT_COMMAND   = 7,
  };
  
  PytesEBoxListener::pwr_LineContents pwr_index_l{};
  PytesEBoxListener::bat_index_LineContents  bat_index_l{};
  PytesEBoxListener::pwr_data_LineContents pwr_data_l{};
  
  void add_polling_command_(const char *command, int _index, ENUMCommand polling_command);
  std::vector<PollingCommand> cmd_queue_{};
  uint8_t command_queue_position_ = 0;
  uint8_t command_queue_max_ = 0;
  uint8_t send_next_command_();

  void processData_pwrLine(std::string &buffer);
  void processData_batIndexLine(std::string &buffer, int bat_num);
  void processData_pwrIndex(std::string &buffer, int bat_num);
  std::vector<PytesEBoxListener *> listeners_{};
  
};

}  // namespace pytes_e_box
}  // namespace esphome
