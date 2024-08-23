#include "pytes_e_box.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <stdlib.h>
#include <regex>

namespace esphome {
namespace pytes_e_box {

static const int MAX_DATA_LENGTH_BYTES = 4096;
static const int MAX_DATA_LINE_LENGTH = 256;
static const uint8_t ASCII_LF = 0x0A;
static const uint8_t ASCII_CR = 0x0D;
ENUMCommand _cmd_result = CMD_PWR;
ENUMCommand _last_cmd = CMD_NIL;
int pwrcount = 0;
int recv = 0;
int jobCount = 0;
int __run_ = 0;
bool praseData = false;
std::regex pattern("(^\\s|\\s{2,})");

PytesEBoxComponent::PytesEBoxComponent() {}

void PytesEBoxComponent::dump_config() {
  this->check_uart_settings(115200, 1, esphome::uart::UART_CONFIG_PARITY_NONE, 8);
  ESP_LOGCONFIG(TAG, "PytesEBox:");
  ESP_LOGCONFIG(TAG, "  Batteries: %d", this->battaries_in_system_);
  ESP_LOGCONFIG(TAG, "  Poll Timeout: %d", this->polling_timeout_);
  //ESP_LOGCONFIG(TAG, "  Command Idle Time: %d", this->polling_timeout_);
  ESP_LOGCONFIG(TAG, "  Commands in Queue: %d", this->cmd_queue_.size());
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Connection with PytesEBox failed!");
  }
}

void PytesEBoxComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up PytesEBox...");
  this->state_ = STATE_WAIT;
  this->clear_uart_buffer();
  this->last_poll_ = -1;
  this->add_polling_command_("pwr",0,CMD_PWR);
  std::string cmd;
  for (int i = 1; i <= this->battaries_in_system_; i++) {
    cmd = "pwr "+to_string(i);
    this->add_polling_command_(cmd.c_str(),i,CMD_PWR_INDEX);
    cmd = "bat "+to_string(i);
    this->add_polling_command_(cmd.c_str(),i,CMD_BAT_INDEX);
  }  
  if (this->is_ready()) { this->state_ = STATE_IDLE; }
}

void PytesEBoxComponent::add_polling_command_(const char *command, int _index, ENUMCommand polling_command) {
  for (auto &_command : this->cmd_queue_) {
    if (_command.command.c_str() == command) { 
      //all ready added, just to be sure we wont take more we need.
      return;
      }
  }
  PollingCommand _cmd_;
  _cmd_.command = command;
  _cmd_.errors = 0;
  _cmd_.index = _index;
  _cmd_.identifier = polling_command;
  _cmd_.length = strlen(command);  
  cmd_queue_.push_back(_cmd_);
  command_queue_max_++;
}

uint8_t PytesEBoxComponent::send_next_command_() {
    if (this->cmd_queue_[this->command_queue_position_].command != "") {
    const char *command = this->cmd_queue_[this->command_queue_position_].command.c_str();
    this->buffer_index_read_ = 0;
    this->buffer_index_write_ = 0;
    this->clear_uart_buffer();
    this->pwr_index_l = {};
    this->bat_index_l = {};
    this->pwr_data_l = {};
    this->write_str(this->cmd_queue_[this->command_queue_position_].command.c_str());
    this->write_str("\n");
    this->last_poll_ = millis();
    //this->command_start_millis_ = millis();
    this->state_ = STATE_POLL;
    ESP_LOGD(TAG, "Sending command from queue: %s from index: %d", this->cmd_queue_[this->command_queue_position_].command.c_str(), this->command_queue_position_);
    return 1;
  }
  return 0;
}


void PytesEBoxComponent::update() {
  if (this->state_ == STATE_IDLE) {
      this->command_queue_position_ = 0;
      if (this->send_next_command_() == 0) {
        ESP_LOGE(TAG, "Command failed: %s",this->cmd_queue_[this->command_queue_position_].command.c_str());
        return;
    }
  }
}

