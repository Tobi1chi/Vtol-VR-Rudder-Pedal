#include "InputHandler.h"

InputHandler::InputHandler() {}

void InputHandler::begin() {
  pinMode(PIN_RUDDER_L, INPUT);
  pinMode(PIN_RUDDER_R, INPUT);
}

int InputHandler::readRudder(const RudderSettings& settings) {
  int rawL = analogRead(PIN_RUDDER_L);
  int rawR = analogRead(PIN_RUDDER_R);

  int valL = ADCmodify(rawL, settings.minRudder_L, settings.maxRudder_L, settings.ERange);
  int valR = ADCmodify(rawR, settings.minRudder_R, settings.maxRudder_R, settings.ERange);

  return valL - valR;
}

int InputHandler::ADCmodify(int rudder, int minValue, int maxValue, int eRange) {
  int zeroP = minValue + eRange;
  int effectiveMax = maxValue - zeroP;
  rudder -= zeroP;

  if (rudder <= 0) {
    return 0;
  }

  long mapped = map(rudder, 0, effectiveMax, 0, 127);
  return constrain(mapped, 0, 127);
}
