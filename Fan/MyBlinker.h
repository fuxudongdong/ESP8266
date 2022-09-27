#ifndef _MyBlinker_
#define _MyBlinker_

#include "arduino.h"
#include "WiFiHtml.h"
#include <ESP8266WiFi.h>

class MyBlinker
{
public:
    MyBlinker();
    String getPage();
    // void setTitle(String title);

private:
};
#endif