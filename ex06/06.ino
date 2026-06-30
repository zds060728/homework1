#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "刘栋WIN";
const char* password = "12346789";
const int LED_PIN = 4;

// PWM 参数
const int freq = 5000;
const int resolution = 8; // 0~255

WebServer server(80);

// 生成网页，内置滑动条+JS异步请求
String makePage() {
  int currentDuty = ledcRead(LED_PIN);
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>LED PWM调光</title>
  <style>
    body {font-family:Arial; text-align:center; margin-top:50px;}
    .slider-box {margin:30px auto; width:80%; max-width:500px;}
    input[type="range"] {width:100%; height:12px;}
    button {padding:10px 20px; font-size:16px; margin:0 8px;}
    #valText {font-size:22px; color:#c00; font-weight:bold;}
  </style>
</head>
<body>
  <h1>网页滑动条 PWM 调光 LED</h1>
  <p>当前亮度：<span id="valText">)" + String(currentDuty) + R"rawliteral(</span></p>

  <div class="slider-box">
    <input type="range" id="pwmSlider" min="0" max="255" value=")" + String(currentDuty) + R"rawliteral(">
  </div>

  <a href="/on"><button>最大亮度</button></a>
  <a href="/off"><button>关闭LED</button></a>

<script>
// 获取滑块与显示文本
const slider = document.getElementById("pwmSlider");
const valText = document.getElementById("valText");

// 滑块拖动事件
slider.addEventListener("input", function(){
  let val = this.value;
  valText.innerText = val;
  // fetch异步发送亮度值，不刷新页面
  fetch(`/set?pwm=${val}`)
    .catch(err => console.log("请求异常", err));
});
</script>
</body>
</html>
)rawliteral";
  return html;
}

// 主页路由
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 全开 255
void handleOn() {
  ledcWrite(LED_PIN, 255);
  server.sendHeader("Location", "/");
  server.send(303);
}

// 全关 0
void handleOff() {
  ledcWrite(LED_PIN, 0);
  server.sendHeader("Location", "/");
  server.send(303);
}

// 接收滑块PWM数值路由
void handleSetPWM() {
  // 获取url参数 pwm=xxx
  int duty = server.arg("pwm").toInt();
  // 限制0~255范围防止越界
  if(duty < 0) duty = 0;
  if(duty > 255) duty = 255;
  ledcWrite(LED_PIN, duty);
  // 返回空响应，不需要跳转
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  // 初始化PWM通道
  ledcAttach(LED_PIN, freq, resolution);
  ledcWrite(LED_PIN, 0);

  // WiFi连接
  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("网页地址：http://");
  Serial.println(WiFi.localIP());

  // 绑定路由
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/set", handleSetPWM);

  server.begin();
}

void loop() {
  server.handleClient();
}