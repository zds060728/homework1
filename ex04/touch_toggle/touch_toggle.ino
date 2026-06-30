// 作业4 (ex04): 触摸传感器"自锁"开关 [调试版 v2]
//
// 引脚: D4 (GPIO4) = T0 (触摸传感器)
//       D2 (GPIO2) = 板载LED

const int ledPin = 2;        // 板载LED = GPIO2 = D2
const int touchPin = T0;     // 触摸引脚 = GPIO4 = D4

// ========== 可调参数 ==========
const int TOUCH_THRESHOLD = 500;   // 触摸阈值: 100(摸到) < 500 < 1200(没摸)
const unsigned long DEBOUNCE_MS = 150;
// =============================

bool ledState = false;
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;

void setup() {
  // ★ 先用LED闪烁3次, 确认程序已烧录成功
  pinMode(ledPin, OUTPUT);
  for (int i = 0; i < 3; i++) {
    digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(ledPin, LOW);
    delay(200);
  }

  Serial.begin(115200);
  // 等串口准备好
  delay(500);

  int initVal = touchRead(touchPin);
  Serial.println();
  Serial.println("==========================================");
  Serial.println("ex04: 触摸传感器自锁开关 (调试版 v2)");
  Serial.print("触摸引脚: GPIO");
  Serial.print(touchPin);
  Serial.print(" (D4), 初始值: ");
  Serial.println(initVal);
  Serial.print("当前阈值: ");
  Serial.println(TOUCH_THRESHOLD);
  Serial.println("规则: 摸线时数值应 < 阈值 → 触发");
  Serial.println("      不摸时数值应 > 阈值 → 不触发");
  Serial.println("==========================================");
}

void loop() {
  int touchValue = touchRead(touchPin);

  // 始终打印, 方便观察范围
  Serial.print("Touch: ");
  Serial.print(touchValue);
  Serial.print("  ");
  Serial.println(touchValue < TOUCH_THRESHOLD ? "● 摸到了" : "○ 没摸");

  bool currentTouchState = (touchValue < TOUCH_THRESHOLD);

  if (currentTouchState != lastTouchState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    static bool stableTouchState = false;
    if (currentTouchState != stableTouchState) {
      stableTouchState = currentTouchState;

      if (currentTouchState == true) {
        ledState = !ledState;
        digitalWrite(ledPin, ledState ? HIGH : LOW);
        Serial.print(">>> [触发!] LED → ");
        Serial.println(ledState ? "亮 ON" : "灭 OFF");
      }
    }
  }

  lastTouchState = currentTouchState;
  delay(80);
}
