#include "Weather.h"

// current Weather: http://api.openweathermap.org/data/2.5/weather?q=magdeburg&mode=json&units=metric&APPID=f3c45b5a19426de9ea6ba7eb6c6969d7
// NextDay Weather: http://api.openweathermap.org/data/2.5/forecast/daily?q=magdeburg&mode=json&units=metric&cnt=1&APPID=f3c45b5a19426de9ea6ba7eb6c6969d7
// Next3Hour Weather: http://api.openweathermap.org/data/2.5/forecast?q=magdeburg&mode=json&lang=de&units=metric&cnt=2&APPID=7e3e7d9fa9f748cf5173069533e5bb90

// https://randomnerdtutorials.com/esp8266-weather-forecaster/
// https://github.com/G6EJD/ESP32-42e-Paper-Weather-Display-/blob/master/ESP32_OWM_Current_Forecast_75on42_epaper_v2.ino

Weather::Weather() {
  this->condition = new std::vector<weather_t>{};
  this->getData();
}

void Weather::getData() {
  this->condition->clear();
  
  this->httpRequest(CURRENT);
  this->httpRequest(FORECAST);
}

void Weather::httpRequest(owm_type_t type) {
  // close any connection before send a new request to allow client make connection to server
  client.stop();

  String path = ""; uint8_t cnt = 0;
  if (type == CURRENT) { path = "weather"; }
  if (type == FORECAST){ path = "forecast"; cnt = 2;}
 
  // if there's a successful connection:
  if (client.connect("api.openweathermap.org", 80)) {
    // Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("GET /data/2.5/" + path + "?q=" + Config->GetOWCity() + "&APPID=" + Config->GetOWApiKey() + "&mode=json&units=metric&cnt=" + cnt + " HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    String json = ""; 
    bool jsonBegin=false;
    while (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.startsWith("{")) { jsonBegin=true; }
      if(jsonBegin) {json.concat(line); }
    }
    if (type == CURRENT) { this->parseCurrentData(&json); }
    if (type == FORECAST){ this->parseForecastData(&json); }
  }
  else {
    Serial.println("connection failed");
    return;
  }
}

void Weather::parseCurrentData(String* jsonData) {
  Serial.println(*jsonData);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(jsonData->c_str());

  if (json.success()) {
    weather_t cond;
    /*if (json.containsKey("main")) {
      JsonObject& main = json["main"];
      if (main.containsKey("temp"))     { this->currentTemp = main["temp"].as<float>(); }
    }*/
    cond.dayNumber    = 0;
    cond.dt           = json["dt"];
    cond.temperature  = json["main"]["temp"].as<float>();
    cond.icon         = json["weather"][0]["icon"].as<String>();
    this->condition->push_back(cond);
  }
}

void Weather::parseForecastData(String* jsonData) {
  Serial.println(*jsonData);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(jsonData->c_str());
  int cnt;
  
  if (json.success()) {
    JsonArray& list = json["list"];
    if (json.containsKey("cnt")) {cnt = json["cnt"];} else {cnt=0;} 

    for (byte i=0; i < cnt; i++) {
      weather_t cond;
      cond.dayNumber    = i + 0;
      cond.dt           = list[i]["dt"];
      cond.temperature  = list[i]["main"]["temp"].as<float>();
      cond.icon         = list[i]["weather"][0]["icon"].as<String>();
      this->condition->push_back(cond);
    }
  }
}

String Weather::GetIcon(uint8_t dayNumber) {
  for (int i=0; i< this->condition->size(); i++ ) {
    if (this->condition->at(i).dayNumber == dayNumber) {return this->condition->at(i).icon; }
  }

  return "";
}

