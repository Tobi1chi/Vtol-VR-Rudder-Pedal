#include "InputHandler.h"

InputHandler::InputHandler() {}

void InputHandler::begin() {
    pinMode(PIN_RUDDER_L, INPUT);
    pinMode(PIN_RUDDER_R, INPUT);
}

void InputHandler::readRaw(int& rawL, int& rawR) {
    rawL = analogRead(PIN_RUDDER_L);
    rawR = analogRead(PIN_RUDDER_R);
}

int InputHandler::readRudder(const RudderSettings& settings) {
    int rawL = 0;
    int rawR = 0;
    readRaw(rawL, rawR);

    int valL = adcModify(rawL, settings.minRudder_L, settings.maxRudder_L, settings.ERange);
    int valR = adcModify(rawR, settings.minRudder_R, settings.maxRudder_R, settings.ERange);
    return valL - valR;
}

int InputHandler::adcModify(int rudder, int minValue, int maxValue, int eRange) {
    const int zeroP = minValue + eRange;
    const int effectiveMax = maxValue - zeroP;
    if (effectiveMax <= 0) {
        return 0;
    }

    rudder -= zeroP;
    if (rudder <= 0) {
        return 0;
    }

    long mapped = map(rudder, 0, effectiveMax, 0, 127);
    return constrain(mapped, 0, 127);
}
