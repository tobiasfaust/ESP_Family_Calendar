#include "BaseConfig.h"

BaseConfig::BaseConfig() {
  SPIFFS.begin();
  this->LoadJsonConfig();
}

void BaseConfig::StoreJsonConfig(String* json) {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(*json);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/BaseConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open BaseConfig.json file for writing");
    } else {  
      root.printTo(Serial);
      root.printTo(configFile);
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void BaseConfig::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  if (SPIFFS.exists("/BaseConfig.json")) {
    //file exists, reading and loading
    Serial.println("reading sensor config file");
    File configFile = SPIFFS.open("/BaseConfig.json", "r");
    if (configFile) {
      Serial.println("opened config file");
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);

      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      json.printTo(Serial);
      if (json.success()) {
        Serial.println("\nparsed json");
        if (json.containsKey("ow_apikey"))        { this->ow_apikey  = json["ow_apikey"].as<String>();}
        if (json.containsKey("ow_country"))       { this->ow_country = json["ow_country"].as<String>();}
        if (json.containsKey("ow_city"))          { this->ow_city    = json["ow_city"].as<String>();}
        if (json.containsKey("pinsda"))           { this->pin_sda  = atoi(json["pinsda"]);}
        if (json.containsKey("pinscl"))           { this->pin_scl  = atoi(json["pinscl"]);}
        if (json.containsKey("i2c_addr"))         { this->i2c_addr = strtoul(json["i2c_addr"], NULL, 16);} // hex convert to dec        
      } else {
        Serial.println("failed to load json config, load default config");
        loadDefaultConfig = true;
      }
    }
  } else {
    Serial.println("BaseConfig.json config File not exists, load default config");
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    this->ow_apikey = "abcefghijklmnopqrstuwvxyz";
    this->ow_country  = "de";
    this->ow_city = "Berlin";
    this->pin_sda = 5;
    this->pin_scl = 4;
    this->i2c_addr = 60; //0x3C;
    
    loadDefaultConfig = false; //set back
  }

}

void BaseConfig::GetWebContent(ESPWebServer* server) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  String html = "";
  
  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 250px;'>Name</td>\n");
  html.concat("<td style='width: 200px;'>Wert</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");
/*
  html.concat("<tr>\n");
  html.concat("<td>Device Name</td>\n");
  sprintf(buffer, "<td><input size='30' maxlength='40' name='mqttroot' type='text' value='%s'/></td>\n", this->mqtt_root.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  html.concat("<tr>\n");
  html.concat("<td>MQTT Server IP</td>\n");
  sprintf(buffer, "<td><input size='30' name='mqttserver' type='text' value='%s'/></td>\n", this->mqtt_server.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  html.concat("<tr>\n");
  html.concat("<td>MQTT Server Port</td>\n");
  sprintf(buffer, "<td><input maxlength='5' name='mqttport' type='text' value='%d'/></td>\n", this->mqtt_port);
  html.concat(buffer);
  html.concat("</tr>\n");
*/
  html.concat("<tr>\n");
  html.concat("<td colspan='2'><b>Konfiguration Display</b></td>\n");
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Pin i2c SDA</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_0' name='pinsda' type='number' value='%d'/></td>\n", this->pin_sda );
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Pin i2c SCL</td>\n");
  sprintf(buffer, "<td><input min='0' max='15' id='GpioPin_1' name='pinscl' type='number' value='%d'/></td>\n", this->pin_scl );
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>i2c Adresse Display</td>\n");
  sprintf(buffer, "<td><input maxlength='2' name='i2c_addr' type='text' value='%02x'/></td>\n", this->i2c_addr);
  html.concat(buffer);
  html.concat("</tr>\n");

  server->sendContent(html.c_str()); html = "";
  
  html.concat("<tr>\n");
  html.concat("<td colspan='2'><b>Konfiguration OpenWeather</b></td>\n");
  html.concat("</tr>\n");
  
  html.concat("<tr>\n");
  html.concat("<td>API-Key</td>\n");
  sprintf(buffer, "<td><input size='50' name='ow_appikey' type='text' value='%s'/></td>\n", this->ow_apikey.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Länderkennung</td>\n");
  sprintf(buffer, "<td><input maxlength='2' name='ow_country' type='text' value='%s'/></td>\n", this->ow_country.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Stadt</td>\n");
  sprintf(buffer, "<td><input size='12' name='ow_city' type='text' value='%s'/></td>\n", this->ow_city.c_str());
  html.concat(buffer);
  html.concat("</tr>\n");
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");


  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreBaseConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");
  html.concat("<div id='ErrorText' class='errortext'></div>\n");  

  server->sendContent(html.c_str());
}

