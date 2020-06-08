#ifndef WEATHER_H
#define WEATHER_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#if defined(ESP8266)
  #include <ESP8266WebServer.h>
  #define ESPWebServer ESP8266WebServer
#else
  #include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )
  #define ESPWebServer WebServer
#endif

#include <vector>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "BaseConfig.h"

extern BaseConfig* Config;

class Weather {
  enum owm_type_t {UNKNOWN, CURRENT, FORECAST};

  typedef struct {
    time_t dt;
    byte dayNumber; // 0=today, 1=tomorrow
    String icon;
    float temperature;
  } weather_t;
  
  public:
    Weather();

    String  GetIcon(uint8_t dayNumber);
 
  private:
  WiFiClient client;

  std::vector<weather_t>* condition = NULL;

  void      getData();
  void      httpRequest(owm_type_t type);
  void      parseCurrentData(String* jsonData);
  void      parseForecastData(String* jsonData);
  
};

#endif

