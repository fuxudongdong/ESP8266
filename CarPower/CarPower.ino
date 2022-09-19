#define BLINKER_WIFI
#define BLINKER_MIOT_OUTLET //小爱同学
//#define BLINKER_MIOT_MULTI_OUTLET  //设置为小爱多个插座的模式
#define BLINKER_ALIGENIE_OUTLET //天猫精灵
//#define BLINKER_ALIGENIE_MULTI_OUTLET  //设置为天猫精灵多个插座的模式
#include <Blinker.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h> //闪存文件系统
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ritos.h" //多线程

String AP_NAME = "充电桩_" + (String)ESP.getChipId(); // wifi名字
const byte DNS_PORT = 53;                             // DNS端口号
IPAddress apIP(192, 168, 66, 1);                      // esp8266-AP-IP地址
ESP8266WebServer server(80);                          //创建WebServer
DNSServer dnsServer;
WiFiUDP ntpUDP;
NTPClient ntp(ntpUDP, "ntp1.aliyun.com", 60 * 60 * 8, 30 * 60 * 1000);

const char *auth;
const char *ssid;
const char *pswd;
int startH = 22;
int startM = 5;
int endH = 7;
int endM = 55;

bool isBlinker = 1;

int ledState = 0;
Ritos ledTare;

// esp01F
int ledPin = 2;
int r1 = 0;
int keyPin = 15;
int eState = 13;
// esp-01
// int ledPin = 2;
// int r1 = 0;
// int keyPin = 3;
// int eState = 1;

bool onlyLowPowerOn = true;
int power = 0;
//***************配网*********************

// d5f4f1151c3b 测试
// 91c916b2f584 充电桩

static const char HTML[] PROGMEM = R"KEWL(
<!DOCTYPE html>
<html lang='en'>
<head>
  <meta charset="utf-8">
  <title>充电桩配网</title>
  <meta charset='UTF-8'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
</head>
<body>
<div class="main">
  <h1>充电桩配网</h1><br>
  <form name='input' action='/' method='POST'>
        <div class='bb'>WiFi名称:</div> <br>
        <input class='input' type='text' value='xier-hx-01' placeholder='输入SSID' name='ssid'><br><br>
        <div class='bb'>WiFi密码:</div><br>
        <input class='input' type='password' value='xier0571@' placeholder='输入密码' name='pswd'><br><br>
        <div class='bb'>点灯密钥:</div><br>
        <input class='input' type='text' value='d5f4f1151c3b' placeholder='输入点灯密钥' name='auth'><br><br>
        <div class='bb'>谷电开始时间:
        <select class='select' name='startH'>
          <option value='22'>22</option>
          <option value ='00'>00</option>
          <option value ='23'>23</option>
          <option value='21'>21</option>
          <option value='20'>20</option>
          <option value='19'>19</option>
          <option value='18'>18</option>
          <option value='17'>17</option>
          <option value='16'>16</option>
          <option value='15'>15</option>
          <option value='14'>14</option>
          <option value='13'>13</option>
          <option value='12'>12</option>
          <option value='11'>11</option>
          <option value='10'>10</option>
          <option value='9'>09</option>
          <option value='8'>08</option>
          <option value='7'>07</option>
          <option value='6'>06</option>
          <option value='5'>05</option>
          <option value='4'>04</option>
          <option value='3'>03</option>
          <option value='2'>02</option>
          <option value='1'>01</option>
         </select>时
         <select class='select' name='startM'>
            <option value='5'>05</option>
            <option value='10'>10</option>
            <option value='15'>15</option>
            <option value='20'>20</option>
            <option value='25'>25</option>
            <option value='30'>30</option>
            <option value='35'>35</option>
            <option value='40'>40</option>
            <option value='45'>45</option>
            <option value='50'>50</option>
            <option value='55'>55</option>
            <option value='0'>00</option>
          </select>分
        </div><br><br>
        <div class='bb'>谷电结束时间:
       <select class='select' name='endH'>
          <option value='7'>07</option>
          <option value='1'>01</option>
          <option value='2'>02</option>
          <option value='3'>03</option>
          <option value='4'>04</option>
          <option value='5'>05</option>
          <option value='6'>06</option>
          <option value='8'>08</option>
          <option value='9'>09</option>
          <option value='10'>10</option>
          <option value='11'>11</option>
          <option value='12'>12</option>
          <option value='13'>13</option>
          <option value='14'>14</option>
          <option value='15'>15</option>
          <option value='16'>16</option>
          <option value='17'>17</option>
          <option value='18'>18</option>
          <option value='19'>19</option>
          <option value='20'>20</option>
          <option value='21'>21</option>
          <option value='22'>22</option>
          <option value ='23'>23</option>
          <option value ='0'>00</option>
        </select>时
        <select class='select' name='endM'>
          <option value='55'>55</option>
          <option value='50'>50</option>
          <option value='45'>45</option>
          <option value='40'>40</option>
          <option value='35'>35</option>
          <option value='30'>30</option>
          <option value='25'>25</option>
          <option value='20'>20</option>
          <option value='15'>15</option>
          <option value='10'>10</option>
          <option value='5'>05</option>
          <option value='0'>00</option>
        </select>分
      </div><br><br><br><br>
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

