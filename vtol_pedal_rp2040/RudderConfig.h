#ifndef RUDDER_CONFIG_H
#define RUDDER_CONFIG_H

#include <Arduino.h>

// RP2040 analog pins
#define PIN_RUDDER_L A0
#define PIN_RUDDER_R A1

// Default calibration/settings
#define DEFAULT_MIN_L 1000
#define DEFAULT_MIN_R 1000
#define DEFAULT_MAX_L 3000
#define DEFAULT_MAX_R 3000
#define DEFAULT_ERANGE 40
#define DEFAULT_MODE 1
#define DEFAULT_CURVE 1
#define DEFAULT_FILTER 1

#define MODE_HID 1
#define MODE_BLE 0

#define SERIAL_PARAM_MIN 1
#define SERIAL_PARAM_MAX 4095

struct RudderSettings {
    int minRudder_L;
    int minRudder_R;
    int maxRudder_L;
    int maxRudder_R;
    int ERange;
    int MODE;
    int Curve;
    int Filter;
    bool printControl;
    bool Test;
};

#endif