/* only 1-line per run, othertwise we will block the component. */
void PytesEBoxComponent::loop() {
  /** nothing to do, keep chilling */
  if (this->state_ == STATE_IDLE || this->state_ == STATE_WAIT) {
  return;
  }

  /** check if we run into a "deathloop" or something is blocking */
  if (millis() - this->last_poll_ > this->polling_timeout_) {
      this->last_poll_ = millis();
      const char *command = this->cmd_queue_[this->command_queue_position_].command.c_str();
      ESP_LOGE(TAG, "timeout command from queue: %s", command);
      this->clear_uart_buffer();
      //this->command_queue_position_ = (this->command_queue_position_+1) % COMMAND_QUEUE_LENGTH;
      this->state_ = STATE_SEND_NEXT_COMMAND;
      if (this->command_queue_position_ == this->command_queue_max_) { this->state_ = STATE_IDLE; } //this->command_queue_position_ = 0;}
      return;
  }


  /** command queue */
  if (this->state_ == STATE_SEND_NEXT_COMMAND) {
  if (millis() - this->last_poll_ <= this->command_idle_time_) { return; }

    this->command_queue_position_ = (this->command_queue_position_+1) % COMMAND_QUEUE_LENGTH;
    if (this->command_queue_position_ == this->command_queue_max_) { 
      this->state_ = STATE_IDLE; 
      ESP_LOGI(TAG, "PytesEBox command queue done.");
      return; 
      }
    if (this->send_next_command_() == 0) {
      ESP_LOGE(TAG, "Command failed: %s",this->cmd_queue_[this->command_queue_position_].command.c_str());
      return;
    }
  }

  /**  */
  if (this->state_ == STATE_COMMAND_COMPLETE) {
    for (PytesEBoxListener *listener : this->listeners_) {
      switch (_last_cmd) {
        case CMD_PWR: {
            listener->on_pwr_line_read(&pwr_index_l);
          break;
          }
        case CMD_PWR_INDEX: {
          listener->on_pwrn_line_read(&pwr_data_l);
          break;
        }
        case CMD_BAT_INDEX: {
          listener->on_batn_line_read(&bat_index_l);
          break;
          }
        case CMD_BAT:
        case CMD_NIL:
        case CMD_ERROR:
        default:
          this->state_ = STATE_SEND_NEXT_COMMAND;
          break;
      }
  }
    this->state_ = STATE_SEND_NEXT_COMMAND;
    if (this->command_queue_position_ == this->command_queue_max_) { this->state_ = STATE_IDLE; }
    return;
  }

  /** put data into listener */
  if (this->state_ == STATE_COMMAND) {
    if (this->buffer_index_read_ != this->buffer_index_write_) {
      switch (_last_cmd) {
        case CMD_PWR: {
          this->processData_pwrLine(this->buffer_[this->buffer_index_read_]);
          break;
          }
        case CMD_PWR_INDEX: {
          this->processData_pwrIndex(this->buffer_[this->buffer_index_read_],
          this->cmd_queue_[this->command_queue_position_].index);
          break;
        }
        case CMD_BAT_INDEX: {
          this->processData_batIndexLine(this->buffer_[this->buffer_index_read_],
          this->cmd_queue_[this->command_queue_position_].index);
          break;
          }
        case CMD_BAT:
        case CMD_NIL:
        case CMD_ERROR:
        default:
          this->state_ = STATE_SEND_NEXT_COMMAND;
          return;
      }
      if (this->isLineComplete(this->buffer_[buffer_index_read_]) > 0) {
        ESP_LOGVV(TAG, "Command Complete, switch to STATE_COMMAND_COMPLETE");
        this->state_ = STATE_COMMAND_COMPLETE; 
        return;
      }
      buffer_index_read_ = (buffer_index_read_ + 1) % NUM_BUFFERS;
      return;
    }
  }

  /** triming data */
  if (this->state_ == STATE_POLL_DECODED) {
    if (this->buffer_index_read_ != this->buffer_index_write_) {
      switch (_last_cmd) {
        case CMD_PWR_INDEX: {
          this->buffer_[buffer_index_read_] = std::regex_replace(this->buffer_[buffer_index_read_], pattern, "");
          break;
          }
        case CMD_PWR: { this->state_ = STATE_COMMAND; return; }
        case CMD_BAT_INDEX: { this->state_ = STATE_COMMAND; return; }
        case CMD_BAT: { this->state_ = STATE_COMMAND; return; }
        case CMD_NIL:
        case CMD_ERROR:
        default:
          this->state_ = STATE_SEND_NEXT_COMMAND;
          return;
      }      
        if (this->isLineComplete(this->buffer_[buffer_index_read_]) > 0) {
        this->state_ = STATE_COMMAND;
        this->buffer_index_read_ = (3) % NUM_BUFFERS;
        return;
      }
      //ESP_LOGD(TAG, "(%d) %s",this->buffer_index_read_, buffer_[buffer_index_read_].c_str());
      buffer_index_read_ = (buffer_index_read_ + 1) % NUM_BUFFERS;
      return;
    }
  }
  /** read command we send */
  if (this->state_ == STATE_POLL_COMPLETE) {
      _last_cmd = this->readCommand(this->buffer_[0]);
      if (_last_cmd != CMD_NIL || _last_cmd != CMD_ERROR) {
      this->state_ = STATE_POLL_DECODED;
      this->buffer_[0].clear();
      this->buffer_[1].clear();
      this->buffer_[2].clear();
      this->buffer_index_read_ = (3) % NUM_BUFFERS;
      ESP_LOGD(TAG, "parsed command -> %s [Battery: %i]",this->CommandtoString(_last_cmd).c_str(),
                    this->cmd_queue_[this->command_queue_position_].index);
      return;
    } else {
      ESP_LOGE(TAG, "No command parsed: %s ",this->buffer_[0].c_str()); 
      //this->state_ = STATE_IDLE;
      this->state_ = STATE_SEND_NEXT_COMMAND;
      return;
      }
  }

  /** pull all the serial data from buffer */
  if (this->state_ == STATE_POLL) {
    while (this->available()) {
      static char buffer[MAX_DATA_LINE_LENGTH];
      if(readline(read(), buffer, MAX_DATA_LENGTH_BYTES) > 0) {
        this->buffer_[buffer_index_write_] = buffer;
        ESP_LOGV(TAG, "(%d) %s",this->buffer_index_write_, buffer);
      if (this->isLineComplete(this->buffer_[buffer_index_write_]) > 0) {
          this->state_ = STATE_POLL_COMPLETE;
          ESP_LOGVV(TAG, "Data Complete");
        }
        this->buffer_index_write_ = (this->buffer_index_write_ + 1) % NUM_BUFFERS;
      } // readLine
    } //while available 
  }/** Read UART Buffer End*/
}

