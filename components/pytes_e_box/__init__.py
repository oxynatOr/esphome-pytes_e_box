import esphome.codegen as cg
from esphome.components import binary_sensor, sensor, uart
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_TYPE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_POWER,
    ENTITY_CATEGORY_DIAGNOSTIC,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_PERCENT,
    UNIT_WATT,
)

CODEOWNERS = ["@oxynatOr"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "binary_sensor"]
MULTI_CONF = True

CV_NUM_BATTERIES = cv.int_range(1, 16)
CV_NUM_CELLS = cv.int_range(0, 15)

UNIT_AMPERE_HOURS = "Ah"

pytes_e_box_ns = cg.esphome_ns.namespace("pytes_e_box")
PytesEBoxComponent = pytes_e_box_ns.class_("PytesEBoxComponent", cg.PollingComponent, uart.UARTDevice)

# Selectable BMS protocol driver. Add new entries here (and a matching C++ driver)
# to support additional battery types.
BmsType = pytes_e_box_ns.enum("BmsType", is_class=True)
BMS_TYPES = {
    "pytes_e_box": BmsType.PYTES_E_BOX,
    "example": BmsType.EXAMPLE,
    "pylontech": BmsType.PYLONTECH,
    "pytes_lv1": BmsType.PYTES_LV1,
}
CONF_BATTERIES_COMPONENT = "batteries"
CONF_POLL_TIMEOUT = "poll_timeout"
CONF_CMD_IDLE_TIME = "command_idle_time"
CONF_MAX_RETRIES = "max_retries"
CONF_PYTES_E_BOX_ID = "pytes_e_box_id"
CONF_CELL = "cell"
CONF_BATTERY = "battery"
CONF_CELL_ARRAYS = "cells"

CONF_CONNECTED = "connected"
CONF_ERROR_COUNT = "error_count"
CONF_TOTAL_CURRENT = "total_current"
CONF_TOTAL_POWER = "total_power"
CONF_AVERAGE_SOC = "average_soc"

PytesEBoxBatteryTextSensor = pytes_e_box_ns.class_("PytesEBoxBatteryTextSensor", cg.Component)
PytesEBoxBatteryCellTextSensor = pytes_e_box_ns.class_("PytesEBoxBatteryCellTextSensor", cg.Component)
PytesEBoxBatterySensor = pytes_e_box_ns.class_("PytesEBoxBatterySensor", cg.Component)
PytesEBoxBatteryCellSensor = pytes_e_box_ns.class_("PytesEBoxBatteryCellSensor", cg.Component)


PYTES_E_BOX_COMPONENT_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_PYTES_E_BOX_ID): cv.use_id(PytesEBoxComponent),
    }
)

HUB_SENSORS: dict[str, cv.Schema] = {
    CONF_ERROR_COUNT: sensor.sensor_schema(
        accuracy_decimals=0,
        state_class=STATE_CLASS_TOTAL_INCREASING,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
        icon="mdi:alert-circle-outline",
    ),
    CONF_TOTAL_CURRENT: sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_TOTAL_POWER: sensor.sensor_schema(
        unit_of_measurement=UNIT_WATT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_AVERAGE_SOC: sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PytesEBoxComponent),
            cv.Optional(CONF_TYPE, default="pytes_e_box"): cv.enum(BMS_TYPES, lower=True),
            cv.Required(CONF_BATTERIES_COMPONENT): CV_NUM_BATTERIES,
            cv.Required(CONF_POLL_TIMEOUT): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_CMD_IDLE_TIME, default="150ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_MAX_RETRIES, default=2): cv.int_range(0, 10),
            cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_CONNECTIVITY,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            ),
        }
    )
    .extend({cv.Optional(key): schema for key, schema in HUB_SENSORS.items()})
    .extend(cv.polling_component_schema("15s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

FINAL_VALIDATE_SCHEMA = uart.final_validate_device_schema(
    "pytes_e_box", baud_rate=115200, require_tx=True, require_rx=True, parity="NONE", stop_bits=1, data_bits=8
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_driver_type(config[CONF_TYPE]))
    cg.add(var.set_system_battery_count(config[CONF_BATTERIES_COMPONENT]))
    cg.add(var.set_polling_timeout(config[CONF_POLL_TIMEOUT]))
    cg.add(var.set_cmd_idle_time(config[CONF_CMD_IDLE_TIME]))
    cg.add(var.set_max_retries(config[CONF_MAX_RETRIES]))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if conf := config.get(CONF_CONNECTED):
        sens = await binary_sensor.new_binary_sensor(conf)
        cg.add(var.set_connected_binary_sensor(sens))
    for key in HUB_SENSORS:
        if conf := config.get(key):
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(var, f"set_{key}_sensor")(sens))