void handleRoot()
{ //访问主页回调函数
    server.send(200, "text/html", HTML);
}

bool web = false;

void handleRootPost()
{ // Post回调函数
    web = false;
    String my_ssid = server.arg("ssid");
    String my_pswd = server.arg("pswd");
    String my_auth = server.arg("auth");
    if (my_ssid == "")
    {
        server.send(200, "text/html", "<meta charset='utf-8'>请输入WiFi帐号");
        return;
    }
    else
    {
        ssid = my_ssid.c_str();
    }
    if (my_pswd == "")
    {
        server.send(200, "text/html", "<meta charset='utf-8'>请输入WiFi密码");
        return;
    }
    else
    {
        pswd = my_pswd.c_str();
    }
    if (my_auth == "")
    {
        isBlinker = 0;
    }
    else
    {
        auth = my_auth.c_str();
    }
    startH = atoi(server.arg("startH").c_str());
    startM = atoi(server.arg("startM").c_str());
    endH = atoi(server.arg("endH").c_str());
    endM = atoi(server.arg("endM").c_str());
    Serial.println("WiFi账号：" + (String)ssid);
    Serial.println("WiFi密码：" + (String)pswd);
    Serial.println("点灯密钥：" + (String)auth);
    Serial.println("起始时间：" + (String)startH + ":" + (String)startM);
    Serial.println("结束时间：" + (String)endH + ":" + (String)endM);
    server.send(200, "text/html", "<meta charset='utf-8'>上传成功，正在配网"); //返回保存成功页面
    delay(2000);
    server.stop();
    connectNewWiFi();
}

void webServerStart()
{
    Serial.println("正在启动web配网模式");
    if (dnsServer.start(DNS_PORT, "*", apIP))
    { //判断将所有地址映射到esp8266的ip上是否成功
        Serial.println("DNS服务已启动");
    }
    else
    {
        Serial.println("DNS服务启动失败");
    }
    delay(200);
    server.on("/", HTTP_GET, handleRoot);      //设置主页回调函数
    server.onNotFound(handleRoot);             //设置无法响应的http请求的回调函数
    server.on("/", HTTP_POST, handleRootPost); //设置Post请求回调函数
    server.begin();                            //启动WebServer
    Serial.println("Web服务器已启动");
    delay(200);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    if (WiFi.softAP(AP_NAME))
    {
        Serial.println("请连接热点：" + AP_NAME + ",浏览器输入\"192.168.66.1\"配置网络");
        web = true;
    }
    delay(200);
}

int rMillis = 0;
void reConnect()
{
    int r = 0;
    rMillis = millis();
    while (web)
    {
        ledState = 3;
        r = millis() - rMillis;
        // Serial.println(r);
        if (r >= 180000)
        {
            server.stop();
            connectWiFi();
            break;
        }
        else
        {
            server.handleClient();
            dnsServer.processNextRequest();
            if (!web)
            {
                break;
            }
        }
        delay(100);
    }
}

