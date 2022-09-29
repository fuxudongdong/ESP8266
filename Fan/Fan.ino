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
#include <Flash.h>
#include <LED.h>
#include "WiFiHtml.h"
#include "MyBlinker.h"

// 0fbae1d3bba4  壁扇
// ad2634fb78ab  家里风扇
// bdcdc3404089  操作间壁扇
// 88d4ebf71de6  循环扇

//***************配网*********************

MyBlinker mb;

String AP_NAME = "风扇_" + (String)ESP.getChipId(); // wifi名字
const byte DNS_PORT = 53;                           // DNS端口号
IPAddress apIP(192, 168, 66, 1);                    // esp8266-AP-IP地址
ESP8266WebServer server(80);                        //创建WebServer
DNSServer dnsServer;
Flash fl("WiFiConfig.json");
LED led(15);

String auth;
String ssid;
String pswd;

void handleRoot()
{ //访问主页回调函数
    server.send(200, "text/html", mb.getPage());
}

bool web = false;
void handleRootPost()
{ // Post回调函数
    web = false;
    if (server.hasArg("ssid"))
    {                                      //判断是否有账号参数
        ssid = server.arg("ssid").c_str(); //将账号参数拷贝到my_ssid中
        Serial.println("WiFi账号：" + (String)ssid);
    }
    else
    { //没有参数
        Serial.println("错误，未找到WiFi账号");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi账号"); //返回错误页面
        return;
    }
    if (server.hasArg("password"))
    {
        pswd = server.arg("password").c_str();
        Serial.println("WiFi密码：" + (String)pswd);
    }
    else
    {
        Serial.println("错误，未找到WiFi密码");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到WiFi密码");
        return;
    }
    if (server.hasArg("authkey"))
    {
        auth = server.arg("authkey").c_str();
        Serial.println("点灯密钥：" + (String)auth);
    }
    else
    {
        Serial.println("错误，未找到点灯密钥");
        server.send(200, "text/html", "<meta charset='UTF-8'>错误，未找到点灯密钥");
        return;
    }
    server.send(200, "text/html", "<meta charset='utf-8'>上传成功，正在配网"); //返回保存成功页面
    delay(2000);
    server.stop();
    connectNewWiFi();
}

void webServerStart()
{
    Serial.println("正在启动web配网模式");
    mb.setTitle("风扇配网");
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
        led.mode(1);
        r = millis() - rMillis;
        // Serial.println(r);
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
            led.mode(3);
        }
        Serial.println(i++);
        if (i > 20)
        { //如果60秒内没有连上，就开启Web配网 可适当调整这个时间
            Serial.println("点灯科技连接失败");
            led.mode(1);
            break; //跳出 防止无限初始化
        }
    }
    if (Blinker.connect())
    { //如果连接上 就输出IP信息 防止未连接上break后会误输出
        Serial.println("点灯科技连接成功");
        led.mode(0);
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
        led.mode(1);
    }
    else
    {
        // led.mode(0);
    }
}
//****************************配网**********************

int L1 = 3; // RXD  1档
int L2 = 1; // IO0   摆风
int L3 = 2; // IO2   3档
int L4 = 0; // TXD  2档

int fanLevel = 0;
int hs = 0;

void miotLevel(uint8_t level)
{
    BLINKER_LOG("need set level:", level);
    setFan(level);
    fanLevel = level;
    BlinkerMIOT.level(level);
    BlinkerMIOT.print();
}

void miotHSwingState(const String &state)
{ // hs左右摆风
    BLINKER_LOG("need set HSwing state:", state);
    if (state == BLINKER_CMD_ON)
    {
        setFan(4); //开启左右摆风
        BlinkerMIOT.hswing("on");
        BlinkerMIOT.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        setFan(5); //关闭左右摆风
        BlinkerMIOT.hswing("off");
        BlinkerMIOT.print();
    }
}

