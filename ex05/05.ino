// 定义两路LED引脚
const int ledPin = 2;
const int ledpin = 4;  

// PWM参数
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

void setup() {
  Serial.begin(115200);
  // 两路分别绑定独立PWM通道
  ledcAttach(ledPin, freq, resolution);
  ledcAttach(ledpin, freq, resolution);
}

void loop() {
  // LED2从0变亮到255，LED4同步从255变暗到0
  for(int i = 0; i <= 255; i++){   
    ledcWrite(ledPin, i);
    ledcWrite(ledpin, 255 - i);
    delay(10);
  }

  // LED2从255变暗到0，LED4同步从0变亮到255
  for(int i = 255; i >= 0; i--){
    ledcWrite(ledPin, i);
    ledcWrite(ledpin, 255 - i);
    delay(10);
  }
  
  Serial.println("Breathing cycle completed");
}