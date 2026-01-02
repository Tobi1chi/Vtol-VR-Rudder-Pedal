#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

#include <Arduino.h>

class LowPassFilter {
public:
    LowPassFilter(float alpha = 0.5);
    float update(float input);
    void setAlpha(float alpha);
private:
    float _alpha;
    float _prevOutput;
};

class BezierCurve {
public:
    BezierCurve();
    void generateLUT();
    float mapValue(float input); // Input -127 to 127, returns mapped value
    
private:
    static const int LUT_SIZE = 128;
    float bezier_x[LUT_SIZE];
    float bezier_y[LUT_SIZE];
    float P0[2] = { 0.0, 0.0 };
    float P1[2] = { 0.35, 0.1 };
    float P2[2] = { 0.8, 1 };
    float P3[2] = { 1.0, 1.0 };
    
    float bezier(float t, int axis);
    float bezierLUTMap(float x_input);
};

#endif