void miotVSwingState(const String &state)
{ // vs上下摆风
    BLINKER_LOG("need set VSwing state:", state);
    if (state == BLINKER_CMD_ON)
    {
        // setFan(4);
        BlinkerMIOT.vswing("on");
        BlinkerMIOT.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        // setFan(5);
        BlinkerMIOT.vswing("off");
        BlinkerMIOT.print();
    }
}

void miotPowerState(const String &state) //小爱同学控制指令
{
    BLINKER_LOG("need power state: ", state);
    if (state == "off")
    {
        setFan(0); //关风扇
        BlinkerMIOT.powerState("off");
    }
    else
    {
        setFan(3); //风扇3档
        BlinkerMIOT.level(fanLevel);
    }
    BlinkerMIOT.print();
}

void miotQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes:", queryCode);
    switch (queryCode)
    {
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

void aligenieLevel(uint8_t level)
{
    BLINKER_LOG("need set level:", level);
    setFan(level);
    BlinkerAliGenie.level(level);
    BlinkerAliGenie.print();
}

void aligenieHSwingState(const String &state)
{ // hs左右摆风
    BLINKER_LOG("need set HSwing state:", state);
    if (state == BLINKER_CMD_ON)
    {
        setFan(4); //开启左右摆风
        BlinkerAliGenie.hswing("on");
        BlinkerAliGenie.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        setFan(5); //关闭左右摆风
        BlinkerAliGenie.hswing("off");
        BlinkerAliGenie.print();
    }
}

void aligenieMode(const String &mode)
{
    BLINKER_LOG("need set mode: ", mode);
    //
    BlinkerAliGenie.mode(mode);
    BlinkerAliGenie.print();
}

void aligenieVSwingState(const String &state)
{ // vs上下摆风
    BLINKER_LOG("need set VSwing state:", state);
    if (state == BLINKER_CMD_ON)
    {
        // setFan(4);
        BlinkerAliGenie.vswing("on");
        BlinkerAliGenie.print();
    }
    else if (state == BLINKER_CMD_OFF)
    {
        // setFan(5);
        BlinkerAliGenie.vswing("off");
        BlinkerAliGenie.print();
    }
}

void aligeniePowerState(const String &state) //天猫精灵
{
    BLINKER_LOG("need power state: ", state);
    if (state == "off")
    {
        setFan(0); //关风扇
        BlinkerAliGenie.powerState("off");
    }
    else
    {
        setFan(3); //风扇3档
        BlinkerAliGenie.level(fanLevel);
    }
    BlinkerAliGenie.print();
}

void aligenieQuery(int32_t queryCode)
{
    BLINKER_LOG("MIOT Query codes:", queryCode);
    switch (queryCode)
    {
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
    BlinkerDuerOS.level(level);
    BlinkerDuerOS.print();
}

void duerRelativeLevel(int32_t level)
{
    BLINKER_LOG("need set relative level: ", level);
    // 0:AUTO MODE, 1-3 LEVEL
    setFan(level + 1);
    BlinkerDuerOS.level(level + 1);
    BlinkerDuerOS.print();
}

void duerMode(const String &mode)
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

void duerPowerState(const String &state)
{
    BLINKER_LOG("need set power state: ", state);

    if (state == BLINKER_CMD_ON)

    {
        setFan(3);
        BlinkerDuerOS.level(3);
        BlinkerDuerOS.print();
    }
    else

    {
        setFan(0);
        BlinkerDuerOS.powerState("off");
        BlinkerDuerOS.print();
    }
}

void duerQuery(int32_t queryCode)
{
    BLINKER_LOG("DuerOS Query codes: ", queryCode);

    switch (queryCode)
    {
    case BLINKER_CMD_QUERY_POWERSTATE_NUMBER:
        BLINKER_LOG("DuerOS Query power state");
        BlinkerDuerOS.powerState(fanLevel ? "on" : "off");
        BlinkerDuerOS.level(fanLevel);
        BlinkerDuerOS.print();
        break;
    case BLINKER_CMD_QUERY_TIME_NUMBER:
        BLINKER_LOG("DuerOS Query time");
        BlinkerDuerOS.time(millis());
        BlinkerDuerOS.print();
        break;
    default:
        BlinkerDuerOS.powerState(fanLevel ? "on" : "off");
        BlinkerDuerOS.level(fanLevel);
        BlinkerDuerOS.print();
        break;
    }
}

void dataRead(const String &data) // 如果未绑定的组件被触发，则会执行其中内容
{
    BLINKER_LOG("Blinker readString: ", data);
    Blinker.vibrate();
    uint32_t BlinkerTime = millis();
    Blinker.print("millis", BlinkerTime);
}

BlinkerButton btnHand("btn-hand"); //定义按钮键名
void btnHand_callback(const String &state)
{
    BLINKER_LOG("get button state: ", state);
    if (state == "tap")
    {
        if (!digitalRead(L4))
        {
            btnHand.color("#808080");
            btnHand.print("off");
            setFan(5);
        }
        else
        {
            btnHand.color("#00ff00");
            btnHand.print("on");
            setFan(4);
        }
    }
    else if (state == "on" or state == "press")
    {
        btnHand.color("#00ff00");
        btnHand.print("on");
        setFan(4);
    }
    else if (state == "off")
    {
        btnHand.color("#808080");
        btnHand.print("off");
        setFan(5);
    }
}

BlinkerSlider sliderSpeed("ran-speed"); //定义滑块
void sliderSpeed_callback(int32_t value)
{
    BLINKER_LOG("get slider value: ", value);
    switch (value)

    {
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
void setFan(int i)
{
    while (millis() - fanMillis < 1000)
    {
        delay(500);
    }
    fanMillis = millis();
    switch (i)
    {
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

void heartbeat()
{
    switch (fanLevel)
    {
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
    if (hs)

    {
        btnHand.color("#00ff00");
        btnHand.print("on");
    }

    else

    {
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
    //  BlinkerMIOT.attachVSwing(miotVSwingState);

    BlinkerAliGenie.attachPowerState(aligeniePowerState); //天猫
    BlinkerAliGenie.attachQuery(aligenieQuery);           //天猫
    BlinkerAliGenie.attachLevel(aligenieLevel);
    BlinkerAliGenie.attachHSwing(aligenieHSwingState);
    BlinkerAliGenie.attachMode(aligenieMode);
    //  BlinkerMIOT.attachVSwing(aligenieVSwingState);

    BlinkerDuerOS.attachPowerState(duerPowerState);
    BlinkerDuerOS.attachLevel(duerLevel);
    BlinkerDuerOS.attachRelativeLevel(duerRelativeLevel);
    BlinkerDuerOS.attachMode(duerMode);
    BlinkerDuerOS.attachQuery(duerQuery);

    BUILTIN_SWITCH.attach(switch_callback);
    Blinker.attachHeartbeat(heartbeat);
    Blinker.attachSummary(summary);

    //  BLINKER_DEBUG.stream(Serial);

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

void switch_callback(const String &state)
{
    BLINKER_LOG("get switch state: ", state);
    if (state == BLINKER_CMD_ON)

    {
        setFan(3);
        BUILTIN_SWITCH.print("on");
    }
    else if (state == BLINKER_CMD_OFF)

    {
        setFan(0);
        BUILTIN_SWITCH.print("off");
    }
}

String summary()
{
    String data;
    if (fanLevel != 0)
    {
        if (hs)
        {
            data = "电扇: " + (String)fanLevel + "档;左右摆风：开";
        }
        else
        {
            data = "电扇: " + (String)fanLevel + "档;左右摆风：关";
        }
    }
    else
    {
        data = "电扇: 关";
    }
    return data;
}
