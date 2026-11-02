# Commissioning and functional test

## Before powering on

- [ ] IR receiver is connected to 3V3, GND, and D5.
- [ ] D6 is connected to the transistor only through the base resistor.
- [ ] IR LED has a series resistor.
- [ ] Transistor pinout was checked against the datasheet.
- [ ] All GND connections are common.

## Firmware test

1. Compile the sketch.
2. Upload it to the D1 mini Pro.
3. Open the serial monitor at 115200 baud.
4. Check that `ESP-IR started` appears.
5. Connect to `ESP-IR` and open `http://192.168.4.1`.

## Recording test

1. Select slot 1.
2. Press **Record IR signal**.
3. Press a remote control button toward the receiver.
4. Check that the protocol, frequency, and pulse count are displayed.
5. Press **Save signal permanently**.

## Storage test

1. Select another slot.
2. Switch back to slot 1.
3. Check that the saved signal is loaded again.
4. Restart the ESP8266.
5. Open slot 1 again and check that the signal is still present.

## Transmission test

1. Point the target device at the IR LED.
2. Press **Send saved signal**.
3. The target device must react as if the remote control button had been held for approximately two seconds.
4. If reception is poor, check the distance and alignment.

## Troubleshooting

| Problem | Check |
|---|---|
| Wi-Fi network does not appear | Press reset, check the serial monitor, and verify the power supply |
| Recording remains empty | Check receiver pin order, GND, and distance to the remote control |
| Signal is saved but the device does not react | Check LED polarity, transistor pin order, series resistor, and alignment |
| Short action instead of hold function | The target device may interpret repeats differently; record the raw signal again |
| Slots are empty after an update | Record the signal again; old slot files may be incompatible |
| Compile error involving `IRrecv`/`IRsend` | Check the ESP8266 board and `IRremoteESP8266` library |
