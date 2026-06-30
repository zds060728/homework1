// 作业8 (ex08): 物联网安防报警器模拟实验
//
// 功能说明:
//   将ESP32作为一台"安防主机"。
//   网页端提供两个按钮: "布防(Arm)"和"撤防(Disarm)"。
//   在布防状态下触碰引脚 → LED狂闪(报警状态), 即使手松开也不停止,
//   直到用户在网页端点击"撤防"。
//   未布防时触摸引脚无反应。
//
// 系统状态机:
//   DISARMED (未布防)  ──[点击Arm]──▶  ARMED (已布防)
//   ARMED              ──[触摸触发]──▶  ALARM (报警中, LED狂闪)
//   ALARM              ──[点击Disarm]─▶ DISARMED
//   ARMED              ──[点击Disarm]─▶ DISARMED
//
// 关键技术:
//   - WiFi AP + WebServer
//   - HTML按钮 + JavaScript fetch()
//   - 状态机 (DISARMED / ARMED / ALARM)
//   - 触摸传感器 (touchRead)
//   - 非阻塞LED闪烁 (millis)
//
// 使用方法:
//   1. 连接WiFi: ESP32-Alarm (密码: 12345678)
//   2. 浏览器打开: http://192.168.4.1
//   3. 点击"布防" → 触摸引脚触发报警 → 点击"撤防"解除
//
// 硬件连接:
//   - LED: GPIO 2 (板载LED)
//   - 触摸引脚: T0 (GPIO 4)

#include <WiFi.h>
#include <WebServer.h>

const int ledPin = 2;
const int touchPin = T0;

// ========== WiFi AP 参数 ==========
const char* ssid = "ESP32-Alarm";
const char* password = "12345678";
// ===================================

// ========== 触摸参数 ==========
const int TOUCH_THRESHOLD = 500;
// ==============================

// ========== 报警闪烁参数 ==========
const unsigned long ALARM_ON_MS = 100;   // 报警时LED亮100ms
const unsigned long ALARM_OFF_MS = 100;  // 报警时LED灭100ms (高频闪烁)
// ===================================

// ========== 系统状态枚举 ==========
enum AlarmState {
  DISARMED,  // 未布防: 触摸无反应
  ARMED,     // 已布防: 等待触发
  ALARM      // 报警中: LED狂闪, 直到撤防
};
// ===================================

AlarmState currentState = DISARMED;
unsigned long lastBlinkTime = 0;
bool alarmLedOn = false;

WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 启动WiFi AP
  WiFi.softAP(ssid, password);

  Serial.println("==========================================");
  Serial.println("ex08: 物联网安防报警器");
  Serial.print("WiFi名称: "); Serial.println(ssid);
  Serial.print("密码:     "); Serial.println(password);
  Serial.print("IP地址:   ");
  Serial.println(WiFi.softAPIP());
  Serial.println("状态: 未布防 (DISARMED)");
  Serial.println("==========================================");

  // 注册Web路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.on("/status", handleStatus);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Web服务器已启动!");
}

void loop() {
  server.handleClient();

  // ===== 状态机逻辑 =====

  switch (currentState) {
    case DISARMED:
      // 未布防: 不检测触摸, 保持LED熄灭
      // (触摸值仅在串口显示, 无实际触发)
      break;

    case ARMED:
      // 已布防: 检测触摸
      if (touchRead(touchPin) < TOUCH_THRESHOLD) {
        currentState = ALARM;
        digitalWrite(ledPin, HIGH);
        alarmLedOn = true;
        lastBlinkTime = millis();

        Serial.println("!!!!! 报警! 检测到入侵! LED开始狂闪 !!!!!");
        Serial.println("      请在网页端点击\"撤防\"解除报警");
      }
      break;

    case ALARM:
      // 报警中: LED高频闪烁 (非阻塞)
      {
        unsigned long now = millis();
        if (alarmLedOn && (now - lastBlinkTime >= ALARM_ON_MS)) {
          alarmLedOn = false;
          digitalWrite(ledPin, LOW);
          lastBlinkTime = now;
        } else if (!alarmLedOn && (now - lastBlinkTime >= ALARM_OFF_MS)) {
          alarmLedOn = true;
          digitalWrite(ledPin, HIGH);
          lastBlinkTime = now;
        }
      }
      break;
  }
}

