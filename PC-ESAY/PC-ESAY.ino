#define BLINKER_WIFI
#define BLINKER_MIOT_OUTLET //小爱同学
//#define BLINKER_MIOT_MULTI_OUTLET  //设置为小爱多个插座的模式
#define BLINKER_ALIGENIE_OUTLET //天猫精灵
//#define BLINKER_ALIGENIE_MULTI_OUTLET  //设置为天猫精灵多个插座的模式

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <FS.h> //闪存文件系统
#include <Blinker.h>
#include <Ticker.h>
#include <MyBlinker.h>
#include <Flash.h>

MyBlinker mb;
String AP_NAME = "开机板_" + (String)ESP.getChipId(); // wifi名字
const byte DNS_PORT = 53;                             // DNS端口号
IPAddress apIP(192, 168, 66, 1);                      // esp8266-AP-IP地址
ESP8266WebServer server(80);                          //创建WebServer
DNSServer dnsServer;
Flash fl("WiFiConfig.json");

// char auth[] = "c69e47a87363"; //2400G
// 2b5ecff35a89 测试
String auth;
String ssid;
String pswd;

int PP = 0;     // io0 0
int PS = 1;     // txd 1
int ledPin = 2; // io2 2

//***************配网*********************

void handleRoot()
{ //访问主页回调函数
    server.send(200, "text/html", mb.getPage());
}

bool web = false;
void handleRootPost()
{ // Post回调函数
    web = false;
    if (server.hasArg("ssid"))
    {                              //判断是否有账号参数
        ssid = server.arg("ssid"); //将账号参数拷贝到my_ssid中
        Serial.println("WiFi账号：" + ssid);
    }
    else
    { //没有参数
        Serial.println("错误，未找到WiFi账号");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi账号"); //返回错误页面
        return;
    }
    if (server.hasArg("password"))
    {
        pswd = server.arg("password");
        Serial.println("WiFi密码：" + pswd);
    }
    else
    {
        Serial.println("错误，未找到WiFi密码");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi密码");
        return;
    }
    if (server.hasArg("authkey"))
    {
        auth = server.arg("authkey");
        Serial.println("点灯密钥：" + auth);
    }
    else
    {
        Serial.println("错误，未找到点灯密钥");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到点灯密钥");
        return;
    }
    server.send(200, "text/html", "<meta charset='utf-8'>上传成功，正在配网"); //返回保存成功页面
    delay(200);
    server.stop();
    connectNewWiFi();
}

