#ifndef BOARD_CLI_CONFIG_H
#define BOARD_CLI_CONFIG_H

#define CLI_STORAGE_BACKEND_PREFERENCES 0
#define CLI_STORAGE_BACKEND_EEPROM 1

#define CLI_SUPPORTS_HID 1
#define CLI_SUPPORTS_BLE 0
#define CLI_PERSIST_IMMEDIATELY 0
#define CLI_INTERACTIVE 1
#define CLI_SHOW_DISABLED_MODES_IN_HELP 1

#define CLI_BOARD_INFO_MESSAGE "INFO: RP2040 build does not support BLE. This firmware uses USB HID only."
#define CLI_HID_DISABLED_REASON "HID mode is disabled on this build"
#define CLI_BLE_DISABLED_REASON "BLE mode is disabled on RP2040: this build supports USB HID only"
#define CLI_HID_HELP_NOTICE "disabled"
#define CLI_BLE_HELP_NOTICE "disabled on RP2040: this build has no BLE support"
#define CLI_HID_STATUS_NOTICE ""
#define CLI_BLE_STATUS_NOTICE "STATUS: BLE=DISABLED (RP2040 build has no BLE support)"
#define CLI_MODE_PROMPT "OK: enter mode (hid only; ble is disabled on RP2040):"
#define CLI_SAVE_MESSAGE "settings persisted to EEPROM"
#define CLI_FACTORY_RESET_MESSAGE "factory defaults restored"

#endif
