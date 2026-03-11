#ifndef BOARD_CLI_CONFIG_H
#define BOARD_CLI_CONFIG_H

#define CLI_STORAGE_BACKEND_PREFERENCES 1
#define CLI_STORAGE_BACKEND_EEPROM 0

#define CLI_SUPPORTS_HID 0
#define CLI_SUPPORTS_BLE 1
#define CLI_PERSIST_IMMEDIATELY 1
#define CLI_INTERACTIVE 0
#define CLI_SHOW_DISABLED_MODES_IN_HELP 1

#define CLI_BOARD_INFO_MESSAGE "INFO: ESP32-C3 USB HID is disabled. This build supports BLE mode only."
#define CLI_HID_DISABLED_REASON "HID mode is disabled on ESP32-C3: the on-chip USB is USB Serial/JTAG only"
#define CLI_BLE_DISABLED_REASON "BLE mode is disabled on this build"
#define CLI_HID_HELP_NOTICE "disabled on ESP32-C3: USB Serial/JTAG is not USB HID"
#define CLI_BLE_HELP_NOTICE "disabled"
#define CLI_HID_STATUS_NOTICE "STATUS: HID=DISABLED (ESP32-C3 has USB Serial/JTAG, not native USB HID)"
#define CLI_BLE_STATUS_NOTICE ""
#define CLI_MODE_PROMPT "OK: enter mode (ble only):"
#define CLI_SAVE_MESSAGE "settings persisted"
#define CLI_FACTORY_RESET_MESSAGE "factory defaults restored and persisted"

#endif
