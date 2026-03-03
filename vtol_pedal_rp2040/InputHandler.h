#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "RudderConfig.h"

class InputHandler {
public:
    InputHandler();
    void begin();
    int readRudder(const RudderSettings& settings);
    void readRaw(int& rawL, int& rawR);

private:
    int adcModify(int rudder, int minValue, int maxValue, int eRange);
};

#endif
