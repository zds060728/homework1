// 作业5 (ex05): 多档位触摸调速呼吸灯 [修复版]
//
// LED持续呈呼吸灯效果。
// 每触摸一次引脚, 呼吸节奏循环切换:
//   慢速 → 中速 → 快速 → 慢速...
//
// 修复: 触摸检测嵌入呼吸循环内部, 每个step都检查,
//       保证随时触摸都能立刻响应 (延迟最多一个step的时间)

const int ledPin = 2;
const int touchPin = T0;

// ========== PWM参数 ==========
const int freq = 5000;
const int resolution = 8;
const int maxDuty = 255;
// =============================

// ========== 触摸参数 ==========
const int TOUCH_THRESHOLD = 500;
const unsigned long DEBOUNCE_MS = 150;
// ==============================

// ========== 三档速度定义 ==========
const int SPEED_DELAYS[] = { 15, 6, 2 };
const int SPEED_STEPS[]  = { 1,  3, 6 };
const char* SPEED_NAMES[] = {"慢速(~8s)", "中速(~1.5s)", "快速(~0.3s)"};
const int NUM_SPEEDS = 3;
// ===================================

int speedLevel = 0;

// 触摸状态 (跨循环保持)
bool lastTouchState = false;
unsigned long lastDebounceTime = 0;
bool stableTouchState = false;

void setup() {
  Serial.begin(115200);
  ledcAttach(ledPin, freq, resolution);

  Serial.println("==========================================");
  Serial.println("ex05: 多档位触摸调速呼吸灯 (修复版)");
  Serial.print("当前: ");
  Serial.print(SPEED_NAMES[speedLevel]);
  Serial.println(" | 触摸切换档位");
  Serial.println("==========================================");
}

void loop() {
  int delayMs = SPEED_DELAYS[speedLevel];
  int step = SPEED_STEPS[speedLevel];

  // ===== 变亮阶段 (每步都检测触摸) =====
  for (int duty = 0; duty <= maxDuty; duty += step) {
    ledcWrite(ledPin, duty);
    checkTouch();       // ← 关键: 每一步都检查
    delay(delayMs);
  }

  // ===== 变暗阶段 (每步都检测触摸) =====
  for (int duty = maxDuty; duty >= 0; duty -= step) {
    ledcWrite(ledPin, duty);
    checkTouch();       // ← 关键: 每一步都检查
    delay(delayMs);
  }
}

// 触摸检测函数 (在每个呼吸step中调用)
void checkTouch() {
  int touchValue = touchRead(touchPin);
  bool currentTouchState = (touchValue < TOUCH_THRESHOLD);

  // 状态变化时重置防抖计时
  if (currentTouchState != lastTouchState) {
    lastDebounceTime = millis();
  }

  // 防抖确认
  if ((millis() - lastDebounceTime) > DEBOUNCE_MS) {
    if (currentTouchState != stableTouchState) {
      stableTouchState = currentTouchState;

      if (currentTouchState == true) {
        speedLevel = (speedLevel + 1) % NUM_SPEEDS;
        Serial.print(">>> [切换] ");
        Serial.print(SPEED_NAMES[speedLevel]);
        Serial.print(" (delay=");
        Serial.print(SPEED_DELAYS[speedLevel]);
        Serial.print("ms, step=");
        Serial.print(SPEED_STEPS[speedLevel]);
        Serial.println(")");
      }
    }
  }

  lastTouchState = currentTouchState;
}
