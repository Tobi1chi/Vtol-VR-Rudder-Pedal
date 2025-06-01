#ifndef CurveFunc
const int LUT_SIZE = 128;
float bezier_x[LUT_SIZE];
float bezier_y[LUT_SIZE];  // 存储 y 值，对应输入 x = i/(LUT_SIZE - 1)
//更改P1 P2的点来改变曲线
float P0[2] = { 0.0, 0.0 };
float P1[2] = { 0.35, 0.1 };
float P2[2] = { 0.8, 1 };
float P3[2] = { 1.0, 1.0 };

// 贝塞尔函数
float bezier(float t, int axis) {
  float a = pow(1 - t, 3) * P0[axis];
  float b = 3 * pow(1 - t, 2) * t * P1[axis];
  float c = 3 * (1 - t) * t * t * P2[axis];
  float d = t * t * t * P3[axis];
  return a + b + c + d;
}


// 初始化查找表
void generateBezierLUT() {
  for (int i = 0; i < LUT_SIZE; i++) {
    float t = (float)i / (LUT_SIZE - 1);
    bezier_x[i] = bezier(t, 0);  // x(t)
    bezier_y[i] = bezier(t, 1);  // y(t)
  }
}


// 输入值 x (0~1)，返回查表后的 y 值
float bezierLUTMap(float x_input) { 
  // 遍历查找 x 值最近区间
  for (int i = 0; i < LUT_SIZE - 1; i++) {
    if (x_input >= bezier_x[i] && x_input <= bezier_x[i + 1]) {
      // 线性插值计算 y 值
      float ratio = (x_input - bezier_x[i]) / (bezier_x[i + 1] - bezier_x[i]);
      float y = bezier_y[i] + ratio * (bezier_y[i + 1] - bezier_y[i]);
      return y;
    }
  }
  // 越界处理
  if (x_input <= bezier_x[0]) return bezier_y[0];
  if (x_input >= bezier_x[LUT_SIZE - 1]) return bezier_y[LUT_SIZE - 1];
  return 0;
}
#endif
