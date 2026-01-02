#ifndef RUDDER_CONFIG_H
#define RUDDER_CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define PIN_BUTTON 0
#define PIN_RUDDER_L 4
#define PIN_RUDDER_R 5

// Default Constants
#define DEFAULT_MIN_L 1000
#define DEFAULT_MIN_R 1000
#define DEFAULT_MAX_L 3000
#define DEFAULT_MAX_R 3000
#define DEFAULT_ERANGE 40
#define DEFAULT_MODE 1  // 1: HID, 0: BLE
#define DEFAULT_CURVE 1 // 1: On, 0: Off
#define DEFAULT_FILTER 1 // 1: On, 0: Off

// Runtime Settings Structure
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
