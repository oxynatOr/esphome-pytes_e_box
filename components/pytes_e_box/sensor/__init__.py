#import logging
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_VOLTAGE,
    CONF_CURRENT,
    CONF_TEMPERATURE,
    CONF_ID,
    CONF_NAME,    
    UNIT_VOLT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    STATE_CLASS_MEASUREMENT,
    DEVICE_CLASS_EMPTY,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_BATTERY
    )

from .. import (pytes_e_box_ns ,CONF_PYTES_E_BOX_ID, PYTES_E_BOX_COMPONENT_SCHEMA, CONF_CELL, PytesEBoxBatteryCellSensor, PytesEBoxBatterySensor,
                CONF_BATTERY, CONF_CELL_ARRAYS, CV_NUM_CELLS, CV_NUM_BATTERIES, UNIT_AMPERE_HOURS
                )



CONF_COULOMB            = "coulomb"
CONF_TEMPERATURE_LOW    = "temperature_low"
CONF_TEMPERATURE_HIGH   = "temperature_high"
CONF_VOLTAGE_LOW        = "voltage_low"
CONF_VOLTAGE_HIGH       = "voltage_high"

CONF_SOC_VOLTAGE        = "soc_voltage"
CONF_TOTAL_COULOMB      = "total_coulomb"
CONF_REAL_COULOMB       = "real_coulomb"
CONF_TOTAL_POWER_IN     = "total_power_in"
CONF_TOTAL_POWER_OUT    = "total_power_out"
CONF_WORK_STATUS        = "work_status"
CONF_CELLS              = "cell_count"

CONF_CELL_VOLTAGE       = "voltage"
CONF_CELL_TEMPERATURE   = "temperature"
CONF_CELL_COULOMB       = "coulomb"
CONF_CELL_CURRENT       = "current"


BAT_TYPES: dict[str, cv.Schema] = {
    CONF_VOLTAGE: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_CURRENT: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_TEMPERATURE: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_TEMPERATURE_LOW: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_TEMPERATURE_HIGH: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),
    CONF_VOLTAGE_LOW: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_VOLTAGE_HIGH: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_COULOMB: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    CONF_SOC_VOLTAGE: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),    
    CONF_WORK_STATUS: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_EMPTY,
    ),
    CONF_CELLS: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_EMPTY,
    ),
    CONF_TOTAL_COULOMB: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_AMPERE_HOURS,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_EMPTY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_REAL_COULOMB: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_AMPERE_HOURS,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_EMPTY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_TOTAL_POWER_IN: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_AMPERE_HOURS,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_EMPTY,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_TOTAL_POWER_OUT: sensor.sensor_schema(
        #PytesEBoxBatterySensor,
        unit_of_measurement=UNIT_AMPERE_HOURS,
        device_class=DEVICE_CLASS_EMPTY,
        state_class=STATE_CLASS_MEASUREMENT,
        accuracy_decimals=3,
    ),
}

CELL_TYPES: dict[str, cv.Schema] = {
    CONF_VOLTAGE: sensor.sensor_schema(
        #PytesEBoxBatteryCellSensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
    ),
    CONF_CURRENT: sensor.sensor_schema(
        #PytesEBoxBatteryCellSensor,
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
    ),
    CONF_TEMPERATURE: sensor.sensor_schema(
        #PytesEBoxBatteryCellSensor,
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
    ),  
    CONF_COULOMB: sensor.sensor_schema(
        #PytesEBoxBatteryCellSensor,
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
    ),
    #unit_of_measurement=UNIT_AMPERE_HOURS,
    #device_class=DEVICE_CLASS_EMPTY,
    #state_class=STATE_CLASS_MEASUREMENT,    
}


CELLS_ARRAYS_SCHEMA = cv.ensure_list(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PytesEBoxBatteryCellSensor),
            cv.Required(CONF_CELL): CV_NUM_CELLS,
            cv.Optional(CONF_NAME): cv.string_strict,
        }
    ).extend({cv.Optional(marker): schema for marker, schema in CELL_TYPES.items()})
)

CONFIG_SCHEMA = PYTES_E_BOX_COMPONENT_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(PytesEBoxBatterySensor),
            cv.Required(CONF_BATTERY): CV_NUM_BATTERIES,
            cv.Optional(CONF_NAME): cv.string_strict,
            cv.Optional(CONF_CELL_ARRAYS): CELLS_ARRAYS_SCHEMA,
        }
    ).extend({cv.Optional(marker): schema for marker, schema in BAT_TYPES.items()})
           

async def to_code(config):
    paren = await cg.get_variable(config[CONF_PYTES_E_BOX_ID])

    if CONF_BATTERY in config:
        bat = cg.new_Pvariable(config[CONF_ID], config[CONF_BATTERY])
        for marker in BAT_TYPES:
            if marker_config := config.get(marker):
                sens = await sensor.new_sensor(marker_config)
                cg.add(getattr(bat, f"set_{marker}_sensor")(sens))
        cg.add(paren.register_listener(bat))    

    if CONF_CELL_ARRAYS in config:
        for cells_config in config[CONF_CELL_ARRAYS]:
            cell_var = cg.new_Pvariable(cells_config[CONF_ID], config[CONF_BATTERY], cells_config[CONF_CELL])
            for marker, schema in CELL_TYPES.items():
                if marker in cells_config:
                    sens = await sensor.new_sensor(cells_config[marker])
                    cg.add(getattr(cell_var, f"set_{marker}_sensor")(sens))
            cg.add(paren.register_listener(cell_var))