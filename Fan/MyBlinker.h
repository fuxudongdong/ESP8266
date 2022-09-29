#ifndef _MyBlinker_
#define _MyBlinker_

#define BLINKER_WIFI

#include "arduino.h"
#include "WiFiHtml.h"
#include <ESP8266WiFi.h>
#include <Flash.h>

class MyBlinker
{
public:
    MyBlinker();
    String getPage();
    void setTitle(String title);

private:
    String htmlName = "配网";
};
#endif