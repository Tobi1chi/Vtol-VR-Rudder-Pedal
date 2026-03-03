#include <Arduino.h>

#include "InputHandler.h"
#include "OutputHandler.h"
#include "RudderConfig.h"
#include "SerialCommand.h"
#include "SignalProcessing.h"

RudderSettings settings;
InputHandler inputHandler;
OutputHandler outputHandler;
SerialCommand serialCmd;
LowPassFilter lpf(0.5f);
BezierCurve bezier;

void setup() {
  Serial.begin(115200);

  bezier.generateLUT();
  serialCmd.begin(&settings);
  inputHandler.begin();
  outputHandler.begin();
}

void loop() {
  serialCmd.checkSerial();

  if (settings.Test) {
    int rawL = 0;
    int rawR = 0;
    inputHandler.readRaw(rawL, rawR);
    Serial.printf("STATUS: RawL=%d RawR=%d\n", rawL, rawR);
    delay(200);
    return;
  }

  int rudderValue = inputHandler.readRudder(settings);

  if (settings.Curve == 1) {
    rudderValue = (int)round(bezier.mapValue((float)rudderValue));
  }

  if (settings.Filter == 1) {
    rudderValue = (int)round(lpf.update((float)rudderValue));
  }

  rudderValue = constrain(rudderValue, -127, 127);
  outputHandler.send(rudderValue, settings.MODE);

  if (settings.printControl) {
    Serial.printf("STATUS: Rudder=%d\n", rudderValue);
  }

  delay(10);
}
