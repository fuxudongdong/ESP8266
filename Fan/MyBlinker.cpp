#include "MyBlinker.h"
#include "WiFiHtml.h"
#include <ESP8266WiFi.h>
#include "arduino.h"

MyBlinker::MyBlinker() {}

String MyBlinker::getPage()
{
    String page = "";
    Serial.println(F("开始扫描WiFi"));
    String ssid;
    int32_t rssi;
    uint8_t encryptionType;
    uint8_t *bssid;
    int32_t channel;
    bool hidden;
    int scanResult;
    String wifiList = "";
    scanResult = WiFi.scanNetworks(/*async=*/false, /*hidden=*/false);
    if (scanResult == 0)
    {
        Serial.println(F("没有扫描到WiFi信息"));
        wifiList += "<tr><td colspan='2' style='text-align:conter'>没有扫描到WiFi信息</td></tr>";
    }
    else if (scanResult > 0)
    {
        Serial.printf(PSTR("%d 扫描到WiFi:\n"), scanResult);
        bool isHave = 0;
        int count = 0;
        String *ssids = new String[20];
        for (int i = 0; i < scanResult; i++)
        {
            WiFi.getNetworkInfo(i, ssid, encryptionType, rssi, bssid, channel, hidden);
            for (int j = 0; j < count; j++)
            {
                if (ssid == ssids[j])
                {
                    isHave = 1;
                    break;
                }
            }
            if (!isHave)
            {
                Serial.println((String)(count + 1) + ": " + ssid + " " + rssi + "dB");
                ssids[count] = ssid.c_str();
                wifiList += "<tr><td><div onclick=\"c('" + ssid + "')\" class='ssidList'>" + ssid + "</div></td><td style='text-align:right;width:35px'><div onclick=\"c('" + ssid + "')\" class='ssidList' style='text-align:right'>选择</div></td></tr>";
                if (count == 20)
                {
                    break;
                }
                count = count + 1;
            }
            isHave = 0;
        }
    }
    else
    {
        Serial.printf(PSTR("WiFi scan error %d"), scanResult);
        wifiList += "<tr><td colspan='2' style='text-align:conter'>WiFi扫描出错！</td></tr>";
    }
    page += FPSTR(title);
    page += wifiList;
    page += FPSTR(input);
    // Serial.println(page);
    return page;
}