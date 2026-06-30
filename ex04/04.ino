#include <Arduino.h>

// 硬件引脚定义
#define TOUCH_PIN 4
const int ledPin = 2;

// PWM参数
const int freq = 5000;
const int resolution = 8;  // 0~255

// 触摸阈值，根据实际调整
#define TOUCH_THRESHOLD 200

// 档位：1/2/3 三档速度
volatile int speedLevel = 1;
// 中断消抖
unsigned long lastTouchTrigger = 0;
const unsigned long touchDebounceMs = 300;

// 触摸中断回调
void touchISR()
{
  unsigned long now = millis();
  if (now - lastTouchTrigger < touchDebounceMs)
    return;

  lastTouchTrigger = now;
  // 档位循环切换 1→2→3→1
  speedLevel += 1;
  if (speedLevel > 3)
    speedLevel = 1;
}

void setup()
{
  Serial.begin(115200);
  // 绑定PWM
  ledcAttach(ledPin, freq, resolution);
  // 挂载触摸中断
  touchAttachInterrupt(TOUCH_PIN, touchISR, TOUCH_THRESHOLD);
}

void loop()
{
  // 根据档位设置步进 step，step越大呼吸越快
  int step;
  switch (speedLevel)
  {
    case 1: step = 1; break;  // 最慢
    case 2: step = 3; break;  // 中等
    case 3: step = 8; break;  // 最快
    default: step = 1;
  }

  // 渐亮
  for (int duty = 0; duty <= 255; duty += step)
  {
    ledcWrite(ledPin, duty);
    delay(10);
  }
  // 渐暗
  for (int duty = 255; duty >= 0; duty -= step)
  {
    ledcWrite(ledPin, duty);
    delay(10);
  }

  Serial.print("当前档位：");
  Serial.println(speedLevel);
}