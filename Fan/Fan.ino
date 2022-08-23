#define BLINKER_WIFI
//#define BLINKER_ESP_SMARTCONFIG // 这个就设置点灯EspTouch
//#define BLINKER_MIOT_OUTLET   //小爱同学
#define BLINKER_MIOT_FAN
//#define BLINKER_MIOT_MULTI_OUTLET    //设置为小爱多个插座的模式
//#define BLINKER_ALIGENIE_OUTLET //天猫精灵
//#define BLINKER_ALIGENIE_MULTI_OUTLET  //设置为天猫精灵多个插座的模式
#define BLINKER_ALIGENIE_FAN
#define BLINKER_DUEROS_FAN

#include <Blinker.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h> //闪存文件系统

//0fbae1d3bba4  壁扇
//ad2634fb78ab  家里风扇
//bdcdc3404089  操作间壁扇
//88d4ebf71de6  循环扇

//***************配网*********************

String AP_NAME = "风扇_" + (String)ESP.getChipId(); //wifi名字
const byte DNS_PORT = 53;//DNS端口号
IPAddress apIP(192, 168, 66, 1);//esp8266-AP-IP地址
ESP8266WebServer server(80);//创建WebServer
DNSServer dnsServer;

const char* auth;
const char* ssid;
const char* pswd;

int ledPin = 15;

static const char HTML[] PROGMEM = R"KEWL(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset="utf-8">
  <title>风扇配网</title>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
</head>
<body>
<div class="main">
  <h1>风扇配网</h1><br>
  <form name='input' action='/' method='POST'>
        <div class='bb'>WiFi名称:</div> <br>
        <input class='input' type='text' value='' placeholder='输入WiFi名称' name='ssid'><br><br>
        <div class='bb'>WiFi密码:</div><br>
        <input class='input' type='password' value='' placeholder='输入WiFi密码' name='password'><br><br>
        <div class='bb'>点灯密钥:</div><br>
        <input class='input' type='text' value='' placeholder='输入点灯密钥' name='authkey'><br><br><br><br>
        <input style='font-weight:bold ;font-size:x-large;' class='input' type='submit' value='配网'>
   </form>
   </div>
</body>
<style>
    html,body{
        width: 100%;
        height: 100%;
        margin: 0;
        padding: 0;
    }
    .main{
        width: 100%;
        height: 100%;
        margin: 0;
        padding: 0;
        text-align: center;
        font-weight:bold ;
    }
    .input{
        resize: none;
        outline: none;
        margin: 4px auto;
        height: 45px;
        width: 80%;
        border: 1px solid #d0d1ce;
        padding-left: 15px;
        font-size: 14px;
        color: #000;
        margin-left: 10px;
        border-radius: 10px;
        border: 1px solid #dcdfe6;
        background-color: #ffffff;
    } 
    .bb{
        width: 80%;
        margin: 0px auto;
        text-align: left;
        padding-left: 2px;
        height: 0px;
        font-weight:bold ;
    }
</style>
</html>)KEWL";


void handleRoot() {//访问主页回调函数
  server.send(200, "text/html", HTML);
}

bool web = false;
void handleRootPost() {//Post回调函数
  web = false;
  if (server.hasArg("ssid")) {//判断是否有账号参数
    ssid = server.arg("ssid").c_str(); //将账号参数拷贝到my_ssid中
    Serial.println("WiFi账号：" + (String)ssid);
  } else {//没有参数
    Serial.println("错误，未找到WiFi账号");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi账号");//返回错误页面
    return;
  }
  if (server.hasArg("password")) {
    pswd = server.arg("password").c_str();
    Serial.println("WiFi密码：" + (String)pswd);
  } else {
    Serial.println("错误，未找到WiFi密码");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi密码");
    return;
  }
  if (server.hasArg("authkey")) {
    auth = server.arg("authkey").c_str();
    Serial.println("点灯密钥：" + (String)auth);
  } else {
    Serial.println("错误，未找到点灯密钥");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到点灯密钥");
    return;
  }
  server.send(200, "text/html", "<meta charset='utf-8'>上传成功，正在配网");//返回保存成功页面
  delay(2000);
  server.stop();
  connectNewWiFi();
}

