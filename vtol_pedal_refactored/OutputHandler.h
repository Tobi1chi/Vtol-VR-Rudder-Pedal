#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include "USB.h"
#include "USBHIDGamepad.h"
#include <BleGamepad.h>
#include "RudderConfig.h"

class OutputHandler {
public:
    OutputHandler();
    void begin();
    void send(int value, int mode); // mode 1=HID, 0=BLE

private:
    USBHIDGamepad hidGamepad;
    BleGamepad bleGamepad;
    BleGamepadConfiguration bleGamepadConfig;
};

#endif
