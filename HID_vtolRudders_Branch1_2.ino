#ifndef ARDUINO_USB_MODE
#error This ESP32 SoC has no Native USB interface
#elif ARDUINO_USB_MODE == 1
#warning This sketch should be used when USB is in OTG mode
void setup() {}
void loop() {}
#else

#define HID 1
#define BLE 0
//#define MODE HID  //change HID or BLE to change the connection mode 有线/无线连接
//#define Filter 1  //change to 0 to off the filter  滤波器
//#define Curve 1   //using curved output rater than a linear output (0 for off and 1 for on) 曲线映射


#include "USB.h"
#include "USBHIDGamepad.h"
#define USB_PRODUCT_STRING    "VtolRudder"
#define USB_MANUFACTURER_STRING "Origa3mi"
USBHIDGamepad Gamepad;
#include <BleGamepad.h>
BleGamepad bleGamepad("VtolRudder","Origa3mi",100);
BleGamepadConfiguration bleGamepadConfig;
#include <Preferences.h>
Preferences prefs;

//BleGamepad bleGamepad;
//Rudder_L pin:4
//Rudder_R pin:5

//User defined variables start
const int buttonPin = 0;

//int previousButtonState = HIGH;
int minRudder_L = 1010;
int minRudder_R = 980;
int maxRudder_L = 3080;
int maxRudder_R = 3100;
int ERange = 40;
int MODE = 0;
int Filter = 0;
int Curve = 0;
bool printControl = false;
bool Test = false;
int currentCommandId = 0;
bool waitingForValue = false;

//const int zeroP = 995 + ERange;
//const int maxValue = 3070 - zeroP;
float LPFalpha = 0.5;  //一阶LPF滤波系数
//command table
const char* CommandsA[] = {
  "ERROR",        //0
  "RESET",        //1
  "BLE",          //2
  "HID",          //3
  "Debug",        //4
  "Curve",        //5
  "Filter",       //6
  "minRudder_L",  //7
  "minRudder_R",  //8
  "maxRudder_L",  //9
  "maxRudder_R",  //10
  "ERange",       //11
  "Test"          //12
};
int numCommands = sizeof(CommandsA) / sizeof(CommandsA[0]);
char inputBuffer[64];
int inputIndex = 0;
//params:
//User defined variables end


void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(5, INPUT);
  Serial.begin(115200);
  pinMode(4, INPUT);
  generateBezierLUT();  //Create the LUT for curving the output
  prefs.begin("presets", false);
  int bootCount = prefs.getInt("bootCount", 0);
  if (bootCount == 0) {
    prefs.putInt("minRudder_L", 1000);
    prefs.putInt("minRudder_R", 1000);
    prefs.putInt("maxRudder_L", 3000);
    prefs.putInt("maxRudder_R", 3000);
    prefs.putInt("ERange", 40);
    prefs.putInt("MODE", 1);
    prefs.putInt("Curve", 1);
    prefs.putInt("Filter", 1);
  }
  prefs.putInt("bootCount", bootCount + 1);
  minRudder_L = prefs.getInt("minRudder_L", 1000);
  minRudder_R = prefs.getInt("minRudder_R", 1000);
  maxRudder_L = prefs.getInt("maxRudder_L", 3000);
  maxRudder_R = prefs.getInt("maxRudder_R", 3000);
  ERange = prefs.getInt("ERange", 40);
  MODE = prefs.getInt("MODE", 1);
  Curve = prefs.getInt("Curve", 1);
  Filter = prefs.getInt("Filter", 1);
  //Initialize the presets values to the default mode
  prefs.end();
  generateBezierLUT();  //Create the LUT for curving the output
  Gamepad.begin();
  USB.begin();
  bleGamepadConfig.setVid(0xe502);
  bleGamepadConfig.setPid(0xabcd);
  bleGamepadConfig.setAxesMin(-127);  // 0 --> int16_t - 16 bit signed integer - Can be in decimal or hexadecimal
  bleGamepadConfig.setAxesMax(127);
  Serial.println("Starting BLE work!");
  bleGamepad.begin(&bleGamepadConfig);
}

