#define LED_PIN 13
unsigned long prevTime = 0;
const unsigned long interval = 500; // 500ms切换一次状态
bool ledState = LOW;

void setup() {
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  unsigned long curTime = millis();
  if (curTime - prevTime >= interval) {
    prevTime = curTime;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
