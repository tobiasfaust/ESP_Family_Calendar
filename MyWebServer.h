// https://github.com/esp8266/Arduino/issues/3205
// https://github.com/Hieromon/PageBuilder
// https://www.mediaevent.de/tutorial/sonderzeichen.html

// WebUpdater ESP32 -> https://github.com/zhouhan0126/WebServer-esp32/blob/master/examples/WebUpdate/WebUpdate.ino
//                     https://github.com/espressif/arduino-esp32/blob/master/libraries/WebServer/examples/WebUpdate/WebUpdate.ino
#ifndef MYWEBSERVER_H
#define MYWEBSERVER_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include <FS.h> 
#include <ArduinoJson.h>

#if defined(ESP8266)
  #include <ESP8266WebServer.h>  //Local WebServer used to serve the configuration portal
  #include <ESP8266HTTPUpdateServer.h>
  #include <ESP8266mDNS.h>
  #define ESPWebServer ESP8266WebServer
#else
  #include <WebServer.h> //Local DNS Server used for redirecting all requests to the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )
  #include <ESPmDNS.h>
  #include <Update.h>
  #define ESPWebServer WebServer
#endif

#include <DNSServer.h> //Local WebServer used to serve the configuration portal (  https://github.com/zhouhan0126/DNSServer---esp32  )

#include "BaseConfig.h"
#include "calendar.h"
#include "JavaScript.h"
#include "CSS.h"

extern BaseConfig* Config;
extern calendar* iCal;

class MyWebServer {

  enum page_t {ROOT, BASECONFIG, CALENDAR};

  public:
    MyWebServer();

    void      loop();

  private:
    
    bool      DoReboot;
    MDNSResponder mdns;
    ESPWebServer* server;
    
    #if defined(ESP8266)
      ESP8266HTTPUpdateServer httpUpdater;
    #endif
    
    void      handleNotFound();
    void      handleReboot();
    void      handleCSS();
    void      handleJS();
    //void      handleJSParam();
    void      handleRoot();
    
    void      handleBaseConfig();
    void      handleCalendarConfig();
    
    void      ReceiveJSONConfiguration(page_t page);
    void      getPageHeader(page_t pageactive);
    void      getPageFooter();
    void      getPage_Status();
  
};

#endif
