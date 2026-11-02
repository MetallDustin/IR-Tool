# Wiring

## Pin overview

| D1 mini Pro | Function | Connection |
|---|---|---|
| `D5` / GPIO14 | IR reception | IR receiver OUT |
| `D6` / GPIO12 | IR transmission control | 1 kΩ resistor to the transistor base |
| `3V3` | Receiver power | IR receiver VCC |
| `5V`/`VU` | LED driver power | LED series resistor, only with USB power |
| `G` / GND | Common ground | Receiver GND and transistor emitter |

## IR receiver

The receiver normally has three connections:

```text
Receiver VCC  -> 3V3
Receiver GND  -> GND
Receiver OUT  -> D5
```

The `OUT/GND/VCC` order is not the same on all modules. The labeling or datasheet of the specific module takes precedence.

## IR driver with an NPN transistor

```text
D6 ── 1 kΩ ──> Base        2N2222 / BC337
                 Collector ── IR LED cathode
GND ──────────── Emitter

5V/VU ── 50 Ω ──> IR LED anode
```

Important:

- `5V/VU` is only available when the board is powered over USB.
- The transistor must match the pinout of the specific component; the pin order can differ between 2N2222 and BC337.
- The IR LED anode is the longer lead; the cathode is usually marked with a flat edge.
- The receiver, ESP8266, and transistor GND connections must be common.
- Do not connect the GPIO directly to the LED.

## Current warning for 50 Ω

At 5 V, with an LED forward voltage of approximately 1.2 V and a saturated transistor, 50 Ω produces approximately 70–75 mA peak current. Use this only if the specific IR LED supports that pulse current and repetition rate. For a typical 20 mA IR LED, 50 Ω is too small; use a larger resistor instead.

## Alternative with 3.3 V power

If the driver is powered entirely from 3.3 V, the LED series resistor can also be connected to `3V3`. The LED will usually be less bright. Always follow the permitted continuous and pulse current from the datasheet.

## Schematic

The graphical schematic is available as [`schematic.svg`](schematic.svg) in the repository.
