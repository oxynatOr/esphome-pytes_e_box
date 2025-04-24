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


ESPHome Setup
-------------

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

