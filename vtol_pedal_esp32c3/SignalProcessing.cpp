#include "SignalProcessing.h"

LowPassFilter::LowPassFilter(float alpha) : _alpha(alpha), _prevOutput(0.0f) {}

float LowPassFilter::update(float input) {
  float output = input * _alpha + (1.0f - _alpha) * _prevOutput;
  _prevOutput = output;
  return output;
}

BezierCurve::BezierCurve() {}

float BezierCurve::bezier(float t, int axis) {
  float a = pow(1.0f - t, 3) * P0[axis];
  float b = 3.0f * pow(1.0f - t, 2) * t * P1[axis];
  float c = 3.0f * (1.0f - t) * t * t * P2[axis];
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

float BezierCurve::bezierLUTMap(float xInput) {
  for (int i = 0; i < LUT_SIZE - 1; i++) {
    if (xInput >= bezier_x[i] && xInput <= bezier_x[i + 1]) {
      float ratio = (xInput - bezier_x[i]) / (bezier_x[i + 1] - bezier_x[i]);
      return bezier_y[i] + ratio * (bezier_y[i + 1] - bezier_y[i]);
    }
  }

  if (xInput <= bezier_x[0]) {
    return bezier_y[0];
  }
  if (xInput >= bezier_x[LUT_SIZE - 1]) {
    return bezier_y[LUT_SIZE - 1];
  }
  return 0.0f;
}

float BezierCurve::mapValue(float input) {
  float sign = input >= 0.0f ? 1.0f : -1.0f;
  float ratio = input / 127.0f;
  return sign * 127.0f * bezierLUTMap(abs(ratio));
}