bool blinkerConnect(const char *auth, const char *ssid, const char *pswd)
{
    WiFi.mode(WIFI_STA); // 更换wifi模式      //WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    if (isBlinker)
    {
        Blinker.begin(auth, ssid, pswd);
        int i = 0;
        Serial.println("正在连接点灯科技");
        while (!Blinker.connect())
        {
            delay(1000);
            ledState = 4;
            Serial.println(i++);
            if (i > 20)
            { //如果60秒内没有连上，就开启Web配网 可适当调整这个时间
                Serial.println("点灯科技连接失败");
                ledState = 3;
                break; //跳出 防止无限初始化
            }
        }
        if (Blinker.connect())
        { //如果连接上 就输出IP信息 防止未连接上break后会误输出
            Serial.println("点灯科技连接成功");
            ntp.begin();
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        WiFi.begin(ssid, pswd);
        int i = 0;
        Serial.println("正在连接WiFi");
        while (!WiFi.isConnected())
        {
            delay(1000);
            ledState = 4;
            Serial.println(i++);
            if (i > 20)
            { //如果20秒内没有连上，就开启Web配网 可适当调整这个时间
                Serial.println("WiFi连接失败");
                ledState = 3;
                break; //跳出 防止无限初始化
            }
        }
        if (WiFi.isConnected())
        { //如果连接上 就输出IP信息 防止未连接上break后会误输出
            Serial.println("WiFi连接成功");
            ntp.begin();
            return true;
        }
        else
        {
            return false;
        }
    }
}

void connectWiFi()
{
    web = false;
    if (SPIFFS.begin())
    { // 打开闪存文件系统
        Serial.println("");
        Serial.println("闪存文件系统打开成功");
    }
    else
    {
        Serial.println("");
        Serial.println("闪存文件系统打开失败");
    }
    if (SPIFFS.exists("/WiFi.json")) // exists 判断有没有config.json这个文件
    {
        Serial.println("存在配置信息，正在自动连接");
        const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 156; //分配一个内存空间
        DynamicJsonDocument doc(capacity);                                       // 声明json处理对象
        File configJson = SPIFFS.open("/WiFi.json", "r");
        deserializeJson(doc, configJson); // json数据序列化
        auth = doc["auth"];
        ssid = doc["ssid"];
        pswd = doc["pswd"];
        startH = doc["startH"];
        startM = doc["startM"];
        endH = doc["endH"];
        endM = doc["endM"];
        Serial.println("WiFi账号：" + (String)ssid);
        Serial.println("WiFi密码：" + (String)pswd);
        Serial.println("点灯密钥：" + (String)auth);
        Serial.println("起始时间：" + (String)startH + ":" + (String)startM);
        Serial.println("结束时间：" + (String)endH + ":" + (String)endM);
        configJson.close();
        if ((String)auth == "")
        {
            isBlinker = 0;
        }
        else
        {
            isBlinker = 1;
        }
        if (!blinkerConnect(auth, ssid, pswd))
        {
            Serial.println("连接失败，正在打开web配网模式");
            webServerStart();
        }
    }
    else
    {
        Serial.println("不存在配置信息，正在打开web配网模式");
        webServerStart();
    }
}

void connectNewWiFi(void)
{
    if (!blinkerConnect(auth, ssid, pswd))
    {
        webServerStart();
    }
    else
    {
        removeWiFiConfig();
        String json;
        json += "{\"ssid\":\"";
        json += ssid;
        json += "\",\"pswd\":\"";
        json += pswd;
        json += "\",\"auth\":\"";
        json += auth;
        json += "\",\"startH\":\"";
        json += startH;
        json += "\",\"startM\":\"";
        json += startM;
        json += "\",\"endH\":\"";
        json += endH;
        json += "\",\"endM\":\"";
        json += endM;
        json += "\"}";
        File wifiConfig = SPIFFS.open("/WiFi.json", "w");
        wifiConfig.println(json); //将数据写入config.json文件中
        Serial.println("配置文件写入成功！");
        wifiConfig.close();
    }
}

void removeWiFiConfig()
{ //移除缓存中配置信息文件
    if (SPIFFS.exists("/WiFi.json"))
    { // 判断有没有config.json这个文件
        if (SPIFFS.remove("/WiFi.json"))
        {
            Serial.println("删除旧WiFi配置");
        }
        else
        {
            Serial.println("删除旧WiFi配置失败");
        }
    }
}

void keepOnline()
{
    if (isBlinker)
    {
        if (!Blinker.connect())
        {
            r1Power(1);
            ledState = 3;
        }
        else
        {
            ntp.update();
            if (lowPowerTime())
            {
                r1Power(power);
            }
            else
            {
                if (onlyLowPowerOn)
                {
                    r1Power(0);
                    power = 1;
                }
            }
        }
    }
    else
    {
        if (!WiFi.isConnected())
        {
            r1Power(1);
            ledState = 3;
        }
        else
        {
            ntp.update();
            if (lowPowerTime())
            {
                r1Power(power);
            }
            else
            {
                if (onlyLowPowerOn)
                {
                    r1Power(0);
                    power = 1;
                }
            }
        }
    }
}

void led()
{
    switch (ledState)
    {
    case 0:
        digitalWrite(ledPin, HIGH);
        break;
    case 1:
        digitalWrite(ledPin, LOW);
        break;
    case 2:
        analogWriteRange(1000);
        analogWriteFreq(1000);
        for (int i = 0; i < 1000; i++)
        {
            analogWrite(ledPin, i);
            delay(1);
        }
        for (int i = 1000; i > 0; i--)
        {
            analogWrite(ledPin, i);
            delay(1);
        }
        break;
    case 3:
        digitalWrite(ledPin, !digitalRead(ledPin));
        delay(1000);
        break;
    case 4:
        digitalWrite(ledPin, !digitalRead(ledPin));
        delay(50);
        break;
    }
}
//****************************配网**********************

void miotPowerState(const String &state, uint8_t num) //小爱同学控制指令
{
    BLINKER_LOG("need set outlet: ", num, ", power state: ", state);
    if (state == BLINKER_CMD_ON)
    {
        if (num == 0)
        {
            r1Power(1);
        }
        else if (num == 1)
        {
        }
        else if (num == 2)
        {
        }
        else if (num == 3)
        {
        }
        else if (num == 4)
        {
        }
        BlinkerMIOT.powerState("on", num);
        BlinkerMIOT.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        if (num == 0)
        {
            r1Power(0);
        }
        else if (num == 1)
        {
        }
        else if (num == 2)
        {
        }
        else if (num == 3)
        {
        }
        else if (num == 4)
        {
        }
        BlinkerMIOT.powerState("off", num);
        BlinkerMIOT.print();
    }
}

void aligeniePowerState(const String &state, uint8_t num) //天猫精灵控制指令
{
    BLINKER_LOG("need set outlet: ", num, ", power state: ", state);
    if (state == BLINKER_CMD_ON)
    {
        if (num == 0)
        {
            r1Power(1);
        }
        else if (num == 1)
        {
        }
        else if (num == 2)
        {
        }
        else if (num == 3)
        {
        }
        else if (num == 4)
        {
        }
        BlinkerAliGenie.powerState("on", num);
        BlinkerAliGenie.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        if (num == 0)
        {
            r1Power(0);
        }
        else if (num == 1)
        {
        }
        else if (num == 2)
        {
        }
        else if (num == 3)
        {
        }
        else if (num == 4)
        {
        }
        BlinkerAliGenie.powerState("off", num);
        BlinkerAliGenie.print();
    }
}

bool miotState[2] = {false};

void miotQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("MIOT Query outlet: ", num, ", codes: ", queryCode);

    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
        BLINKER_LOG("MIOT Query All");
        if (r1State())
        {
            miotState[num] = true;
        }
        else
        {
            miotState[num] = false;
        }
        BlinkerMIOT.powerState(miotState[num] ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER:
        BLINKER_LOG("MIOT Query Power State");
        if (r1State())
        {
            miotState[num] = true;
        }
        else
        {
            miotState[num] = false;
        }
        BlinkerMIOT.powerState(miotState[num] ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    default:
        if (r1State())
        {
            miotState[num] = true;
        }
        else
        {
            miotState[num] = false;
        }
        BlinkerMIOT.powerState(miotState[num] ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    }
}

bool aliState[2] = {false};

void aligenieQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("AliGenie Query outlet: ", num, ", codes: ", queryCode);

    BlinkerAliGenie.print();
    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
        BLINKER_LOG("AliGenie Query All");
        if (r1State())
        {
            aliState[num] = true;
        }
        else
        {
            aliState[num] = false;
        }
        BlinkerAliGenie.powerState(aliState[num] ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER:
        BLINKER_LOG("AliGenie Query Power State");
        if (r1State())
        {
            aliState[num] = true;
        }
        else
        {
            aliState[num] = false;
        }
        BlinkerAliGenie.powerState(aliState[num] ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    default:
        if (r1State())
        {
            aliState[num] = true;
        }
        else
        {
            aliState[num] = false;
        }
        BlinkerAliGenie.powerState(aliState[num] ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    }
}

BlinkerButton btnHL("btn-hl");
void btnHL_callback(const String &state) //点灯app内控制按键触发
{
    Blinker.vibrate();
    BLINKER_LOG("get button state: ", state);
    if (state == "tap")
    {
    }
    else if (state == "on")
    {
        btnHL.color("#00FF00"); //设置开关颜色
        btnHL.print("on");
        onlyLowPowerOn = true;
        if (!lowPowerTime())
        {
            r1Power(0);
        }
    }
    else if (state == "off")
    {
        btnHL.color("#808080"); //设置开关颜色
        btnHL.print("off");     // 反馈开关状态
        onlyLowPowerOn = false;
    }
    else if (state == "press")
    {
    }
    else
    {
    }
}

BlinkerButton btnR1("btn-r1");           //定义按钮键名
void btnR1_callback(const String &state) //点灯app内控制按键触发
{
    Blinker.vibrate();
    BLINKER_LOG("get button state: ", state);
    if (onlyLowPowerOn)
    {
        if (lowPowerTime())
        {
            if (state == "tap")
            {
            }
            else if (state == "on")
            {
                btnR1.color("#00FF00"); //设置开关颜色
                btnR1.text("充电桩已就绪");
                btnR1.print("on");
                BUILTIN_SWITCH.print("on");
                r1Power(1);
            }
            else if (state == "off")
            {
                btnR1.color("#808080"); //设置开关颜色
                btnR1.text("点击启动");
                btnR1.print("off"); // 反馈开关状态
                BUILTIN_SWITCH.print("off");
                r1Power(0);
            }
            else if (state == "press")
            {
            }
            else
            {
                // Blinker.print("当前仅谷电时段可用，如需立即使用请关闭上方“仅谷电时段可用”按钮开关");
            }
        }
        else
        {
        }
    }
    else
    {
        if (state == "tap")
        {
        }
        else if (state == "on")
        {
            btnR1.color("#00FF00"); //设置开关颜色
            btnR1.text("充电桩已就绪");
            btnR1.print("on");
            BUILTIN_SWITCH.print("on");
            r1Power(1);
        }
        else if (state == "off")
        {
            btnR1.color("#808080"); //设置开关颜色
            btnR1.text("点击启动");
            btnR1.print("off"); // 反馈开关状态
            BUILTIN_SWITCH.print("off");
            r1Power(0);
        }
        else if (state == "press")
        {
        }
        else
        {
        }
    }
}

void r1Power(int i)
{
    switch (i)
    {
    case 1:
        power = 1;
        digitalWrite(r1, LOW);
        break;
    case 0:
        power = 0;
        digitalWrite(r1, HIGH);
        break;
    }
}

bool r1State()
{
    if (digitalRead(r1))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool lowPowerTime()
{
    bool state = false;
    int nowH, nowM;
    nowH = ntp.getHours();
    nowM = ntp.getMinutes();
    if (startH > endH)
    {
        if (nowH > startH or nowH < endH)
        {
            state = true;
        }
        else if (nowH == startH)
        {
            if (nowM >= startM)
            {
                state = true;
            }
        }
        else if (nowH == endH)
        {
            if (nowM < endM)
            {
                state = true;
            }
        }
    }
    else if (startH < endH)
    {
        if (nowH > startH and nowH < endH)
        {
            state = true;
        }
        else if (nowH == startH)
        {
            if (nowM >= startM)
            {
                state = true;
            }
        }
        else if (nowH == endH)
        {
            if (nowM < endM)
            {
                state = true;
            }
        }
    }
    else if (startH == endH)
    {
        if (nowH == startH)
        {
            if (startM < endM)
            {
                if (nowM >= startM and nowM < endM)
                {
                    state = true;
                }
            }
            else if (startM > endM)
            {
                if (nowM >= startM or nowM < endM)
                {
                    state = true;
                }
            }
            else
            {
                if (nowM == startM)
                {
                    state = true;
                }
            }
        }
    }
    return state;
}

int jMillis = 0;
void key()
{
    int j = 0;
    jMillis = millis();
    while (!digitalRead(keyPin))
    {
        j = millis() - jMillis;
        Serial.println(j);
        if (j >= 3000 and j < 3100)
        {
            onlyLowPowerOn = !onlyLowPowerOn;
            if (!onlyLowPowerOn)
            {
                r1Power(1);
            }
            Serial.println("开关仅谷电可用");
        }
        else if (j >= 10000)
        {
            removeWiFiConfig();
            Serial.println("重启");
            ledState = 1;
            delay(1000);
            ESP.restart();
        }
        delay(100);
    }
    if (j > 50 and j < 3000)
    {
        r1Power(!power);
        Serial.println("启动或关闭");
    }
    jMillis = millis();
}

Ticker ticker;

void flash()
{

    r1State();
    if (WiFi.isConnected())
    {
        if (!onlyLowPowerOn)
        {
            ledState = 3;
        }
        else
        {
            ledState = 0;
        }
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(r1, OUTPUT);
    r1Power(1);

    pinMode(ledPin, OUTPUT);

    pinMode(keyPin, OUTPUT);
    digitalWrite(keyPin, HIGH);

    BlinkerMIOT.attachPowerState(miotPowerState); //小爱
    BlinkerMIOT.attachQuery(miotQuery);           //小爱

    BlinkerAliGenie.attachPowerState(aligeniePowerState); //天猫
    BlinkerAliGenie.attachQuery(aligenieQuery);           //天猫

    Blinker.attachHeartbeat(heartbeat);
    Blinker.attachSummary(summary);
    BUILTIN_SWITCH.attach(switch_callback);

    Blinker.attachData(dataRead);
    BLINKER_DEBUG.stream(Serial);

    ticker.attach(2, flash);

    connectWiFi();

    btnR1.attach(btnR1_callback);
    btnHL.attach(btnHL_callback);

    btnHL.color("#00FF00"); //设置开关颜色
    btnHL.print("on");
    onlyLowPowerOn = true;
}

void loop()
{
    keepOnline();
    reConnect();
    key();
    Blinker.run();
}

void switch_callback(const String &state)
{
    BLINKER_LOG("get switch state: ", state);
    if (state == BLINKER_CMD_ON)
    {
        r1Power(1);
        BUILTIN_SWITCH.print("on");
    }
    else if (state == BLINKER_CMD_OFF)
    {
        r1Power(0);
        BUILTIN_SWITCH.print("off");
    }
}

void heartbeat()
{
    if (r1State())
    {
        btnR1.color("#00FF00"); //设置开关颜色
        btnR1.text("充电桩已就绪");
        btnR1.print("on");
        BUILTIN_SWITCH.print("on");
    }
    else
    {
        btnR1.color("#808080"); //设置开关颜色
        btnR1.text("点击启动");
        btnR1.print("off"); // 反馈开关状态
        BUILTIN_SWITCH.print("off");
    }

    if (onlyLowPowerOn)
    {
        btnHL.color("#00FF00"); //设置开关颜色
        btnHL.print("on");
    }
    else
    {
        btnHL.color("#808080"); //设置开关颜色
        btnHL.print("off");     // 反馈开关状态
    }
}

String summary()
{
    String data = "充电桩: " + STRING_format(r1State() ? "就绪" : "关闭");
    return data;
}

void dataRead(const String &data) // 如果未绑定的组件被触发，则会执行其中内容
{
    BLINKER_LOG("Blinker readString: ", data);
    Blinker.vibrate();
    uint32_t BlinkerTime = millis();
    Blinker.print("millis", BlinkerTime);
}