void webServerStart() {
  Serial.println("正在启动web配网模式");
  if (dnsServer.start(DNS_PORT, "*", apIP)) { //判断将所有地址映射到esp8266的ip上是否成功
    Serial.println("DNS服务已启动");
  }
  else {
    Serial.println("DNS服务启动失败");
  }
  delay(200);
  server.on("/", HTTP_GET, handleRoot);//设置主页回调函数
  server.onNotFound(handleRoot);//设置无法响应的http请求的回调函数
  server.on("/", HTTP_POST, handleRootPost);//设置Post请求回调函数
  server.begin();//启动WebServer
  Serial.println("Web服务器已启动");
  delay(200);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (WiFi.softAP(AP_NAME)) {
    Serial.println("请连接热点：" + AP_NAME + ",浏览器输入\"192.168.66.1\"配置网络");
    web = true;
  }
  delay(200);
}

int rMillis = 0;
void reConnect() {
  int r = 0;
  rMillis = millis();
  while (web) {
    led(1);
    r = millis() - rMillis;
    Serial.println(r);
    if (r >= 180000) {
      connectWiFi();
      break;
    } else {
      server.handleClient();
      dnsServer.processNextRequest();
      if (!web) {
        break;
      }
    }
    delay(100);
  }
}

bool blinkerConnect(const char* auth, const char* ssid, const char* pswd) {
  WiFi.mode(WIFI_STA); // 更换wifi模式      //WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
  Blinker.begin(auth, ssid, pswd);
  int i = 0;
  Serial.println("正在连接点灯科技");
  while (!Blinker.connect()) {
    for (int j = 0; j < 10; j++) {
      delay(100);
      led(3);
    }
    Serial.println(i++);
    if (i > 20) { //如果60秒内没有连上，就开启Web配网 可适当调整这个时间
      Serial.println("点灯科技连接失败");
      led(1);
      break;//跳出 防止无限初始化
    }
  }
  if (Blinker.connect()) { //如果连接上 就输出IP信息 防止未连接上break后会误输出
    Serial.println("点灯科技连接成功");
    led(0);
    return true;
  } else {
    return false;
  }
}

void connectWiFi() {
  web = false;
  if (SPIFFS.begin()) {   // 打开闪存文件系统
    Serial.println("");
    Serial.println("闪存文件系统打开成功");
  } else {
    Serial.println("");
    Serial.println("闪存文件系统打开失败");
  }
  if (SPIFFS.exists("/WiFi.json"))     // exists 判断有没有config.json这个文件
  {
    Serial.println("存在配置信息，正在自动连接");
    const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 156; //分配一个内存空间
    DynamicJsonDocument doc(capacity); // 声明json处理对象
    File configJson = SPIFFS.open("/WiFi.json", "r");
    deserializeJson(doc, configJson); // json数据序列化
    auth = doc["auth"];
    ssid = doc["ssid"];
    pswd = doc["pswd"];
    Serial.println(auth);
    Serial.println(ssid);
    Serial.println(pswd);
    configJson.close();
    if (!blinkerConnect(auth, ssid, pswd)) {
      webServerStart();
    }
  } else {
    Serial.println("不存在配置信息，正在打开web配网模式");
    webServerStart();
  }
}

void connectNewWiFi(void) {
  if (!blinkerConnect(auth, ssid, pswd)) {
    webServerStart();
  } else {
    removeWiFiConfig();
    String json;
    json += "{\"ssid\":\"";
    json += ssid;
    json += "\",\"pswd\":\"";
    json += pswd;
    json += "\",\"auth\":\"";
    json += auth;
    json += "\"}";
    File wifiConfig = SPIFFS.open("/WiFi.json", "w");
    wifiConfig.println(json);//将数据写入config.json文件中
    Serial.println("配置文件写入成功！");
    wifiConfig.close();
  }
}

void removeWiFiConfig() {  //移除缓存中配置信息文件
  if (SPIFFS.exists("/WiFi.json")) { // 判断有没有config.json这个文件
    if (SPIFFS.remove("/WiFi.json")) {
      Serial.println("删除旧WiFi配置");
    }
    else {
      Serial.println("删除旧WiFi配置失败");
    }
  }
}

void keepOnline() {
  if (!Blinker.connect()) {
    led(1);
  } else {
    //led(0);
  }
}

