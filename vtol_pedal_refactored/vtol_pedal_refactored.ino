#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#include <Arduino.h>
#include "RudderConfig.h"
#include "InputHandler.h"
#include "SignalProcessing.h"
#include "OutputHandler.h"
#include "SerialCommand.h"

#define ZERO_BIAS 0
// Objects
RudderSettings settings;
InputHandler inputHandler;
OutputHandler outputHandler;
SerialCommand serialCmd;
LowPassFilter lpf(0.5); // Default alpha 0.5
BezierCurve bezier;

void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  Serial.begin(115200);

  // Initialize Modules
  bezier.generateLUT();
  serialCmd.begin(&settings); // Loads settings from Preferences
  inputHandler.begin();
  outputHandler.begin();
}

void loop() {
  // 1. Process Serial Commands
  serialCmd.checkSerial();

  // 2. Handle Test Mode
  if (settings.Test) {
    int rawL = analogRead(PIN_RUDDER_L);
    int rawR = analogRead(PIN_RUDDER_R);
    Serial.printf("Rudder_L: %d\nRudder_R: %d\n", rawL, rawR);
    delay(200);
    return; // Skip normal processing in test mode
  }

  // 3. Read Inputs
  int rudderValue = inputHandler.readRudder(settings);

  // 4. Apply Curve
  if (settings.Curve) {
    rudderValue = (int)round(bezier.mapValue((float)rudderValue));
  }

  // 5. Apply Filter
  if (settings.Filter == 1) {
    rudderValue = (int)round(lpf.update((float)rudderValue));
  }

  // 6. Send Output
  outputHandler.send(rudderValue, settings.MODE);

  // 7. Debug Print
  if (settings.printControl) {
    Serial.printf("The Rudder value is %d\n", rudderValue);
  }

  // Small delay for stability
  delay(10); // Original code didn't have explicit delay in main loop but had logic.
             // Original had while(1) inside loop().
             // Standard Arduino loop() is already a while(1).
}

#endif
