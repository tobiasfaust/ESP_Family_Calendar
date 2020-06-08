// https://www.instructables.com/id/ESP8266-and-ESP32-With-WiFiManager/
// https://learn.adafruit.com/epaper-weather-station/arduino-setup
// https://github.com/doctormord/ESP8266_EPD_Weather_Google_Calendar/blob/master/ESP8266_EPD_Weather_Google_Calendar.ino
// https://www.360customs.de/en/2018/07/4-2-epd-e-ink-display-esp8266-wetter-google-kalender/
// https://github.com/G6EJD/ESP32-e-Paper-Weather-Display/blob/master/examples/Waveshare_7_5_T7/Waveshare_7_5_T7.ino
// https://github.com/ZinggJM/GxEPD/blob/master/extras/Particle/examples/GxEPD_ParticlePartialUpdateExample/src/GxEPD_ParticlePartialUpdateExample.ino

#if defined(ESP8266)
  #include <ESP8266WiFi.h>
#else if defined(ESP32)
  #include <WiFi.h>
#endif

#include <WiFiManager.h>          https://github.com/zhouhan0126/WIFIMANAGER-ESP32
#include <WiFiClientSecure.h>
#include <TimeLib.h>              //TimeLib library is needed https://github.com/PaulStoffregen/Time

#include "BaseConfig.h"
#include "calendar.h";
#include "MyWebServer.h"
#include "NTP.h"
#include "Weather.h"
#include "Display.h"

//https://stackoverflow.com/questions/25402496/api-or-library-to-process-vcalendar-information/25405255#25405255
// stack overflow issue: https://github.com/esp8266/Arduino/commit/d83eabe5b3078b95bddd50f3840234b116aa3063

//const char fingerprint[] PROGMEM = "07:76:0E:05:7F:AF:34:D2:D4:73:5B:76:B7:01:BA:FC:C1:CA:FA:B1";

BaseConfig* Config = NULL;
calendar* iCal = NULL;
MyWebServer* mywebserver = NULL;
NTPClient* NTP = NULL;
Weather* owW = NULL;
Display* d = NULL;

time_t getNtpTime() { return (NTP->getNtpTime()); } 
  
void setup() {
  Serial.begin(115200);
  Serial.println("Warte auf Verbindung");

  WiFiManager wifiManager;
  wifiManager.setTimeout(300);
  if (!wifiManager.autoConnect("ESPClient")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }
  Serial.print("WiFi connected with local IP: ");
  Serial.println(WiFi.localIP());

  NTP = new NTPClient();
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
  Config = new BaseConfig();
  mywebserver = new MyWebServer(); 
  
  iCal = new calendar();
  owW = new Weather(); //"7e3e7d9fa9f748cf5173069533e5bb90", "de", "Magdeburg");
  
  //iCal->addCalendar("https://p24-caldav.icloud.com/published/2/MTE4OTIzMzk0MTExMTg5Mu93ZvvdIfC5WOKMgI_lV7nN-zQq8L4DnIUAPraG3NNnTVrVrYH-GyOVEA0sr0NjDX-kmesy5dDaPA15ZaHQoNo", true);
  //iCal->addCalendar("https://calendar.google.com/calendar/ical/tobiasfaust76%40gmail.com/private-fdd563a99fff296f857ab2cc6d71087f/basic.ics", true);
  //Serial.println("Update Weather");
  //owm = new openweathermap("7e3e7d9fa9f748cf5173069533e5bb90", ""); //ESP8266-Calendar

  d = new Display();
}

void loop() {
  //char buffer[200] = {0};
  //sprintf(buffer, "%d.%d.%d - %d:%d:%d", day(), month(), year(), hour(), minute(), second());
  //Serial.println(buffer);
  //delay (1000);

  //owm->loop();
  mywebserver->loop();
}
