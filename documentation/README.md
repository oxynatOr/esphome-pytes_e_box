<a id="docs-top"></a>

<h1 align="center">Pytes E-Box <sup>Docs</sup></h1>
<p align="center">
  Hardware, console protocol and background for the <code>pytes_e_box</code> ESPHome component.
</p>

This folder holds everything that does not fit into the main
[README](../README.md). The configuration reference (hub, sensors, text
sensors) lives there, so it only has to be kept up to date in one place.


Contents
--------

| Page | What's in it |
| --- | --- |
| [Hardware setup](PCB.md) | Wiring diagram, RS232 transceiver, pin numbering, "nothing arrives" checklist and a complete ESPHome setup |
| [Pytes console manual (PDF)](<Pytes HyperTerminal Software Guide(1).pdf>) | Official guide to the Pytes console commands |
| [Configuration reference](../README.md#component--hub) | Hub, sensors and text sensors |
| [Troubleshooting](../README.md#notes--troubleshooting) | What the timeout messages mean |
| [Upgrading](../README.md#upgrading-from-older-versions) | Breaking changes between versions |
| [Examples](../examples/) | Ready-to-use YAMLs and per-battery packages |


Console protocol
----------------

The component talks to the battery the same way you would in a terminal: it
sends a command, waits for the echo, reads the response and moves on.

| Command | Sent by driver | Response |
| --- | --- | --- |
| `pwr` | `pytes_e_box`, `pytes_lv1`, `pylontech` | One summary line per battery: voltage, current, temperatures, states, SoC, date/time, balance states |
| `pwr N` | `pytes_e_box`, `pytes_lv1` | Details of battery `N` as `key: value` lines (SOC voltage, coulomb counters, firmware, barcode, protection and event flags) |
| `bat N` | `pytes_e_box`, `pytes_lv1` | One line per cell of battery `N`: voltage, temperature, states, SoC and current |

A polling cycle for `batteries: 2` with the default driver is therefore
`pwr`, `pwr 1`, `bat 1`, `pwr 2`, `bat 2`. A response ends with `$$` or
`Command completed successfully`. `pylontech` only sends `pwr` and parses the
lines as they stream in.

### Units on the wire

The battery reports raw integers. The component converts them before
publishing:

| Value | On the wire | Published |
| --- | --- | --- |
| Voltage | mV | V |
| Current | mA | A |
| Temperature | m°C | °C |
| Total coulomb | mAs | Ah |
| Real coulomb | mAh | Ah |
| Total power in / out | As | Ah |


<p align="right">(<a href="#docs-top">back to top</a>)</p>
