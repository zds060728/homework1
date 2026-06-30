// 作业9 (ex09): 实时传感器Web仪表盘
//
// 功能说明:
//   网页上显示着一个实时跳动的数字。
//   当手逐渐靠近ESP32触摸引脚时, 网页上的数字实时变小;
//   手指离开时, 数值恢复。实现类似仪器仪表的数据实时监控面板。
//
// 关键技术:
//   - WiFi AP + WebServer
//   - RESTful API端点: GET /api/touch 返回JSON
//   - HTML页面使用AJAX (fetch)定时拉取传感器数据
//   - 实时数据可视化 (数值+进度条)
//   - touchRead() 读取触摸传感器模拟量
//
// 使用方法:
//   1. 连接WiFi: ESP32-Dashboard (密码: 12345678)
//   2. 浏览器打开: http://192.168.4.1
//   3. 观察实时数值, 用手靠近/触摸引脚看数值变化
//
// 硬件连接:
//   - 触摸引脚: T0 (GPIO 4), 连接一根导线作为感应天线
//   - 板载LED (GPIO 2) 用于辅助指示

#include <WiFi.h>
#include <WebServer.h>
// 无需额外库, 用字符串拼接构建JSON

const int ledPin = 2;
const int touchPin = T0;

// ========== WiFi AP 参数 ==========
const char* ssid = "ESP32-Dashboard";
const char* password = "12345678";
// ===================================

WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 启动WiFi AP
  WiFi.softAP(ssid, password);

  Serial.println("==========================================");
  Serial.println("ex09: 实时传感器Web仪表盘");
  Serial.print("WiFi名称: "); Serial.println(ssid);
  Serial.print("密码:     "); Serial.println(password);
  Serial.print("IP地址:   ");
  Serial.println(WiFi.softAPIP());
  Serial.println("打开浏览器查看实时触摸传感器数据");
  Serial.println("==========================================");

  // 注册路由
  server.on("/", handleRoot);              // 主页仪表盘
  server.on("/api/touch", handleApiTouch); // API: 返回触摸传感器JSON数据
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Web服务器已启动! 等待客户端连接...");
}

void loop() {
  server.handleClient();

  // 板载LED根据触摸状态亮灭 (辅助指示)
  int touchVal = touchRead(touchPin);
  if (touchVal < 500) {
    digitalWrite(ledPin, HIGH);  // 触摸时LED亮
  } else {
    digitalWrite(ledPin, LOW);   // 未触摸时LED灭
  }

  delay(10);
}

