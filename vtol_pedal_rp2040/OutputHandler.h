#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include <Arduino.h>
#include <Joystick.h>
#include "RudderConfig.h"

class OutputHandler {
public:
    OutputHandler();
    void begin();
    void send(int value, int mode);
};

#endif
