#define LED_PIN 2

// 时序参数
const unsigned long SHORT_ON  = 200;
const unsigned long LONG_ON   = 600;
const unsigned long FLASH_GAP = 200;
const unsigned long SOS_BLACK = 2000;

unsigned long prevTime = 0;
bool ledState = LOW;

// 四阶段：第一轮短闪、长闪、第二轮短闪、黑屏
enum Mode {
    S_SHORT1,
    S_LONG,
    S_SHORT2,
    S_BLACK
};
Mode mode = S_SHORT1;
int cnt = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  unsigned long now = millis();
  unsigned long timeWait;

  if (mode == S_BLACK) {
    timeWait = SOS_BLACK;
  } else {
    if (ledState == HIGH) {
      if(mode == S_LONG) timeWait = LONG_ON;
      else timeWait = SHORT_ON;
    } else {
      timeWait = FLASH_GAP;
    }
  }

  if (now - prevTime >= timeWait) {
    prevTime = now;

    if (mode == S_BLACK) {
      // 黑屏结束，从头开始
      mode = S_SHORT1;
      cnt = 0;
      ledState = LOW;
    } else {
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);

      // 熄灭时计数，一组3次完成切换阶段
      if (ledState == LOW) {
        cnt++;
        if (cnt >= 3) {
          cnt = 0;
          if (mode == S_SHORT1) {
            mode = S_LONG;
          } else if (mode == S_LONG) {
            mode = S_SHORT2;
          } else if (mode == S_SHORT2) {
            // 三组全部完成，进入长黑屏
            mode = S_BLACK;
          }
        }
      }
    }
    digitalWrite(LED_PIN, ledState);
  }
}