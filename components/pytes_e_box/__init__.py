import logging
import esphome.codegen as cg
from esphome.components import uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_NAME,
)

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@oxynatOr"]
DEPENDENCIES = ["uart"]
#AUTO_LOAD = ["sensor"]
MULTI_CONF = True

## Helper
CV_NUM_BATTERIES = cv.int_range(1, 16)
CV_NUM_CELLS = cv.int_range(0, 15)




# ICON_CONF_BASE_STATE          = "base_state"
# ICON_CONF_VOLTAGE_STATE       = "voltage_state"
# ICON_CONF_CURRENT_STATE       = "current_state"
# ICON_CONF_TEMPERATURE_STATE   = "temperature_state"
# ICON_CONF_BARCODE             = "barcode"
# ICON_CONF_DEV_TYPE            = "dev_type"
# ICON_CONF_FIRM_VERSION        = "firm_version"
# ICON_CONF_COULOMB_STATUS      = "coulomb_status"
# ICON_CONF_BAT_STATUS          = "bat_status"
# ICON_CONF_CMOS_STATUS         = "cmos_status"
# ICON_CONF_DMOS_STATUS         = "dmos_status"
# ICON_CONF_BAT_PROTECT_ENA     = "bat_protect_ena"
# ICON_CONF_PWR_PROTECT_ENA     = "pwr_protect_ena"
# ICON_CONF_BAT_EVENTS          = "bat_events"
# ICON_CONF_POWER_EVENTS        = "power_events"
# ICON_CONF_SYSTEM_FAULT        = "system_fault"


# ICON_CONF_COULOMB            = "coulomb"
# ICON_CONF_TEMPERATURE_LOW    = "temperature_low"
# ICON_CONF_TEMPERATURE_HIGH   = "temperature_high"
# ICON_CONF_VOLTAGE_LOW        = "voltage_low"
# ICON_CONF_VOLTAGE_HIGH       = "voltage_high"

# ICON_CONF_SOC_VOLTAGE        = "soc_voltage"
# ICON_CONF_TOTAL_COULOMB      = "total_coulomb"
# ICON_CONF_REAL_COULOMB       = "real_coulomb"
# ICON_CONF_TOTAL_POWER_IN     = "total_power_in"
# ICON_CONF_TOTAL_POWER_OUT    = "total_power_out"
# ICON_CONF_WORK_STATUS        = "work_status"
# ICON_CONF_CELLS              = "cell_count"

# ICON_CONF_CELL_VOLTAGE       = "voltage"
# ICON_CONF_CELL_TEMPERATURE   = "temperature"
# ICON_CONF_CELL_COULOMB       = "coulomb"
# ICON_CONF_CELL_CURRENT       = "current"

# ##Icons
# ICON_CURRENT_DC = "mdi:current-dc"
# ICON_MIN_VOLTAGE_CELL = "mdi:battery-minus-outline"
# ICON_MAX_VOLTAGE_CELL = "mdi:battery-plus-outline"
# ICON_BATTERY_STRINGS = "mdi:car-battery"
# ICON_CAPACITY_REMAINING_DERIVED = "mdi:battery-50"
# ICON_ACTUAL_BATTERY_CAPACITY = "mdi:battery-50"
# ICON_TOTAL_BATTERY_CAPACITY_SETTING = "mdi:battery-sync"
# ICON_DEVICE_ADDRESS = "mdi:identifier"
# ICON_ERRORS_BITMASK = "mdi:alert-circle-outline"
# ICON_OPERATION_MODE_BITMASK = "mdi:heart-pulse"
# ICON_CHARGING_CYCLES = "mdi:battery-sync"
# ICON_ALARM_LOW_VOLUME = "mdi:volume-high"

##Custom 
UNIT_AMPERE_HOURS = "Ah"

pytes_e_box_ns = cg.esphome_ns.namespace("pytes_e_box")
PytesEBoxComponent = pytes_e_box_ns.class_("PytesEBoxComponent", cg.PollingComponent, uart.UARTDevice)
CONF_BATTERIES_COMPONENT = "batteries"
CONF_POLL_TIMEOUT = "poll_timeout"
CONF_CMD_IDLE_TIME = "command_idle_time" 
CONF_PYTES_E_BOX_ID = "pytes_e_box_id"
CONF_CELL = "cell"
CONF_BATTERY = "battery"
CONF_CELL_ARRAYS = "cells"


#Sensors
PytesEBoxBatteryTextSensor = pytes_e_box_ns.class_("PytesEBoxBatteryTextSensor", cg.Component)
PytesEBoxBatteryCellTextSensor = pytes_e_box_ns.class_("PytesEBoxBatteryCellTextSensor", cg.Component)
PytesEBoxBatterySensor = pytes_e_box_ns.class_("PytesEBoxBatterySensor", cg.Component)
PytesEBoxBatteryCellSensor = pytes_e_box_ns.class_("PytesEBoxBatteryCellSensor", cg.Component)


PYTES_E_BOX_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYTES_E_BOX_ID): cv.use_id(PytesEBoxComponent),
    }
)


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PytesEBoxComponent),
            cv.Required(CONF_BATTERIES_COMPONENT): CV_NUM_BATTERIES,
            cv.Required(CONF_POLL_TIMEOUT): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_CMD_IDLE_TIME): cv.positive_time_period_milliseconds,
        }
    )
    .extend(cv.polling_component_schema("15s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_system_battery_count(config[CONF_BATTERIES_COMPONENT]))
    cg.add(var.set_polling_timeout(config[CONF_POLL_TIMEOUT]))
    cg.add(var.set_cmd_idle_time(config[CONF_CMD_IDLE_TIME]))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
