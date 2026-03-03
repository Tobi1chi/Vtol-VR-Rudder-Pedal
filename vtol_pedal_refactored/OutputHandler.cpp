#include "OutputHandler.h"

OutputHandler::OutputHandler() 
    : bleGamepad("VtolRudder", "Origa3mi", 100) 
{
}

void OutputHandler::begin() {
    // HID Setup
    hidGamepad.begin();
    USB.begin();

    // BLE Setup
    bleGamepadConfig.setVid(0xe502);
    bleGamepadConfig.setPid(0xabcd);
    bleGamepadConfig.setAxesMin(-127);
    bleGamepadConfig.setAxesMax(127);
    bleGamepad.begin(&bleGamepadConfig);
    Serial.println("Starting BLE work!");
}

void OutputHandler::send(int value, int mode) {
    if (mode == MODE_HID) {
        // HID mode
        hidGamepad.leftStick(value, 0); 
    } else {
        // BLE mode
        bleGamepad.setAxes(value, 0, 0, 0, 0, 0);
    }
}
