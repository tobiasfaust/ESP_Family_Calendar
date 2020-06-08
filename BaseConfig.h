#ifndef BASECONFIG_H
#define BASECONFIG_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif



#if defined(ESP8266)
  #include <FS.h> 
  #include <ESP8266WebServer.h>
  #define ESPWebServer ESP8266WebServer
#else
  #include <SPIFFS.h>
  #include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )
  #define ESPWebServer WebServer
#endif

#include "ArduinoJson.h"

class BaseConfig {

  public:
    BaseConfig();
    void      StoreJsonConfig(String* json); 
    void      LoadJsonConfig();
    void      GetWebContent(ESPWebServer* server);
    const uint8_t& GetPinSDA()      const {return pin_sda;}
    const uint8_t& GetPinSCL()      const {return pin_scl;}
    const uint8_t& GetI2cAddr()     const {return i2c_addr;}
    const String&  GetOWApiKey()    const {return ow_apikey;}
    const String&  GetOWCountry()    const {return ow_country;}
    const String&  GetOWCity()    const {return ow_city;}
    
    //const String&  GetMqttServer()  const {return mqtt_server;}
    //const uint16_t& GetMqttPort()    const {return mqtt_port;}
    //const String&  GetMqttRoot()    const {return mqtt_root;}
    
  private:
    //String    mqtt_server;
    //uint16_t  mqtt_port;
    //String    mqtt_root;
    uint8_t   pin_sda;
    uint8_t   pin_scl;
    uint8_t   i2c_addr;

    String    ow_apikey;
    String    ow_country;
    String    ow_city;
   
};

#endif

