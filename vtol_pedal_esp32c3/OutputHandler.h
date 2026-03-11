#ifndef OUTPUT_HANDLER_H
#define OUTPUT_HANDLER_H

#include <BleGamepad.h>

class OutputHandler {
public:
  OutputHandler();
  void begin();
  void send(int value);

private:
  BleGamepad bleGamepad;
  BleGamepadConfiguration bleGamepadConfig;
};

#endif
