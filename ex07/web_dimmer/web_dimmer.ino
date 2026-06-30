// 作业7 (ex07): Web网页端无极调光器
//
// 功能说明:
//   手机或电脑浏览器打开ESP32网页, 页面上出现一个滑动条。
//   拖动滑动条时, 开发板上的LED亮度会随着滑动条的位置实时、平滑地改变。
//
// 关键技术:
//   - WiFi AP模式 (ESP32作为热点)
//   - WebServer 提供HTML页面
//   - HTML <input type="range"> 滑动条
//   - JavaScript fetch() 发送GET请求携带亮度值
//   - ESP32解析URL参数并设置PWM占空比
//
// 使用方法:
//   1. 上传程序后, 用手机/电脑搜索WiFi: ESP32-Dimmer
//   2. 连接密码: 12345678
//   3. 浏览器打开: http://192.168.4.1
//   4. 拖动滑动条即可调节LED亮度
//
// 硬件连接:
//   - LED: GPIO 2 (板载LED, PWM输出)

#include <WiFi.h>
#include <WebServer.h>

const int ledPin = 2;

// ========== WiFi AP 参数 ==========
const char* ssid = "ESP32-Dimmer";
const char* password = "12345678";
// ===================================

// ========== PWM参数 ==========
const int freq = 5000;
const int resolution = 8;    // 8位 (0-255)
// =============================

WebServer server(80);  // Web服务器监听80端口

void setup() {
  Serial.begin(115200);

  // 初始化PWM
  ledcAttach(ledPin, freq, resolution);
  ledcWrite(ledPin, 0);  // 初始熄灭

  // 启动WiFi AP模式
  WiFi.softAP(ssid, password);
  Serial.println("==========================================");
  Serial.println("ex07: Web网页端无极调光器");
  Serial.print("WiFi名称: "); Serial.println(ssid);
  Serial.print("连接密码: "); Serial.println(password);
  Serial.print("IP地址:   ");
  Serial.println(WiFi.softAPIP());  // 默认 192.168.4.1
  Serial.println("浏览器打开以上IP地址即可使用");
  Serial.println("==========================================");

  // 注册路由
  server.on("/", handleRoot);        // 主页 (HTML页面+滑动条)
  server.on("/set", handleSet);       // 设置亮度 (GET /set?value=128)
  server.onNotFound(handleNotFound);  // 404

  server.begin();
  Serial.println("Web服务器已启动!");
}

void loop() {
  server.handleClient();  // 处理Web请求
}

// ===== 主页: 返回带滑动条的HTML页面 =====
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: linear-gradient(135deg, #0f0c29, #302b63, #24243e);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      color: #fff;
    }
    .container {
      background: rgba(255,255,255,0.1);
      backdrop-filter: blur(20px);
      border-radius: 24px;
      padding: 40px 30px;
      text-align: center;
      width: 90%;
      max-width: 420px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.4);
    }
    h1 { font-size: 1.6em; margin-bottom: 8px; font-weight: 600; }
    .subtitle { font-size: 0.9em; opacity: 0.7; margin-bottom: 30px; }
    .brightness-display {
      font-size: 4em;
      font-weight: 700;
      margin: 15px 0;
      background: linear-gradient(90deg, #f9d423, #ff4e50);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      background-clip: text;
    }
    .led-icon {
      font-size: 3em;
      margin: 10px 0;
      transition: opacity 0.1s;
    }
    input[type=range] {
      -webkit-appearance: none;
      width: 100%;
      height: 12px;
      border-radius: 6px;
      background: linear-gradient(90deg, #333, #ffcc00);
      outline: none;
      margin: 20px 0;
      cursor: pointer;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 36px;
      height: 36px;
      border-radius: 50%;
      background: #fff;
      box-shadow: 0 4px 15px rgba(255,200,0,0.5);
      cursor: pointer;
      transition: 0.15s;
    }
    input[type=range]::-webkit-slider-thumb:active {
      transform: scale(1.15);
    }
    .percent-bar {
      display: flex;
      justify-content: space-between;
      font-size: 0.8em;
      opacity: 0.5;
    }
    .status {
      margin-top: 20px;
      font-size: 0.8em;
      opacity: 0.6;
    }
    .status.ok { color: #4f0; }
  </style>
</head>
<body>
  <div class="container">
    <h1>💡 ESP32 无极调光器</h1>
    <p class="subtitle">拖动下方滑动条调节LED亮度</p>
    <div class="led-icon" id="ledIcon">💡</div>
    <div class="brightness-display" id="brightValue">0%</div>
    <input type="range" id="slider" min="0" max="255" value="0"
           oninput="updateBrightness(this.value)">
    <div class="percent-bar">
      <span>灭</span><span>暗</span><span>中</span><span>亮</span><span>最亮</span>
    </div>
    <p class="status ok" id="status">● 已连接</p>
  </div>

  <script>
    function updateBrightness(val) {
      // 计算百分比显示
      let percent = Math.round((val / 255) * 100);
      document.getElementById('brightValue').textContent = percent + '%';

      // 根据亮度改变LED图标透明度
      let icon = document.getElementById('ledIcon');
      icon.style.opacity = 0.2 + (val / 255) * 0.8;

      // 发送GET请求到ESP32
      fetch('/set?value=' + val)
        .then(response => response.text())
        .then(text => {
          document.getElementById('status').textContent = '● 亮度: ' + percent + '%';
        })
        .catch(err => {
          document.getElementById('status').textContent = '● 连接失败';
        });
    }

    // 页面加载时, 获取初始亮度
    window.onload = function() {
      document.getElementById('slider').value = 0;
      updateBrightness(0);
    };
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html; charset=utf-8", html);
}

// ===== 设置亮度: GET /set?value=128 =====
void handleSet() {
  if (server.hasArg("value")) {
    int value = server.arg("value").toInt();

    // 限制范围 0-255
    if (value < 0) value = 0;
    if (value > 255) value = 255;

    // 设置PWM占空比
    ledcWrite(ledPin, value);

    Serial.print("[调光] 亮度: ");
    Serial.print(value);
    Serial.print("/255 (");
    Serial.print(map(value, 0, 255, 0, 100));
    Serial.println("%)");

    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing 'value' parameter");
  }
}

// ===== 404处理 =====
void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}
