#include <WiFi.h>
#include <WebServer.h>

// WiFi配置
const char* ssid     = "刘栋WIN";
const char* password = "12346789";

// 硬件引脚
#define TOUCH_PIN    4

WebServer server(80);

// 网页页面：AJAX轮询实时刷新触摸数值
String buildHtml()
{
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>触摸传感器实时监测面板</title>
<style>
body {
  font-family: Arial;
  text-align: center;
  margin-top: 80px;
}
.title {
  font-size: 28px;
}
.data-box {
  font-size: 60px;
  color: #e63946;
  margin-top: 40px;
  font-weight: bold;
}
.desc {
  font-size: 18px;
  color: #555;
  margin-top: 15px;
}
</style>
</head>
<body>
<div class="title">ESP32 触摸传感器数据监控</div>
<div class="data-box">触摸实时值：<span id="touchNum">0</span></div>
<div class="desc">手指靠近/触摸引脚，数值下降；松手数值恢复</div>

<script>
// 定时拉取触摸数据，200ms刷新一次
function updateTouchValue() {
  fetch("/getData")
    .then(res => res.text())
    .then(value => {
      document.getElementById("touchNum").innerText = value;
    })
    .catch(err => {});
}
// 循环自动更新
setInterval(updateTouchValue, 200);
</script>
</body>
</html>
)rawliteral";
  return html;
}

// 主页路由：下发网页
void handleRoot()
{
  server.send(200, "text/html; charset=utf-8", buildHtml());
}

// 数据上报接口：返回触摸原始模拟量
void handleGetData()
{
  int touchValue = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(touchValue));
}

void setup()
{
  Serial.begin(115200);

  // WiFi连接
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  Serial.print("设备访问地址：");
  Serial.println(WiFi.localIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/getData", handleGetData);
  server.begin();
}

void loop()
{
  server.handleClient();
  delay(50);
}