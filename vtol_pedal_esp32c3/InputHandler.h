#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include "RudderConfig.h"

class InputHandler {
public:
  InputHandler();
  void begin();
  int readRudder(const RudderSettings& settings);

private:
  int ADCmodify(int rudder, int minValue, int maxValue, int eRange);
};

#endif