void led(int state) {
  switch (state) {
    case 0:
      digitalWrite(ledPin, HIGH);
      break;
    case 1:
      digitalWrite(ledPin, LOW);
      break;
    case 2:
      digitalWrite(ledPin, LOW);
      delay(500);
      digitalWrite(ledPin, HIGH);
      break;
    case 3:
      digitalWrite(ledPin, !digitalRead(ledPin));
      break;
  }
}
//****************************配网**********************

int L1 = 3; //RXD 3 1档
int L2 = 0; //IO0   2档
int L3 = 2; //IO2   3档
int L4 = 1; //TXD 1 摆风

int fanLevel = 0;
int hs = 0;

BlinkerButton btnHand("btn-hand"); //定义按钮键名
BlinkerSlider sliderSpeed("ran-speed");//定义滑块

void miotLevel(uint8_t level) {
  BLINKER_LOG("need set level:", level);
  setFan(level);
  fanLevel = level;
  BlinkerMIOT.level(level);
  BlinkerMIOT.print();
}

void miotHSwingState(const String & state) {//hs左右摆风
  BLINKER_LOG("need set HSwing state:", state);
  if (state == BLINKER_CMD_ON) {
    setFan(4); //开启左右摆风
    BlinkerMIOT.hswing("on");
    BlinkerMIOT.print();
  } else if (state == BLINKER_CMD_OFF) {
    setFan(5); //关闭左右摆风
    BlinkerMIOT.hswing("off");
    BlinkerMIOT.print();
  }
}

void miotVSwingState(const String & state) {//vs上下摆风
  BLINKER_LOG("need set VSwing state:", state);
  if(state==BLINKER_CMD_ON){
    //setFan(4);
    BlinkerMIOT.vswing("on");
    BlinkerMIOT.print();
  }else if(state==BLINKER_CMD_OFF){
    //setFan(5);
    BlinkerMIOT.vswing("off");
    BlinkerMIOT.print();
  }
}

void miotPowerState(const String & state)     //小爱同学控制指令
{
  BLINKER_LOG("need power state: ", state);
  if (state == "off") {
    setFan(0);//关风扇
  } else {
    setFan(3);//风扇3档
  }
  BlinkerMIOT.print();

}

void miotQuery(int32_t queryCode) {
  BLINKER_LOG("MIOT Query codes:", queryCode);
  switch (queryCode) {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
      BLINKER_LOG("MIOT Query All");
      BlinkerMIOT.powerState(fanLevel ? "on" : "off");
      BlinkerMIOT.hswing(hs ? "on" : "off");
      //      BlinkerMIOT.vswing(vsState ? "on" : "off");
      BlinkerMIOT.level(fanLevel);
      BlinkerMIOT.print();
      break;
    default:
      BlinkerMIOT.powerState(fanLevel ? "on" : "off");
      BlinkerMIOT.hswing(hs ? "on" : "off");
      //      BlinkerMIOT.vswing(vsState ? "on" : "off");
      BlinkerMIOT.level(fanLevel);
      BlinkerMIOT.print();
      break;
  }
}

void aligenieLevel(uint8_t level) {
  BLINKER_LOG("need set level:", level);
  setFan(level);
  BlinkerAliGenie.print();
}

void aligenieHSwingState(const String & state) {//hs左右摆风
  BLINKER_LOG("need set HSwing state:", state);
  if (state == BLINKER_CMD_ON) {
    setFan(4); //开启左右摆风
    BlinkerAliGenie.print();
  } else if (state == BLINKER_CMD_OFF) {
    setFan(5); //关闭左右摆风
    BlinkerAliGenie.print();
  }
}

void aligenieMode(const String & mode)
{
  BLINKER_LOG("need set mode: ", mode);

  BlinkerAliGenie.mode(mode);
  BlinkerAliGenie.print();
}

void aligenieVSwingState(const String & state) {//vs上下摆风
  BLINKER_LOG("need set VSwing state:", state);
  if (state == BLINKER_CMD_ON) {
    //setFan(4);
    BlinkerAliGenie.vswing("on");
    BlinkerAliGenie.print();
  } else if (state == BLINKER_CMD_OFF) {
    //setFan(5);
    BlinkerAliGenie.vswing("off");
    BlinkerAliGenie.print();
  }
}

void aligeniePowerState(const String & state)//天猫精灵
{
  BLINKER_LOG("need power state: ", state);
  if (state == "off") {
    setFan(0);//关风扇
  } else {
    setFan(3);//风扇3档
  }
  BlinkerAliGenie.print();

}