void PytesEBoxComponent::processData_batIndexLine(std::string &buffer, int bat_num) {
  if (isdigit(buffer[0])) {
    PytesEBoxListener::bat_index_LineContents l{};
    
    const int parsed = sscanf(                                                                      // NOLINT
      buffer.c_str(),"%d %d %d %7s %7s %7s %7s %d%% %d",                                            // NOLINT
      &l.cell_num, &l.cell_volt, &l.cell_tempr, l.cell_baseState, l.cell_voltState,                 // NOLINT
      l.cell_currState, l.cell_tempState, &l.cell_coulomb, &l.cell_curr);                           // NOLINT
    
    
    if (parsed != 9) {
      ESP_LOGE(TAG, "invalid line: found only %d, should be 9 items. in line %d\n: %s",
                    parsed, l.cell_num, buffer.substr(0, buffer.size() - 2).c_str());
      return;
    }
      
    /*
    if (l.cell_num <= 0) {
        ESP_LOGE(TAG, "invalid cell_num in line %s", buffer.substr(0, buffer.size() - 2).c_str());
    return;
    } 
    */

    l.bat_num = bat_num;

    for (PytesEBoxListener *listener : this->listeners_) {
      listener->on_batn_line_read(&l);
    }
  }
}

void PytesEBoxComponent::processData_pwrLine(std::string &buffer) {
  if (isdigit(buffer[0]) && (buffer.find("Absent") == -1)) {
  PytesEBoxListener::pwr_LineContents l{};
  const int parsed = sscanf(                                                                     // NOLINT
    buffer.c_str(),"%d %d %d %d %d %d %d %d %7s %7s %7s %7s %d%% %d-%d-%d %d:%d:%d %s %s %s %s", // NOLINT
    &l.bat_num, &l.voltage, &l.current, &l.temperature, &l.tlow, &l.thigh, &l.vlow, &l.vhigh,    // NOLINT
    l.base_st, l.volt_st, l.curr_st, l.temp_st, &l.coulomb, &l.day, &l.month, &l.year, &l.hour,  // NOLINT
    &l.min, &l.sec, l.bv_st, l.bt_st,l.serial_st ,l.devtype_st);                                 // NOLINT

    if (parsed != 23) {
      ESP_LOGE(TAG, "invalid line: found only %d, should be 23 items. in line %d\n: %s",
                    parsed, l.bat_num, buffer.substr(0, buffer.size() - 2).c_str());
      return;
    }

    if (l.bat_num <= 0) {
      ESP_LOGE(TAG, "invalid bat_num in line %s", buffer.substr(0, buffer.size() - 2).c_str());
    return;
    }
    if (l.bat_num > this->found_battaries_in_system_  ) {this->found_battaries_in_system_ = l.bat_num;}

    for (PytesEBoxListener *listener : this->listeners_) {
      listener->on_pwr_line_read(&l);
    }
  }  
}



