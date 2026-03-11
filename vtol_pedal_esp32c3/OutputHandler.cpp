#include "OutputHandler.h"

#include <Arduino.h>

OutputHandler::OutputHandler() : bleGamepad("VtolRudder", "Origa3mi", 100) {}

void OutputHandler::begin() {
  bleGamepadConfig.setVid(0xe502);
  bleGamepadConfig.setPid(0xabcd);
  bleGamepadConfig.setAxesMin(-127);
  bleGamepadConfig.setAxesMax(127);
  bleGamepad.begin(&bleGamepadConfig);
  Serial.println("OK: BLE gamepad started");
}

void OutputHandler::send(int value) {
  if (!bleGamepad.isConnected()) {
    return;
  }
  bleGamepad.setAxes(value, 0, 0, 0, 0, 0);
}