// ===== 主页: 仪表盘HTML =====
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 传感器仪表盘</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Courier New', 'SF Mono', 'Consolas', monospace;
      background: #0a0e14;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      color: #e0e0e0;
    }
    .dashboard {
      background: #121820;
      border: 2px solid #1e2a38;
      border-radius: 20px;
      padding: 35px 30px;
      width: 90%;
      max-width: 480px;
      box-shadow: 0 0 40px rgba(0,150,255,0.1);
    }
    .header {
      text-align: center;
      margin-bottom: 25px;
      border-bottom: 1px solid #1e2a38;
      padding-bottom: 15px;
    }
    .header h1 {
      font-size: 1.3em;
      color: #4a9eff;
      letter-spacing: 2px;
    }
    .header .subtitle {
      font-size: 0.7em;
      color: #556;
      margin-top: 5px;
      letter-spacing: 3px;
      text-transform: uppercase;
    }
    .main-value {
      text-align: center;
      margin: 20px 0;
    }
    .label {
      font-size: 0.8em;
      color: #667;
      letter-spacing: 2px;
      text-transform: uppercase;
      margin-bottom: 8px;
    }
    .value {
      font-size: 5em;
      font-weight: 700;
      color: #0ff;
      text-shadow: 0 0 30px rgba(0,255,255,0.4);
      transition: color 0.3s;
      letter-spacing: 2px;
    }
    .value.near { color: #ff0; text-shadow: 0 0 30px rgba(255,255,0,0.5); }
    .value.touch { color: #f33; text-shadow: 0 0 40px rgba(255,50,50,0.6); }
    .progress-container {
      margin: 20px 0;
      background: #0d1117;
      border-radius: 10px;
      height: 24px;
      overflow: hidden;
      border: 1px solid #1e2a38;
    }
    .progress-bar {
      height: 100%;
      border-radius: 9px;
      transition: width 0.2s, background 0.3s;
      background: linear-gradient(90deg, #0ff, #0af);
    }
    .progress-bar.near { background: linear-gradient(90deg, #ff0, #fa0); }
    .progress-bar.touch { background: linear-gradient(90deg, #f33, #f06); }
    .status-row {
      display: flex;
      justify-content: space-between;
      font-size: 0.75em;
      color: #556;
      margin-top: 15px;
    }
    .status-indicator {
      display: flex;
      align-items: center;
      gap: 6px;
    }
    .dot {
      width: 8px; height: 8px;
      border-radius: 50%;
      background: #0f0;
      animation: blink 1s infinite;
    }
    @keyframes blink {
      0%, 100% { opacity: 1; }
      50% { opacity: 0.3; }
    }
    .history {
      margin-top: 25px;
      border-top: 1px solid #1e2a38;
      padding-top: 15px;
    }
    .history h3 {
      font-size: 0.8em;
      color: #556;
      letter-spacing: 2px;
      margin-bottom: 10px;
    }
    .history-bars {
      display: flex;
      align-items: flex-end;
      gap: 4px;
      height: 60px;
    }
    .history-bar {
      flex: 1;
      background: #1a3040;
      border-radius: 2px 2px 0 0;
      min-height: 4px;
      transition: height 0.3s, background 0.3s;
    }
    .state-text {
      text-align: center;
      font-size: 1em;
      margin-top: 10px;
      font-weight: 600;
      letter-spacing: 2px;
    }
  </style>
</head>
<body>
  <div class="dashboard">
    <div class="header">
      <h1>📡 ESP32 传感器仪表盘</h1>
      <div class="subtitle">Real-Time Touch Sensor Monitor</div>
    </div>

    <div class="main-value">
      <div class="label">触摸传感器 · 模拟量</div>
      <div class="value" id="mainValue">--</div>
      <div class="state-text" id="stateText">等待数据...</div>
    </div>

    <div class="progress-container">
      <div class="progress-bar" id="progressBar" style="width: 0%"></div>
    </div>

    <div class="status-row">
      <div class="status-indicator">
        <div class="dot"></div>
        <span>实时监控中</span>
      </div>
      <span id="updateTime">--</span>
    </div>

    <div class="history">
      <h3>📊 近20次读数趋势</h3>
      <div class="history-bars" id="historyBars"></div>
    </div>
  </div>

  <script>
    const MAX_HISTORY = 20;
    let history = [];

    // 初始化历史柱状条
    function initHistoryBars() {
      let container = document.getElementById('historyBars');
      container.innerHTML = '';
      for (let i = 0; i < MAX_HISTORY; i++) {
        let bar = document.createElement('div');
        bar.className = 'history-bar';
        bar.style.height = '4px';
        container.appendChild(bar);
      }
    }

    // 更新历史柱状图
    function updateHistoryBars(newValue) {
      history.push(newValue);
      if (history.length > MAX_HISTORY) history.shift();

      let bars = document.getElementById('historyBars').children;
      // 触摸传感器的正常值范围大约在 5-80 (根据环境变化)
      let minVal = Math.min(...history);
      let maxVal = Math.max(...history);
      let range = maxVal - minVal || 1;

      for (let i = 0; i < MAX_HISTORY; i++) {
        let bar = bars[i];
        if (i < history.length) {
          let h = ((history[i] - minVal) / range) * 56 + 4;  // 4-60px
          bar.style.height = h + 'px';
          // 值越低 = 越接近 = 颜色越暖
          let ratio = history[i] / 1500;
          if (ratio < 0.25) bar.style.background = '#f33';
          else if (ratio < 0.5) bar.style.background = '#fa0';
          else bar.style.background = '#0af';
        } else {
          bar.style.height = '4px';
          bar.style.background = '#1a3040';
        }
      }
    }

    // 更新仪表盘
    function updateDisplay(data) {
      let touchValue = data.touch;
      let timestamp = data.time;

      // 主数值显示
      let valueEl = document.getElementById('mainValue');
      valueEl.textContent = touchValue;

      // 根据触摸程度改变颜色
      let progressBar = document.getElementById('progressBar');
      let stateText = document.getElementById('stateText');

      valueEl.className = 'value';
      progressBar.className = 'progress-bar';

      if (touchValue < 200) {
        valueEl.classList.add('touch');
        progressBar.classList.add('touch');
        stateText.textContent = '🔴 已触摸 (Touched)';
        stateText.style.color = '#f33';
      } else if (touchValue < 600) {
        valueEl.classList.add('near');
        progressBar.classList.add('near');
        stateText.textContent = '🟡 手指靠近 (Near)';
        stateText.style.color = '#ff0';
      } else {
        stateText.textContent = '🟢 无触摸 (Idle)';
        stateText.style.color = '#0f0';
      }

      // 进度条 (归一化: 假设范围 0-80, 触摸值越小进度越满)
      let normalized = Math.max(0, Math.min(100, ((1500 - touchValue) / 1500) * 100));
      progressBar.style.width = normalized + '%';

      // 更新时间
      document.getElementById('updateTime').textContent =
        new Date(timestamp).toLocaleTimeString();

      // 更新柱状图
      updateHistoryBars(touchValue);
    }

    // AJAX 拉取传感器数据
    function fetchData() {
      fetch('/api/touch')
        .then(response => response.json())
        .then(data => updateDisplay(data))
        .catch(err => {
          document.getElementById('stateText').textContent = '⚠ 连接错误';
          document.getElementById('stateText').style.color = '#f90';
        });
    }

    // 初始化
    initHistoryBars();
    fetchData();  // 立即获取一次

    // 每300ms刷新一次 (约3次/秒, 实现实时效果)
    setInterval(fetchData, 300);
  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html; charset=utf-8", html);
}

// ===== API: 返回触摸传感器JSON数据 =====
// GET /api/touch
// 返回: {"touch": 45, "time": 1234567890}
void handleApiTouch() {
  int touchValue = touchRead(touchPin);

  // 直接用字符串拼接构建JSON (无需ArduinoJson库)
  String jsonStr = "{\"touch\":" + String(touchValue) + ",\"time\":" + String(millis()) + "}";

  // 添加CORS头, 允许跨域访问
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", jsonStr);

  // 串口输出 (观察数据变化, 可注释以减少输出)
  // Serial.print("[API] touch=");
  // Serial.println(touchValue);
}

void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}
