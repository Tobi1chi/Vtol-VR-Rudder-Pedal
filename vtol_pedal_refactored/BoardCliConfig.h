#ifndef BOARD_CLI_CONFIG_H
#define BOARD_CLI_CONFIG_H

#define CLI_STORAGE_BACKEND_PREFERENCES 1
#define CLI_STORAGE_BACKEND_EEPROM 0

#define CLI_SUPPORTS_HID 1
#define CLI_SUPPORTS_BLE 1
#define CLI_PERSIST_IMMEDIATELY 1
#define CLI_INTERACTIVE 0
#define CLI_SHOW_DISABLED_MODES_IN_HELP 0

#define CLI_BOARD_INFO_MESSAGE ""
#define CLI_HID_DISABLED_REASON "HID mode is disabled on this build"
#define CLI_BLE_DISABLED_REASON "BLE mode is disabled on this build"
#define CLI_HID_HELP_NOTICE "disabled"
#define CLI_BLE_HELP_NOTICE "disabled"
#define CLI_HID_STATUS_NOTICE ""
#define CLI_BLE_STATUS_NOTICE ""
#define CLI_MODE_PROMPT "OK: enter mode (hid/ble):"
#define CLI_SAVE_MESSAGE "settings persisted"
#define CLI_FACTORY_RESET_MESSAGE "factory defaults restored and persisted"

#endif
