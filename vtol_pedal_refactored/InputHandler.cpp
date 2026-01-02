#include "InputHandler.h"

InputHandler::InputHandler() {}

void InputHandler::begin() {
    pinMode(PIN_RUDDER_L, INPUT);
    pinMode(PIN_RUDDER_R, INPUT);
}

int InputHandler::readRudder(const RudderSettings& settings) {
    int rawL = analogRead(PIN_RUDDER_L);
    int rawR = analogRead(PIN_RUDDER_R);

    // If in Test mode, we might want to print raw values externally, 
    // but the main loop handles the logic flow. 
    // Here we just return the processed combined value.

    int valL = ADCmodify(rawL, settings.minRudder_L, settings.maxRudder_L, settings.ERange);
    int valR = ADCmodify(rawR, settings.minRudder_R, settings.maxRudder_R, settings.ERange);
    
    // Right rudder is inverted in the original logic: -1 * ADCmodify(...)
    return valL - valR; 
}

int InputHandler::ADCmodify(int Rudder, int MinValue, int MaxValue, int ERange) {
    int zeroP = MinValue + ERange;
    // Adjust MaxValue relative to zero point
    int effectiveMax = MaxValue - zeroP;
    
    // Adjust Rudder relative to zero point
    Rudder = Rudder - zeroP;
    
    if (Rudder <= 0) {
        return 0;
    }
    
    // Map to 0-127
    long mapped = map(Rudder, 0, effectiveMax, 0, 127);
    return constrain(mapped, 0, 127);
}

void InputHandler::printDebug(int l_raw, int r_raw) {
    Serial.printf("Rudder_L: %d\nRudder_R: %d\n", l_raw, r_raw);
}
