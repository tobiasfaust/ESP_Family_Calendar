// Based on Example: https://github.com/PaulStoffregen/Time/blob/master/examples/TimeNTP_ESP8266WiFi/TimeNTP_ESP8266WiFi.ino

#ifndef NTP_H
#define NTP_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <TimeLib.h>
//#include "BaseConfig.h"

//extern BaseConfig* Config;

class NTPClient {

  public:
    NTPClient();
    time_t getNtpTime();

    String getTimeStr(time_t moment);
    String getDateStr(time_t moment);
    String getTimeDateString(time_t moment);
    String getTimeStr();
    String getDateStr();
    String getTimeDateString();

  private:
    String ntpServerName;

    uint8_t timeZone;     // Central European Time

    unsigned int UDPlocalPort = 8888;  // local port to listen for UDP packets
    const int NTP_PACKET_SIZE = 48;   // NTP time is in the first 48 bytes of message
    byte packetBuffer[48];            //buffer to hold incoming & outgoing packets
    
    WiFiUDP Udp;
    void sendNTPpacket(IPAddress &address);
    
};

#endif
