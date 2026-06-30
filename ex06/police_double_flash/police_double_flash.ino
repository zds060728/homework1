// 作业6 (ex06): 警车双闪灯效 (双通道PWM)
//
// ★ 无电阻版本: 板载LED显示通道A, 串口绘图器显示双通道波形
//   打开 Arduino IDE → 工具 → 串口绘图器 (波特率115200)
//   可以看到红蓝两条反相正弦波曲线
//
// ★ 加分项: 有220Ω电阻后, 在GPIO5接第二个LED即可看到实物双闪
//   接线: GPIO5 → 220Ω → LED(+) → LED(-) → GND

const int ledPinA = 2;   // LED A (板载, 无需接线)
const int ledPinB = 5;   // LED B (需要220Ω电阻+LED, 暂时留空)

const int freq = 5000;
const int resolution = 8;
const int maxDuty = 255;

void setup() {
  Serial.begin(115200);

  // 两路PWM
  ledcAttach(ledPinA, freq, resolution);
  ledcAttach(ledPinB, freq, resolution);

  Serial.println("==========================================");
  Serial.println("ex06: 警车双闪灯效");
  Serial.println("打开串口绘图器查看双通道波形!");
  Serial.println("板载LED显示通道A, 通道B需外接LED+电阻");
  Serial.println("==========================================");

  // 串口绘图器用: 打印表头标签
  Serial.println("dutyA,dutyB");
}

void loop() {
  // 阶段1: A 0→255, B 255→0
  for (int dutyA = 0; dutyA <= maxDuty; dutyA++) {
    int dutyB = maxDuty - dutyA;
    ledcWrite(ledPinA, dutyA);
    ledcWrite(ledPinB, dutyB);

    // 串口绘图器输出 (两条曲线: 反相)
    Serial.print(dutyA);
    Serial.print(",");
    Serial.println(dutyB);

    delay(10);
  }

  // 阶段2: A 255→0, B 0→255
  for (int dutyA = maxDuty; dutyA >= 0; dutyA--) {
    int dutyB = maxDuty - dutyA;
    ledcWrite(ledPinA, dutyA);
    ledcWrite(ledPinB, dutyB);

    Serial.print(dutyA);
    Serial.print(",");
    Serial.println(dutyB);

    delay(10);
  }
}
