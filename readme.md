# VTOL VR Rudder Pedal (ESP32)

## Languages

- English: `readme.md`
- 中文: `readme_zh.md`

## Serial CLI (`vtol_pedal_refactored`)

Baud rate: `115200`

The refactored firmware uses a single-line CLI. Old two-step serial input is no longer supported.

Available modes depend on the target board:
- `vtol_pedal_refactored` (ESP32 native-USB target): supports `mode hid|ble`
- `vtol_pedal_esp32c3`: supports `mode ble` only; `mode hid` prints a disabled message
- `vtol_pedal_rp2040`: supports `mode hid` only; `mode ble` prints a disabled message

### Command list

- `help`
- `status`
- `mode hid|ble`
- `debug on|off`
- `test on|off`
- `set curve 0|1`
- `set filter 0|1`
- `set min_l <1..4095>`
- `set min_r <1..4095>`
- `set max_l <1..4095>`
- `set max_r <1..4095>`
- `set erange <1..4095>`
- `save` (no-op; values are persisted immediately)
- `factory_reset`

### Response format

- Success: `OK: <message>`
- Error: `ERR: <reason>`
- Status lines: `STATUS: key=value`

### Input behavior

- Commands are case-insensitive.
- Leading/trailing spaces and repeated spaces are accepted.
- Number parsing is strict (`12abc` is rejected).
- Invalid values are rejected and are not written to `Preferences`.

### Validation rules

- `curve` and `filter` only accept `0` or `1`.
- `min_l`, `min_r`, `max_l`, `max_r`, `erange` range: `1..4095`.
- Coupled constraints:
  - `max_l > min_l + erange`
  - `max_r > min_r + erange`

### Example session

```text
help
status
mode hid
set min_l 1100
set erange 60
set max_l 2500
debug on
test off
factory_reset
status
```

### Old-to-new command mapping

- `HID` -> `mode hid`
- `BLE` -> `mode ble`
- `Debug` -> `debug on|off` (explicit state)
- `Test` -> `test on|off` (explicit state)
- `Curve` + value -> `set curve <0|1>`
- `Filter` + value -> `set filter <0|1>`
- `minRudder_L` + value -> `set min_l <value>`
- `minRudder_R` + value -> `set min_r <value>`
- `maxRudder_L` + value -> `set max_l <value>`
- `maxRudder_R` + value -> `set max_r <value>`
- `ERange` + value -> `set erange <value>`
- `RESET` -> `factory_reset`

## RP2040 version (`vtol_pedal_rp2040`)

This folder reuses the ESP32 refactored architecture for:
- input sampling and calibration mapping
- Bezier curve and low-pass filter signal processing
- single-line serial CLI (`help/status/mode/debug/test/set/save/factory_reset`)

### Notes

- Current RP2040 build outputs USB HID via `Joystick` library (`Joystick.h`).
- RP2040 build currently supports `mode hid` only.
- On boot the CLI prints a `BLE disabled` notice, and `status` reports `BLE=DISABLED`.
- Entering `mode ble` returns an explicit error that the RP2040 build has no BLE support.
- RP2040 settings persistence is EEPROM-based:
  - `save` writes current runtime settings to EEPROM.
  - `factory_reset` restores defaults and writes them to EEPROM.
- RP2040 serial supports interactive input:
  - `mode`, `debug`, `test`, `set`, `set <key>` can prompt for the next value.
  - `cancel` exits current interactive input state.

### UF2 flashing

1. Press and hold the `BOOT` button on the RP2040 board.
2. While holding `BOOT`, plug the device into USB.
3. Release `BOOT` after the board appears as a USB mass storage device.
4. Copy the compiled `.uf2` file directly into that device storage.
5. The board will reboot automatically and run the new firmware.

## ESP32-C3 version (`vtol_pedal_esp32c3`)

This folder reuses most of the refactored input, filtering, and serial CLI structure, but its output capabilities differ from the original ESP32 native-USB build.

### Notes

- Current ESP32-C3 build supports `mode ble` only.
- On boot the CLI prints a `USB HID disabled / BLE only` notice.
- `status` reports `HID=DISABLED`.
- Entering `mode hid` returns an explicit error explaining that ESP32-C3 uses `USB Serial/JTAG`, not native USB HID.
- In the current default pin assignment:
  - `GPIO4` / `GPIO5` are used for the two rudder ADC inputs and are a low-risk choice.
  - `GPIO0` is currently used as the button pin; it works, but it is not the preferred default for production hardware because of boot-related constraints.
