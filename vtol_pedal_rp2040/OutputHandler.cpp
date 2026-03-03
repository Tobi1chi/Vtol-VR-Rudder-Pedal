#include "OutputHandler.h"

OutputHandler::OutputHandler() {}

void OutputHandler::begin() {
    Joystick.begin();
}

void OutputHandler::send(int value, int mode) {
    if (mode != MODE_HID) {
        return;
    }

    int clamped = constrain(value, -127, 127);
    Joystick.X(clamped);
}