// ===== 主页 HTML =====
void handleRoot() {
  String stateText;
  String stateColor;

  switch (currentState) {
    case DISARMED:
      stateText = "未布防";
      stateColor = "#888";
      break;
    case ARMED:
      stateText = "已布防 · 监控中";
      stateColor = "#0f0";
      break;
    case ALARM:
      stateText = "!!! 报警中 !!!";
      stateColor = "#f00";
      break;
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 安防报警器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      background: #0a0a0a;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      color: #fff;
    }
    .container {
      background: #1a1a2e;
      border-radius: 24px;
      padding: 40px 30px;
      text-align: center;
      width: 90%;
      max-width: 420px;
      box-shadow: 0 20px 60px rgba(0,0,0,0.6);
      border: 2px solid #333;
    }
    h1 { font-size: 1.5em; margin-bottom: 5px; }
    .status-circle {
      display: inline-block;
      width: 16px; height: 16px;
      border-radius: 50%;
      margin-right: 6px;
      vertical-align: middle;
      animation: )rawliteral";

  // 报警时状态指示灯闪烁
  if (currentState == ALARM) {
    html += "pulse 0.5s infinite";
  } else {
    html += "none";
  }

  html += R"rawliteral(;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.2; }
    }
    .state-text {
      font-size: 1.3em;
      font-weight: 700;
      margin: 20px 0;
      color: )rawliteral";

  html += stateColor;
  html += R"rawliteral(;
    }
    .icon {
      font-size: 5em;
      margin: 20px 0;
    }
    .btn {
      display: block;
      width: 100%;
      padding: 16px;
      margin: 12px 0;
      border: none;
      border-radius: 12px;
      font-size: 1.1em;
      font-weight: 600;
      cursor: pointer;
      transition: 0.2s;
      color: #fff;
    }
    .btn:active { transform: scale(0.96); }
    .btn-arm {
      background: linear-gradient(135deg, #11998e, #38ef7d);
      color: #000;
    }
    .btn-disarm {
      background: linear-gradient(135deg, #cb2d3e, #ef473a);
    }
    .btn:disabled {
      opacity: 0.35;
      pointer-events: none;
    }
    .info {
      margin-top: 20px;
      font-size: 0.8em;
      opacity: 0.5;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔒 ESP32 安防系统</h1>
    <div class="icon" id="icon">)rawliteral";

  // 根据状态显示不同图标
  switch (currentState) {
    case DISARMED: html += "🔓"; break;
    case ARMED:    html += "🛡️"; break;
    case ALARM:    html += "🚨"; break;
  }

  html += R"rawliteral(</div>
    <div class="state-text">
      <span class="status-circle" style="background: )rawliteral";
  html += stateColor;
  html += R"rawliteral(;"></span>
      <span id="stateLabel">)rawliteral";
  html += stateText;
  html += R"rawliteral(</span>
    </div>
    <button class="btn btn-arm" id="btnArm" onclick="sendCmd('arm')"
            )rawliteral";

  if (currentState == ALARM) html += "disabled";

  html += R"rawliteral(>🔒 布防 (Arm)</button>
    <button class="btn btn-disarm" id="btnDisarm" onclick="sendCmd('disarm')">🔓 撤防 (Disarm)</button>
    <p class="info" id="msg">触摸引脚将触发报警</p>
  </div>

  <script>
    function sendCmd(cmd) {
      fetch('/' + cmd)
        .then(r => r.text())
        .then(() => location.reload())
        .catch(() => {
          document.getElementById('msg').textContent = '连接失败, 请检查WiFi';
        });
    }

    // 自动刷新状态 (每2秒)
    setInterval(() => {
      fetch('/status')
        .then(r => r.text())
        .then(state => {
          if (state !== ')rawliteral";
  html += (currentState == DISARMED ? "DISARMED" : (currentState == ARMED ? "ARMED" : "ALARM"));
  html += R"rawliteral(') {
            location.reload();
          }
        });
    }, 2000);
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html; charset=utf-8", html);
}

// ===== 布防 =====
void handleArm() {
  currentState = ARMED;
  digitalWrite(ledPin, LOW);
  Serial.println("[命令] 布防 (ARMED) - 监控中...");
  server.send(200, "text/plain", "ARMED");
}

// ===== 撤防 =====
void handleDisarm() {
  currentState = DISARMED;
  digitalWrite(ledPin, LOW);
  alarmLedOn = false;
  Serial.println("[命令] 撤防 (DISARMED) - 报警已解除");
  server.send(200, "text/plain", "DISARMED");
}

// ===== 状态查询 (用于页面自动刷新) =====
void handleStatus() {
  switch (currentState) {
    case DISARMED: server.send(200, "text/plain", "DISARMED"); break;
    case ARMED:    server.send(200, "text/plain", "ARMED"); break;
    case ALARM:    server.send(200, "text/plain", "ALARM"); break;
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}
