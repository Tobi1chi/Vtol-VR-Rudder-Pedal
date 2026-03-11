#include <Arduino.h>
#include "RudderConfig.h"
#include "InputHandler.h"
#include "SignalProcessing.h"
#include "OutputHandler.h"
#include "SerialCommand.h"

RudderSettings settings;
InputHandler inputHandler;
OutputHandler outputHandler;
SerialCommand serialCmd;
LowPassFilter lpf(0.5f);
BezierCurve bezier;

void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);

  bezier.generateLUT();
  serialCmd.begin(&settings);
  inputHandler.begin();
  outputHandler.begin();
}

void loop() {
  serialCmd.checkSerial();

  if (settings.Test) {
    int rawL = analogRead(PIN_RUDDER_L);
    int rawR = analogRead(PIN_RUDDER_R);
    Serial.printf("Rudder_L: %d\nRudder_R: %d\n", rawL, rawR);
    delay(200);
    return;
  }

  int rudderValue = inputHandler.readRudder(settings);

  if (settings.Curve) {
    rudderValue = (int)round(bezier.mapValue((float)rudderValue));
  }

  if (settings.Filter == 1) {
    rudderValue = (int)round(lpf.update((float)rudderValue));
  }

  outputHandler.send(rudderValue);

  if (settings.printControl) {
    Serial.printf("The Rudder value is %d\n", rudderValue);
  }

  delay(10);
}