void PytesEBoxComponent::processData_pwrIndex(std::string &buffer, int bat_num) {  
pwr_data_l.bat_num = bat_num;
int i = this->buffer_index_read_;

  if (this->buffer_[i].rfind("SOC Voltage:", 0) == 0) {
    sscanf(this->buffer_[i].c_str(),"SOC Voltage: %dmV",&pwr_data_l.SOCVoltage);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.SOCVoltage);
  }

  if (this->buffer_[i].rfind("Total Coulomb:", 0) == 0) {
    sscanf(this->buffer_[i].c_str(),"Total Coulomb: %dmAS",&pwr_data_l.totalCoulomb);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.totalCoulomb);
  }

  if (this->buffer_[i].rfind("Real Coulomb:", 0) == 0) {
    sscanf(this->buffer_[i].c_str(),"Real Coulomb: %dmAH",&pwr_data_l.realCoulomb);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.realCoulomb);
  }

  if (this->buffer_[i].rfind("Total Power In:", 0) == 0) { 
    sscanf(this->buffer_[i].c_str(),"Total Power In: %dAS",&pwr_data_l.totalPowerIn);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.totalPowerIn);
  }
  
  if (this->buffer_[i].rfind("Work Status:", 0) == 0) {
    sscanf(this->buffer_[i].c_str(),"Work Status: %d",&pwr_data_l.workStatus);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.workStatus);
  }

  if (this->buffer_[i].rfind("Bat Num:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Bat Num: %d",&pwr_data_l.cells);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.cells);
  }

  if (this->buffer_[i].rfind("Total Power Out :", 0) == 0) {
    sscanf(this->buffer_[i].c_str(),"Total Power Out : %dAS ",&pwr_data_l.totalPowerOut);
    ESP_LOGV(TAG,"%s -> %d",this->buffer_[i].c_str(),pwr_data_l.totalPowerOut);
  }

  if (this->buffer_[i].rfind("Barcode:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Barcode: %[^\n]",pwr_data_l.Barcode);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.Barcode);
  }

  if (this->buffer_[i].rfind("Firm Version:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Firm Version: %[^\n]",pwr_data_l.FirmVersion); 
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.FirmVersion);
  }

  if (this->buffer_[i].rfind("Coul. Status:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Coul. Status: %[^\n]",pwr_data_l.CoulStatus);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.CoulStatus);
  }

  if (this->buffer_[i].rfind("Bat Status:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Bat Status: %[^\n]",pwr_data_l.BatStatus);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.BatStatus);
  }
  
  if (this->buffer_[i].rfind("CMOS Status:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," CMOS Status: %[^\n]",pwr_data_l.CMOSStatus);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.CMOSStatus);
  }

  if (this->buffer_[i].rfind("DMOS Status:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," DMOS Status: %[^\n]",pwr_data_l.DMOSStatus);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.DMOSStatus);
  }
  
  if (this->buffer_[i].rfind("Bat Protect ENA :", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Bat Protect ENA : %[^\n]",pwr_data_l.BatProtectENA);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.BatProtectENA);
  }
    
  if (this->buffer_[i].rfind("Pwr Protect ENA :", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Pwr Protect ENA : %[^\n]",pwr_data_l.PwrProtectENA);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.PwrProtectENA);
  }
    
  if (this->buffer_[i].rfind("Bat Events:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Bat Events: %[^\n]",pwr_data_l.BatEvents);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.BatEvents);
  }

  if (this->buffer_[i].rfind("Power Events:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," Power Events: %[^\n]",pwr_data_l.PowerEvents);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.PowerEvents);
  }

  if (this->buffer_[i].rfind("System Fault:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," System Fault: %[^\n]",pwr_data_l.SystemFault);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.SystemFault);
  }

  if (this->buffer_[i].rfind("DevType:", 0) == 0) {
    sscanf(this->buffer_[i].c_str()," DevType: %[^\n]",pwr_data_l.DevType);
    ESP_LOGV(TAG,"%s -> %s",this->buffer_[i].c_str(),pwr_data_l.DevType);
  }

  //ESP_LOGD(TAG,"%s",this->buffer_[i].c_str());

/** already in pwr
  if (this->buffer_[i].rfind("Coulomb:", 0) == 0) sscanf(this->buffer_[i].c_str(),"Coulomb: %d%%",&pwr_data_l.coulomb);  
  if (this->buffer_[i].rfind("Basic Status:", 0) == 0) sscanf(this->buffer_[i].c_str()," Basic Status: %s",pwr_data_l.BasicStatus);
  if (this->buffer_[i].rfind("Volt Status:", 0) == 0) sscanf(this->buffer_[i].c_str()," Volt Status: %s",pwr_data_l.VoltStatus);
  if (this->buffer_[i].rfind("Current Status:", 0) == 0) sscanf(this->buffer_[i].c_str()," Current Status: %s",pwr_data_l.CurrentStatus);
  if (this->buffer_[i].rfind("Tmpr. Status:", 0) == 0) sscanf(this->buffer_[i].c_str()," Tmpr. Status: %s",pwr_data_l.TmprStatus);
*/
}

void PytesEBoxComponent::clear_uart_buffer() {
  uint8_t byte;
  this->flush();
  while (this->available()) {
    this->read_byte(&byte);
  }
}
float PytesEBoxComponent::get_setup_priority() const { return setup_priority::DATA; }

}  // namespace pytes_e_box
}  // namespace esphome