void webServerStart()
{
    Serial.println("正在启动web配网模式");
    mb.setTitle("远程开机配网");
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
        led(1);
        r = millis() - rMillis;
        Serial.println(r);
        if (r >= 180000)
        {
            ESP.restart();
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

bool blinkerConnect()
{
    WiFi.mode(WIFI_STA); // 更换wifi模式      //WiFi.mode(WIFI_AP_STA);//设置模式为AP+STA
    Blinker.begin(auth.c_str(), ssid.c_str(), pswd.c_str());
    int i = 0;
    Serial.println("正在连接点灯科技");
    while (!Blinker.connect())
    {
        for (int j = 0; j < 10; j++)
        {
            delay(100);
            led(3);
        }
        Serial.println(i++);
        if (i > 20)
        { //如果60秒内没有连上，就开启Web配网 可适当调整这个时间
            Serial.println("点灯科技连接失败");
            led(1);
            break; //跳出 防止无限初始化
        }
    }
    if (Blinker.connect())
    { //如果连接上 就输出IP信息 防止未连接上break后会误输出
        Serial.println("点灯科技连接成功");
        led(0);
        return true;
    }
    else
    {
        return false;
    }
}

void connectWiFi()
{
    web = false;
    auth = fl.getContent("auth");
    ssid = fl.getContent("ssid");
    pswd = fl.getContent("pswd");
    if (ssid == "")
    {
        Serial.println("不存在配置信息，正在打开web配网模式");
        webServerStart();
    }
    else
    {
        if (!blinkerConnect())
        {
            webServerStart();
        }
    }
}

void connectNewWiFi()
{
    if (!blinkerConnect())
    {
        webServerStart();
    }
    else
    {
        fl.add("auth", auth);
        fl.add("ssid", ssid);
        fl.add("pswd", pswd);
        fl.save();
    }
}

void keepOnline()
{
    if (!Blinker.connect())
    {
        led(1);
    }
    else
    {
        // led(0);
    }
}

void led(int state)
{
    switch (state)
    {
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

void dataRead(const String &data) // 如果未绑定的组件被触发，则会执行其中内容
{
    BLINKER_LOG("Blinker readString: ", data);
    Blinker.vibrate();
    uint32_t BlinkerTime = millis();
    Blinker.print("millis", BlinkerTime);
}

void miotPowerState(const String &state, uint8_t num) //小爱同学控制指令
{
    BLINKER_LOG("need set outlet: ", num, ", power state: ", state);
    if (state == BLINKER_CMD_ON)
    {
        if (num == 0)
        {
            PCPower(1);
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
            PCPower(0);
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
            PCPower(1);
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
            PCPower(0);
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

bool miotState[5] = {false};

void miotQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("MIOT Query outlet: ", num, ", codes: ", queryCode);
    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
        BLINKER_LOG("MIOT Query All");
        BlinkerMIOT.powerState(pcState() ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER:
        BLINKER_LOG("MIOT Query Power State");
        BlinkerMIOT.powerState(pcState() ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    default:
        BlinkerMIOT.powerState(pcState() ? "on" : "off", num);
        BlinkerMIOT.print();
        break;
    }
}

bool aliState[5] = {false};

void aligenieQuery(int32_t queryCode, uint8_t num)
{
    BLINKER_LOG("AliGenie Query outlet: ", num, ", codes: ", queryCode);
    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_ALL_NUMBER:
        BLINKER_LOG("AliGenie Query All");
        BlinkerAliGenie.powerState(pcState() ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER:
        BLINKER_LOG("AliGenie Query Power State");
        BlinkerAliGenie.powerState(pcState() ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    default:
        BlinkerAliGenie.powerState(pcState() ? "on" : "off", num);
        BlinkerAliGenie.print();
        break;
    }
}

BlinkerButton btnPower("btn-pwr"); //定义按钮键名

void btnPower_callback(const String &state) //点灯app内控制按键触发
{
    BLINKER_LOG("get button state: ", state);
    Blinker.vibrate();
    if (state == "tap")
    {
        Blinker.print("tap模式只能打开电脑不能关电脑！");
        PCPower(1);
    }
    else if (state == "on")
    {
        PCPower(1);
    }
    else if (state == "off")
    {
        PCPower(0);
    }
    else if (state == "press")
    {
        PCPower(2);
    }
    else if (state == "pressup")
    {
    }
}

void PCPower(int state)
{
    switch (state)
    {
    case 0:
        if (pcState())
        {
            Blinker.print("正在关闭电脑...");
            keypp(1000);
        }
        else
        {
            Blinker.print("电脑已关闭");
        }
        break;
    case 1:
        if (pcState())
        {
            Blinker.print("电脑已打开");
        }
        else
        {
            Blinker.print("正在打开电脑...");
            keypp(1000);
            // wakeMyPC();
        }
        break;
    case 2:
        Blinker.print("正在强制关闭电脑...");
        btnPower.color("#FF0000"); //设置开关颜色
        btnPower.print();
        keypp(6000);
        btnPower.color("#808080"); //设置开关颜色
        btnPower.print();
        Blinker.print("完成");
        break;
    }
}

bool pcState()
{
    bool state = false;
    if (digitalRead(PS))
    {
        state = false;
    }
    else
    {
        state = true;
    }
    return state;
}

void keypp(int t)
{
    led(1);
    digitalWrite(PP, LOW);
    delay(t);
    digitalWrite(PP, HIGH);
    led(0);
}

void switch_callback(const String &state)
{
    BLINKER_LOG("get switch state: ", state);
    if (state == BLINKER_CMD_ON)
    {
        PCPower(1);
    }
    else if (state == BLINKER_CMD_OFF)
    {
        PCPower(0);
    }
}

void heartbeat()
{
    if (pcState())
    {
        btnPower.color("#FF0000"); //设置开关颜色
        btnPower.text("关");
        btnPower.print("on");
        BUILTIN_SWITCH.print("on");
    }
    else
    {
        btnPower.color("#808080"); //设置开关颜色
        btnPower.text("开");
        btnPower.print("off"); // 反馈开关状态
        BUILTIN_SWITCH.print("off");
    }
}

Ticker ticker;

void flash()
{
    pcState();
}

void setup()
{
    Serial.begin(115200);
    WiFi.hostname("Smart-ESP8266"); //设置ESP8266设备名

    pinMode(ledPin, OUTPUT);
    led(1);

    Blinker.attachData(dataRead);
    BLINKER_DEBUG.stream(Serial);

    BlinkerMIOT.attachPowerState(miotPowerState); //小爱
    BlinkerMIOT.attachQuery(miotQuery);           //小爱

    BlinkerAliGenie.attachPowerState(aligeniePowerState); //天猫
    BlinkerAliGenie.attachQuery(aligenieQuery);           //天猫

    Blinker.attachHeartbeat(heartbeat);
    BUILTIN_SWITCH.attach(switch_callback);

    pinMode(PS, OUTPUT);
    digitalWrite(PS, HIGH);

    pinMode(PP, OUTPUT);
    digitalWrite(PP, HIGH);

    btnPower.attach(btnPower_callback);

    ticker.attach(3, flash);

    connectWiFi();
}

void loop()
{
    keepOnline();
    reConnect();
    Blinker.run();
}