void loop() {
  int Ruddervalue = 0;
  float RuddervalueRatio = 0;
  float mult = 0;
  float tempRuddervalue = 0;
  float Prev = 0;
  int Rudder_L = 0;
  int Rudder_R = 0;
  int cmdid = 0;
  while (1) {
    Rudder_L = analogRead(4);
    Rudder_R = analogRead(5);
    Rudder_L = ADCmodify(Rudder_L, minRudder_L, maxRudder_L, ERange);
    Rudder_R = -1 * ADCmodify(Rudder_R, minRudder_R, maxRudder_R, ERange);
    Ruddervalue = Rudder_R + Rudder_L;

    if (Curve) {
      tempRuddervalue = Ruddervalue;
      if (Ruddervalue >= 0) {
        mult = 1;
      } else {
        mult = -1;
      }
      RuddervalueRatio = tempRuddervalue / 127;
      tempRuddervalue = mult * 127 * bezierLUTMap(abs(RuddervalueRatio));
      Ruddervalue = (int)round(tempRuddervalue);
    }

    if (Filter == 1) {
      tempRuddervalue = FirstOrder_LowPassFilter(tempRuddervalue, LPFalpha, Prev);
      Prev = tempRuddervalue;
      Ruddervalue = (int)round(tempRuddervalue);
    }

    if (MODE == 1) {
      Gamepad.leftStick(Ruddervalue, 0);  //HID mode
    } else {
      bleGamepad.setAxes(Ruddervalue, 0, 0, 0, 0, 0);  //BLE mode
    }

    //test
    //Gamepad.leftStick(RudderV, 0);
    //test code end
    if (printControl) {
      Serial.println(tempRuddervalue);
      Serial.println(RuddervalueRatio);
      Serial.printf("The Rudder value is %d\n", Ruddervalue);
    }


    while (Serial.available()) {
      char ch = Serial.read();

      if (ch == '\n' || ch == '\r') {
        inputBuffer[inputIndex] = '\0';  // 结束当前输入

        if (!waitingForValue) {
          // 处理命令
          currentCommandId = processCommand(inputBuffer, CommandsA);
          if (currentCommandId >= 5 && currentCommandId <= 11) {
            Serial.printf("请输入参数值（例如：1100）：\n");
            waitingForValue = true;  // 等待数值输入
          } else {
            // 不需要数值的命令可以立即执行（如 Debug 或 Test）
            processParam("0", currentCommandId);  // 用默认值或切换逻辑
            currentCommandId = 0;
          }
        } else {
          // 已经输入过命令，现在输入的是数值
          processParam(inputBuffer, currentCommandId);
          currentCommandId = 0;
          waitingForValue = false;
        }

        inputIndex = 0;
      } else {
        if (inputIndex < sizeof(inputBuffer) - 1) {
          inputBuffer[inputIndex++] = ch;
        }
      }
    }

    while (Test) {
      Rudder_L = analogRead(4);
      delay(20);
      Rudder_R = analogRead(5);
      Serial.printf("Rudder_L: %d\nRudder_R: %d\n", Rudder_L, Rudder_R);
      delay(200);
      if(Serial.available()){
        break;
      }
    }

    delay(50);
  }
}

int ADCmodify(int Rudder, int MinValue, int MaxValue, int ERange) {
  //Serial.println(Rudder);
  int zeroP = MinValue + ERange;
  MaxValue = MaxValue - zeroP;
  Rudder = Rudder - zeroP;
  if (Rudder <= 0) {
    return 0;
  }
  Rudder = map(Rudder, 0, MaxValue, 0, 127);
  Rudder = constrain(Rudder, 0, 127);
  return Rudder;
}

int processCommand(const char* cmd, const char** validCommands) {
  for (int i = 0; i < numCommands; i++) {
    if (strcmp(cmd, validCommands[i]) == 0) {
      Serial.print("匹配成功：");
      Serial.println(validCommands[i]);
      return i;
    }
  }
  Serial.println("匹配失败: ERROR");
  return 0;
}


int processParam(const char* input, int id) {
  int value = atoi(input);  // 将字符串转换为整数
  prefs.begin("presets", false);
  switch (id) {
    case 1:                          //RESET
      prefs.putInt("bootCount", 0);  //reset boot count
      break;

    case 2:  //BLE Mode
      MODE = 0;
      prefs.putInt("MODE", 0);
      break;

    case 3:  //HID Mode
      MODE = 1;
      prefs.putInt("MODE", 0);
      break;

    case 4:                          //Debug Mode (Serial output)
      printControl = !printControl;  //switch status for bool variable
      break;

    case 5:  //change curve mode
      Curve = value;
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      prefs.putInt("Curve", value);
      break;

    case 6:
      Filter = value;
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      prefs.putInt("Filter", value);
      break;

    case 7:  //minRudder_L
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      minRudder_L = value;
      prefs.putInt("minRudder_L", value);
      break;

    case 8:  //minRudder_R
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      minRudder_R = value;
      prefs.putInt("minRudder_R", value);
      break;

    case 9:  //maxRudder_L
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      maxRudder_L = value;
      prefs.putInt("maxRudder_L", value);
      break;

    case 10:  //maxRudder_R
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      maxRudder_R = value;
      prefs.putInt("maxRudder_R", value);

      break;

    case 11:  //Erange
      if (value <= 0 || value >= 4096) {
        return id;
        //Serial.printf("Invalid input\n");
      }
      ERange = value;
      prefs.putInt("ERange", value);
      break;

    case 12:
      Test = !Test;
      break;

    default:
      return 0;
      break;
  }
  prefs.end();
  Serial.println(value);
  return 0;
}


#endif /* ARDUINO_USB_MODE */
