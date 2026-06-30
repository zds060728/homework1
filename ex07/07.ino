#include <WiFi.h>
#include <WebServer.h>

// 硬件配置
const char* ssid     = "刘栋WIN";
const char* password = "12346789";

#define TOUCH_PIN    4
#define ALARM_LED    2
#define TOUCH_THRESH 150

// 系统状态标记
bool isArm    = false;
bool isAlarm  = false;
unsigned long flashTimer = 0;
const unsigned long flashInterval = 80;

WebServer server(80);

// 网页页面
String buildHtml()
{
  String statusText;
  if(isAlarm)
    statusText = "⚠️ 报警触发中！";
  else if(isArm)
    statusText = "🔒 已布防";
  else
    statusText = "🔓 已撤防";

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>物联网安防报警器</title>
<style>
body {font-family:Arial;text-align:center;margin-top:60px;}
.info {font-size:24px;margin:20px 0;font-weight:bold;}
.btn {padding:12px 24px;font-size:18px;margin:0 10px;cursor:pointer;}
.arm {background:#2ecc71;color:white;border:none;}
.disarm {background:#e74c3c;color:white;border:none;}
</style>
</head>
<body>
<h1>ESP32 安防报警主机</h1>
<div class="info">当前系统状态：)" + statusText + R"rawliteral(</div>
<a href="/arm"><button class="btn arm">布防 Arm</button></a>
<a href="/disarm"><button class="btn disarm">撤防 Disarm</button></a>
</body>
</html>
)rawliteral";
  return html;
}

// 主页
void handleRoot()
{
  server.send(200, "text/html; charset=utf-8", buildHtml());
}

// 布防
void handleArm()
{
  isArm = true;
  server.sendHeader("Location", "/");
  server.send(303);
}

// 撤防，清空报警状态
void handleDisarm()
{
  isArm = false;
  isAlarm = false;
  digitalWrite(ALARM_LED, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup()
{
  Serial.begin(115200);
  pinMode(ALARM_LED, OUTPUT);
  digitalWrite(ALARM_LED, LOW);

  // WiFi连接
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.print("设备IP：");
  Serial.println(WiFi.localIP());

  // 绑定网页路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();
}

void loop()
{
  server.handleClient();

  int touchVal = touchRead(TOUCH_PIN);

  // 仅布防且无报警时检测触摸
  if(isArm && !isAlarm)
  {
    if(touchVal < TOUCH_THRESH)
    {
      isAlarm = true;
    }
  }

  // 非阻塞闪烁，不卡死网页
  unsigned long now = millis();
  if(isAlarm)
  {
    if(now - flashTimer > flashInterval)
    {
      digitalWrite(ALARM_LED, !digitalRead(ALARM_LED));
      flashTimer = now;
    }
  }
  else
  {
    digitalWrite(ALARM_LED, LOW);
  }

  delay(100);
}