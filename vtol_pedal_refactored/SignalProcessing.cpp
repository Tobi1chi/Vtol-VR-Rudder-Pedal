#include "SignalProcessing.h"

// --- LowPassFilter ---

LowPassFilter::LowPassFilter(float alpha) : _alpha(alpha), _prevOutput(0) {}

float LowPassFilter::update(float input) {
    float y = input * _alpha + (1.0f - _alpha) * _prevOutput;
    _prevOutput = y;
    return y;
}

void LowPassFilter::setAlpha(float alpha) {
    _alpha = alpha;
}

// --- BezierCurve ---

BezierCurve::BezierCurve() {
    // Constructor
}

float BezierCurve::bezier(float t, int axis) {
    float a = pow(1 - t, 3) * P0[axis];
    float b = 3 * pow(1 - t, 2) * t * P1[axis];
    float c = 3 * (1 - t) * t * t * P2[axis];
    float d = t * t * t * P3[axis];
    return a + b + c + d;
}

void BezierCurve::generateLUT() {
    for (int i = 0; i < LUT_SIZE; i++) {
        float t = (float)i / (LUT_SIZE - 1);
        bezier_x[i] = bezier(t, 0);
        bezier_y[i] = bezier(t, 1);
    }
}

float BezierCurve::bezierLUTMap(float x_input) {
    for (int i = 0; i < LUT_SIZE - 1; i++) {
        if (x_input >= bezier_x[i] && x_input <= bezier_x[i + 1]) {
            float ratio = (x_input - bezier_x[i]) / (bezier_x[i + 1] - bezier_x[i]);
            return bezier_y[i] + ratio * (bezier_y[i + 1] - bezier_y[i]);
        }
    }
    if (x_input <= bezier_x[0]) return bezier_y[0];
    if (x_input >= bezier_x[LUT_SIZE - 1]) return bezier_y[LUT_SIZE - 1];
    return 0;
}

float BezierCurve::mapValue(float input) {
    // Logic from original code
    // input is expected to be roughly -127 to 127 (or similar range)
    // The original code uses abs(RuddervalueRatio) where ratio = val/127
    
    float tempRuddervalue = input;
    float mult = (input >= 0) ? 1.0f : -1.0f;
    float RuddervalueRatio = tempRuddervalue / 127.0f;
    
    // Apply curve
    tempRuddervalue = mult * 127.0f * bezierLUTMap(abs(RuddervalueRatio));
    return tempRuddervalue;
}
