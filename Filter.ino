#ifndef FilterFunc
float FirstOrder_LowPassFilter(float x, float alpha, float PrevOutput) {
 float y = x*alpha + (1-alpha) * PrevOutput;
 return y;
}
#endif