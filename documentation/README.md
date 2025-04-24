Pytes E-Box
=================
The PytesEbox component allows you to pull data from Pytes Batteries into ESPHome.
It uses ***UART*** for communication.

Once configured, you can use sensors as described below for your projects.

![pytesebox](https://github.com/user-attachments/assets/699cedf4-fe41-476b-9a39-41ebb7c520f5)


Instructions for setting up Pytes E-Box in ESPHome.

Hardware Setup
--------------

You can connect to Pytes E-Box using the port labeled ***Console***.
Any connections via CAN or RS485 (e.g. to an inverter) are untouched and remain functional.

The console port offers a RS232 interface using a RJ45 connector.
The voltage levels are *not* TTL-compatible. A RS232 transceiver must be placed between the Batteries and the ESPHome device.
MAX3232-based transceivers have been tested and work well.

If you have multiple batteries you need to connect to the master battery's console port.

![rj45_pinout](https://github.com/user-attachments/assets/cb4f9808-333d-4344-b02e-18e8ffaf3341)
| ESP Pin | Transceiver | RJ45 Pin | Function |
| --- | --- | --- | --- |
| GPIO 6 | RX | ***3*** | TX |
| GND | GND | ***4*** | Ground |
| GPIO 5 | TX | ***6*** | RX |
| 3v3 | VCC | ***NC*** | Power |


Tested devcies:
------------------------
| Manufacturer | Devcie |
| --- | --- |
| Pytes | E-BOX-48100R-C |


Component/Hub
-------------

```yaml
pytes_e_box:
```

Configuration variables:
------------------------
- **id** (***Required***): The id to use for this PytesEbox component.
- **uart_id** (*Optional*): The uart Bus ID.
- **batteries** (***Required***): Amount of Batteries. Defaults to ``1``
- **update_interval** (*Optional*): The interval to check the sensor. Defaults to ``60s``.
- **poll_timeout** (*Optional*): --. Defaults to ``4s``.
- **command_idle_time** (*Optional*): --. Defaults to ``150ms``. 

Sensor
------

All values are reported for every Pytes E-Box battery individually.

```yaml
# Example configuration entry
sensor:
  - platform: pytes_e_box
    pytes_e_box_id: pytes01
    battery: 1
    cells:
     - cell: 1
        voltage:
          name: "BatteryCell 1.1 Voltage"         
    voltage:
      name: "Battery1 Voltage"
    current:
      name: "Battery1 Current"
    coulomb:
      name: "Battery1 State of Charge"
```

Configuration variables:
------------------------
- **pytes_e_box_id** (***Required***): Manually specify the ID of the pytes instance if there are multiple.
- **battery** (***Required***): Which battery to monitor. 1 stands for the main battery, 2..6 for child batteries.
- **voltage** (*Optional*): Voltage of the battery. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **current** (*Optional*): Current flowing into the battery. Negative when discharging. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **coulomb** (*Optional*): State of Charge in percent. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **temperature** (*Optional*): Temperature. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **temperature_low** (*Optional*): Historic minimum temperature. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **temperature_high** (*Optional*): Historic maximum temperature. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **voltage_low** (*Optional*): Voltage of the lowest cell. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **voltage_high** (*Optional*): Voltage of the highest cell. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **soc_voltageh** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **total_coulomb** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **real_coulomb** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **total_power_in** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **total_power_out** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **work_status** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
- **cell_count** (*Optional*): -- All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).

- **cells** (*Optional*): Dictionary of Battery cells.
  - **cell** (***Required***): Which battery cell to monitor. 0 to 15. 
    - **voltage** (*Optional*): Voltage of the battery. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
    - **current** (*Optional*): Current flowing into the battery. Negative when discharging. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
    - **coulomb** (*Optional*): State of Charge in percent. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).
    - **temperature** (*Optional*): Temperature. All options from [Sensor](https://esphome.io/components/sensor/#config-sensor).


Text Sensor
-----------

```yaml
text_sensor:
  - platform: pytes_e_box
    pytes_e_box_id: pytes01
    battery: 1    
    base_state:
      id: bat1_base_state
      name: "Battery1 Base State"
```

Configuration variables:
------------------------
- **pytes_e_box_id** (**Required**): Manually specify the ID of the pytes instance if there are multiple.
- **battery** (**Required**): Which battery to monitor. 1 stands for the main battery, 2..6 for child batteries.
- **base_state** (*Optional*): Base state. Usually reads ``Dischg``, ``Charge`` or ``Idle``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **voltage_state** (*Optional*): Voltage state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **current_state** (*Optional*): Current state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **temperature_state** (*Optional*): Temperature state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **barcode** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **dev_type** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **firm_version** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **coulomb_status** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **bat_status** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **cmos_status** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **dmos_status** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **bat_protect_ena** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **pwr_protect_ena** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **bat_events** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **power_events** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
- **system_fault** (*Optional*): -- . All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).

- **cells** (*Optional*): Dictionary of Battery cells.
  - **cell** (***Required***): Which battery cell to monitor. 0 to 15. 
    - **base_state** (*Optional*): Base state. Usually reads ``Dischg``, ``Charge`` or ``Idle``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
    - **voltage_state** (*Optional*): Voltage state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
    - **current_state** (*Optional*): Current state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
    - **temperature_state** (*Optional*): Temperature state. Usually reads ``Normal``. All options from [TextSensor](https://esphome.io/components/text_sensor/#config-text-sensor).
