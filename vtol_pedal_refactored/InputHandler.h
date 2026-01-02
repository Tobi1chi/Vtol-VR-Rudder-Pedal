#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "RudderConfig.h"

class InputHandler {
public:
    InputHandler();
    void begin();
    int readRudder(const RudderSettings& settings); // Returns combined value -127 to 127
    void printDebug(int l_raw, int r_raw);

private:
    int ADCmodify(int Rudder, int MinValue, int MaxValue, int ERange);
};

#endif