void aligenieQuery(int32_t queryCode) {
  BLINKER_LOG("MIOT Query codes:", queryCode);
  switch (queryCode) {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
      BLINKER_LOG("MIOT Query All");
      BlinkerAliGenie.powerState(fanLevel ? "on" : "off");
      BlinkerAliGenie.hswing(hs ? "on" : "off");
      //      BlinkerAliGenie.vswing(vsState ? "on" : "off");
      BlinkerAliGenie.level(fanLevel);
      BlinkerAliGenie.print();
      break;
    default:
      BlinkerAliGenie.powerState(fanLevel ? "on" : "off");
      BlinkerAliGenie.hswing(hs ? "on" : "off");
      //      BlinkerAliGenie.vswing(vsState ? "on" : "off");
      BlinkerAliGenie.level(fanLevel);
      BlinkerAliGenie.print();
      break;
  }
}

void duerLevel(uint8_t level)
{
  BLINKER_LOG("need set level: ", level);
  // 0:AUTO MODE, 1-3 LEVEL

  setFan(level);
  BlinkerDuerOS.print();
}

void duerRelativeLevel(int32_t level)
{
  BLINKER_LOG("need set relative level: ", level);
  // 0:AUTO MODE, 1-3 LEVEL
  setFan(level + 1);
  BlinkerDuerOS.print();
}

void duerMode(const String & mode)
{
  BLINKER_LOG("need set mode: ", mode);
  // NIGHT：夜间
  // SWING：摆动/摆风
  // SINGLE：单人
  // MULTI：多人
  // SPURT：喷射
  // SPREAD：扩散
  // QUIET：安静
  // NORMAL：正常风速/适中风速/一般风速
  // POWERFUL：强效
  // MUTE：静音风速
  // NATURAL：自然风速
  // BABY：无感风速/轻微风速
  // COMFORTABLE：舒适风速
  // FEEL：人感风速
  BlinkerDuerOS.mode(mode);
  BlinkerDuerOS.print();
}

void duerPowerState(const String & state)
{
  BLINKER_LOG("need set power state: ", state);

  if (state == BLINKER_CMD_ON) {
    setFan(3);
    BlinkerDuerOS.print();
  }
  else {
    setFan(0);
    BlinkerDuerOS.print();
  }
}

void duerQuery(int32_t queryCode)
{
  BLINKER_LOG("DuerOS Query codes: ", queryCode);

  switch (queryCode)
  {
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
      BLINKER_LOG("DuerOS Query power state");
      BlinkerDuerOS.powerState(fanLevel ? "on" : "off");
      BlinkerDuerOS.level(fanLevel);
      BlinkerDuerOS.print();
      break;
    case BLINKER_CMD_QUERY_TIME_NUMBER :
      BLINKER_LOG("DuerOS Query time");
      BlinkerDuerOS.time(millis());
      BlinkerDuerOS.print();
      break;
    default :
      BlinkerDuerOS.powerState(fanLevel ? "on" : "off");
      BlinkerDuerOS.level(fanLevel);
      BlinkerDuerOS.print();
      break;
  }
}

void dataRead(const String & data)      // 如果未绑定的组件被触发，则会执行其中内容
{
  BLINKER_LOG("Blinker readString: ", data);
  Blinker.vibrate();
  uint32_t BlinkerTime = millis();
  Blinker.print("millis", BlinkerTime);
}

void btnHand_callback(const String & state)
{
  BLINKER_LOG("get button state: ", state);
  if (state == "tap") {
    if  (digitalRead(L4)) {
      btnHand.color("#808080");
      btnHand.print("off");
      setFan(5);
    } else {
      btnHand.color("#00ff00");
      btnHand.print("on");
      setFan(4);
    }
  } else if (state == "on" or state == "press") {
    btnHand.color("#00ff00");
    btnHand.print("on");
    setFan(4);
  } else if (state == "off") {
    btnHand.color("#808080");
    btnHand.print("off");
    setFan(5);
  }
}

void sliderSpeed_callback(int32_t value)
{
  BLINKER_LOG("get slider value: ", value);
  switch (value) {
    case 0:
      btnHand.color("#808080");
      btnHand.print("off");
      sliderSpeed.color("#808080");
      break;
    case 1:
      sliderSpeed.color("#00ff00");
      break;
    case 2:
      sliderSpeed.color("#0000ff");
      break;
    case 3:
      sliderSpeed.color("#ff0000");
      break;
  }
  sliderSpeed.print(value);
  setFan(value);
}

