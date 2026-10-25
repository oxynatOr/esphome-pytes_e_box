import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_CURRENT,
    CONF_ID,
    CONF_POWER,
    CONF_TEMPERATURE,
    CONF_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
)

from .. import (
    CONF_BATTERY,
    CONF_CELL,
    CONF_CELL_ARRAYS,
    CONF_PYTES_E_BOX_ID,
    CV_NUM_BATTERIES,
    CV_NUM_CELLS,
    PYTES_E_BOX_COMPONENT_SCHEMA,
    UNIT_AMPERE_HOURS,
    PytesEBoxBatteryCellSensor,
    PytesEBoxBatterySensor,
)

CONF_COULOMB = "coulomb"
CONF_TEMPERATURE_LOW = "temperature_low"
CONF_TEMPERATURE_HIGH = "temperature_high"
CONF_MOS_TEMPERATURE = "mos_temperature"
CONF_VOLTAGE_LOW = "voltage_low"
CONF_VOLTAGE_HIGH = "voltage_high"

CONF_SOC_VOLTAGE = "soc_voltage"
CONF_TOTAL_COULOMB = "total_coulomb"
CONF_REAL_COULOMB = "real_coulomb"
CONF_TOTAL_POWER_IN = "total_power_in"
CONF_TOTAL_POWER_OUT = "total_power_out"
CONF_WORK_STATUS = "work_status"
CONF_CELLS = "cell_count"

CONF_CELL_VOLTAGE_MIN = "cell_voltage_min"
CONF_CELL_VOLTAGE_MAX = "cell_voltage_max"
CONF_CELL_VOLTAGE_DELTA = "cell_voltage_delta"


def _voltage(decimals=3):
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=decimals,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _current():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _temperature():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_CELSIUS,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_TEMPERATURE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _soc():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_PERCENT,
        accuracy_decimals=0,
        device_class=DEVICE_CLASS_BATTERY,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def _ampere_hours():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE_HOURS,
        accuracy_decimals=3,
        state_class=STATE_CLASS_MEASUREMENT,
    )


BAT_TYPES: dict[str, cv.Schema] = {
    CONF_VOLTAGE: _voltage(),
    CONF_CURRENT: _current(),
    CONF_TEMPERATURE: _temperature(),
    CONF_TEMPERATURE_LOW: _temperature(),
    CONF_TEMPERATURE_HIGH: _temperature(),
    CONF_MOS_TEMPERATURE: _temperature(),
    CONF_VOLTAGE_LOW: _voltage(),
    CONF_VOLTAGE_HIGH: _voltage(),
    CONF_COULOMB: _soc(),
    CONF_SOC_VOLTAGE: _voltage(),
    CONF_WORK_STATUS: sensor.sensor_schema(accuracy_decimals=0),
    CONF_CELLS: sensor.sensor_schema(accuracy_decimals=0),
    CONF_TOTAL_COULOMB: _ampere_hours(),
    CONF_REAL_COULOMB: _ampere_hours(),
    CONF_TOTAL_POWER_IN: _ampere_hours(),
    CONF_TOTAL_POWER_OUT: _ampere_hours(),
    CONF_POWER: sensor.sensor_schema(
        unit_of_measurement=UNIT_WATT,
        accuracy_decimals=1,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    ),
    CONF_CELL_VOLTAGE_MIN: _voltage(),
    CONF_CELL_VOLTAGE_MAX: _voltage(),
    CONF_CELL_VOLTAGE_DELTA: _voltage(),
}

CELL_TYPES: dict[str, cv.Schema] = {
    CONF_VOLTAGE: _voltage(),
    CONF_CURRENT: _current(),
    CONF_TEMPERATURE: _temperature(),
    CONF_COULOMB: _soc(),
}


CELLS_ARRAYS_SCHEMA = cv.ensure_list(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(PytesEBoxBatteryCellSensor),
            cv.Required(CONF_CELL): CV_NUM_CELLS,
        }
    ).extend({cv.Optional(marker): schema for marker, schema in CELL_TYPES.items()})
)

CONFIG_SCHEMA = PYTES_E_BOX_COMPONENT_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PytesEBoxBatterySensor),
        cv.Required(CONF_BATTERY): CV_NUM_BATTERIES,
        cv.Optional(CONF_CELL_ARRAYS): CELLS_ARRAYS_SCHEMA,
    }
).extend({cv.Optional(marker): schema for marker, schema in BAT_TYPES.items()})


async def to_code(config):
    paren = await cg.get_variable(config[CONF_PYTES_E_BOX_ID])

    bat = cg.new_Pvariable(config[CONF_ID], config[CONF_BATTERY])
    await cg.register_component(bat, config)
    for marker in BAT_TYPES:
        if marker_config := config.get(marker):
            sens = await sensor.new_sensor(marker_config)
            cg.add(getattr(bat, f"set_{marker}_sensor")(sens))
    cg.add(paren.register_listener(bat))

    for cells_config in config.get(CONF_CELL_ARRAYS, []):
        cell_var = cg.new_Pvariable(cells_config[CONF_ID], config[CONF_BATTERY], cells_config[CONF_CELL])
        await cg.register_component(cell_var, cells_config)
        for marker in CELL_TYPES:
            if marker in cells_config:
                sens = await sensor.new_sensor(cells_config[marker])
                cg.add(getattr(cell_var, f"set_{marker}_sensor")(sens))
        cg.add(paren.register_listener(cell_var))