int fanMillis = 0;
void setFan(int i) {
  while (millis() - fanMillis < 1000) {
    delay(500);
  }
  fanMillis = millis();
  switch (i) {
    case 0:
      delay(200);
      digitalWrite(L2, HIGH);
      digitalWrite(L1, HIGH);
      digitalWrite(L3, HIGH);
      digitalWrite(L4, HIGH);
      fanLevel = 0;
      break;
    case 1:
      delay(200);
      digitalWrite(L2, HIGH);
      digitalWrite(L3, HIGH);
      delay(200);
      digitalWrite(L1, LOW);
      fanLevel = 1;
      break;
    case 2:
      delay(200);
      digitalWrite(L3, HIGH);
      digitalWrite(L1, HIGH);
      delay(200);
      digitalWrite(L2, LOW);
      fanLevel = 2;
      break;
    case 3:
      delay(200);
      digitalWrite(L1, HIGH);
      digitalWrite(L2, HIGH);
      delay(200);
      digitalWrite(L3, LOW);
      fanLevel = 3;
      break;
    case 4:
      delay(200);
      digitalWrite(L4, LOW);
      hs = 1;
      break;
    case 5:
      delay(200);
      digitalWrite(L4, HIGH);
      hs = 0;
      break;
  }
}

void heartbeat() {
  switch (fanLevel) {
    case 0:
      sliderSpeed.color("#808080");
      break;
    case 1:
      sliderSpeed.color("#00ff00");
      break;
    case 2:
      sliderSpeed.color("#0000ff");
      break;
    case 3:
      sliderSpeed.color("#ff0000");
      break;
  }
  sliderSpeed.print(fanLevel);
  if (hs) {
    btnHand.color("#00ff00");
    btnHand.print("on");
  } else {
    btnHand.color("#808080");
    btnHand.print("off");
  }
}
void setup()
{
  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);
  pinMode(L4, OUTPUT);

  setFan(0);

  Serial.begin(115200);
  Blinker.attachData(dataRead);

  BlinkerMIOT.attachPowerState(miotPowerState);
  BlinkerMIOT.attachQuery(miotQuery);
  BlinkerMIOT.attachLevel(miotLevel);
  BlinkerMIOT.attachHSwing(miotHSwingState);
  //BlinkerMIOT.attachVSwing(miotVSwingState);

  BlinkerAliGenie.attachPowerState(aligeniePowerState);//天猫
  BlinkerAliGenie.attachQuery(aligenieQuery);//天猫
  BlinkerAliGenie.attachLevel(aligenieLevel);
  BlinkerAliGenie.attachHSwing(aligenieHSwingState);
  BlinkerAliGenie.attachMode(aligenieMode);
  //BlinkerMIOT.attachVSwing(aligenieVSwingState);

  BlinkerDuerOS.attachPowerState(duerPowerState);
  BlinkerDuerOS.attachLevel(duerLevel);
  BlinkerDuerOS.attachRelativeLevel(duerRelativeLevel);
  BlinkerDuerOS.attachMode(duerMode);
  BlinkerDuerOS.attachQuery(duerQuery);

  BUILTIN_SWITCH.attach(switch_callback);
  Blinker.attachHeartbeat(heartbeat);
  Blinker.attachSummary(summary);

  //BLINKER_DEBUG.stream(Serial);

  connectWiFi();

  btnHand.attach(btnHand_callback);
  sliderSpeed.attach(sliderSpeed_callback);
}

void loop()
{
  keepOnline();
  reConnect();
  Blinker.run();
}

void switch_callback(const String & state)
{
  BLINKER_LOG("get switch state: ", state);
  if (state == BLINKER_CMD_ON) {
    setFan(3);
    BUILTIN_SWITCH.print("on");
  }
  else if (state == BLINKER_CMD_OFF) {
    setFan(0);
    BUILTIN_SWITCH.print("off");
  }
}

String summary() {
  String data;
  if (fanLevel != 0) {
    if (hs) {
      data = "电扇: " + (String)fanLevel + "档;左右摆风：开";
    } else {
      data = "电扇: " + (String)fanLevel + "档;左右摆风：关";
    }
  } else {
    data = "电扇: 关";
  }
  return data;
}
