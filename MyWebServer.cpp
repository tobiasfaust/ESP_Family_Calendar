#include "MyWebServer.h"

MyWebServer::MyWebServer() : DoReboot(false) {
  this->server = new ESPWebServer(80);
  
  if (!MDNS.begin("esp82660"))  {  Serial.println(F("Error setting up MDNS responder!"));  }
  else                          {  Serial.println(F("mDNS responder started"));  }
  
  server->begin(); 
  MDNS.addService("http", "tcp", 80);

  this->server->onNotFound([this]() { this->handleNotFound(); });
  this->server->on("/", [this]() {this->handleRoot(); });
  this->server->on("/BaseConfig", [this]() {this->handleBaseConfig(); });
  this->server->on("/CalendarConfig", [this]() {this->handleCalendarConfig(); });
  
  this->server->on("/style.css", HTTP_GET, [this]() {this->handleCSS(); });
  this->server->on("/javascript.js", HTTP_GET, [this]() {this->handleJS(); });
  
  this->server->on("/StoreBaseConfig", HTTP_POST, [this]()   { this->ReceiveJSONConfiguration(BASECONFIG); });
  this->server->on("/StoreCalendarConfig", HTTP_POST, [this]() { this->ReceiveJSONConfiguration(CALENDAR); });
  this->server->on("/reboot", HTTP_GET, [this]()             { this->handleReboot(); });

  #if defined(ESP8266)
    httpUpdater.setup(server);
  #else if defined(ESP32)
    this->server->on("/update", HTTP_POST, [this]() {
      this->server->sendHeader("Connection", "close");
      this->server->send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, [this]() {
      HTTPUpload& upload = server->upload();
      if (upload.status == UPLOAD_FILE_START) {
        //Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        //Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });
  #endif

  Serial.println(F("WebServer started..."));

}

void MyWebServer::loop() {
  server->handleClient();
  if (this->DoReboot) {ESP.restart();}
}

void MyWebServer::handleNotFound() {
  this->server->send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void MyWebServer::handleRoot() {
  this->server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  this->server->sendHeader("Pragma", "no-cache");
  this->server->sendHeader("Expires", "-1");
  this->server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  this->server->send(200, "text/html", "");
  
  this->getPageHeader(ROOT);
  this->getPage_Status();
  this->getPageFooter();
  
  this->server->sendContent("");
}

void MyWebServer::handleCSS() {
  this->server->send_P(200, "text/css", STYLE_CSS);
}

void MyWebServer::handleJS() {
  this->server->send_P(200, "text/javascript", JAVASCRIPT);
}

void MyWebServer::handleReboot() {
  this->server->sendHeader("Location","/");
  this->server->send(303); 
  this->DoReboot = true;  
}

void MyWebServer::handleBaseConfig() {
  this->server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  this->server->sendHeader("Pragma", "no-cache");
  this->server->sendHeader("Expires", "-1");
  this->server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  this->server->send(200, "text/html", "");
  this->getPageHeader(BASECONFIG);
  
  Config->GetWebContent(this->server);
  this->getPageFooter();
  this->server->sendContent("");
}

void MyWebServer::handleCalendarConfig() {
  this->server->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  this->server->sendHeader("Pragma", "no-cache");
  this->server->sendHeader("Expires", "-1");
  this->server->setContentLength(CONTENT_LENGTH_UNKNOWN);
  this->server->send(200, "text/html", "");
  this->getPageHeader(CALENDAR);
  
  iCal->GetWebContent(this->server);
  this->getPageFooter();
  this->server->sendContent("");
}


void MyWebServer::ReceiveJSONConfiguration(page_t page) {
  String json = server->arg("json");
  String targetPage = "/";
  Serial.print(F("json empfangen: "));
  Serial.println(FPSTR(json.c_str()));  
  
  if (page==BASECONFIG)  { Config->StoreJsonConfig(&json); targetPage = "/BaseConfig"; }
  if (page==CALENDAR)    { iCal->StoreJsonConfig(&json); targetPage = "/CalendarConfig"; }
  
  server->sendHeader("Location", targetPage.c_str());
  server->send(303); 
}


void MyWebServer::getPageHeader(page_t pageactive) {
  char buffer[200] = {0};
  memset(buffer, 0, sizeof(buffer));
  String html = "";
  
  html.concat("<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'/>\n");
  html.concat("<meta charset='utf-8'>\n");
  html.concat("<link rel='stylesheet' type='text/css' href='/style.css'>\n");
  html.concat("<script language='javascript' type='text/javascript' src='/javascript.js'></script>\n");
  html.concat("<title>Meine Kalenderanzeige</title></head>\n");
  html.concat("<body>\n");
  html.concat("<table>\n");
  html.concat("  <tr>\n");
  html.concat("   <td colspan='13'>\n");
  html.concat("     <h2>Konfiguration</h2>\n");
  html.concat("   </td>\n");
  html.concat(" </tr>\n");
  html.concat(" <tr>\n");
  html.concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/'>Status</a></td>\n", (pageactive==ROOT)?"navi_active":"");
  html.concat(buffer);
  html.concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/BaseConfig'>Basis Config</a></td>\n", (pageactive==BASECONFIG)?"navi_active":"");
  html.concat(buffer);
  html.concat("   <td class='navi' style='width: 50px'></td>\n");
  sprintf(buffer, "   <td class='navi %s' style='width: 100px'><a href='/CalendarConfig'>Kalender Config</a></td>\n", (pageactive==CALENDAR)?"navi_active":"");
  html.concat(buffer);
  html.concat("   <td class='navi' style='width: 50px'></td>\n");
  html.concat("   <td class='navi' style='width: 100px'><a href='https://github.com/tobiasfaust/ESP8266_PumpControl/wiki' target='_blank'>Wiki</a></td>\n");
  html.concat("   <td class='navi' style='width: 50px'></td>\n");
  html.concat(" </tr>\n");
  html.concat("  <tr>\n");
  html.concat("   <td colspan='11'>\n");
  html.concat("   <p />\n");

  this->server->sendContent(html.c_str());
}

void MyWebServer::getPageFooter() {
  String html = "";
  html.concat("</html>\n");
  this->server->sendContent(html.c_str());
}

void MyWebServer::getPage_Status() {
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));
  uint8_t count = 0;
  String html = "";

  html.concat("<table class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 250px;'>Name</td>\n");
  html.concat("<td style='width: 200px;'>Wert</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  html.concat("<tbody>\n");

  html.concat("<tr>\n");
  html.concat("<td>IP-Adresse:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.localIP().toString().c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>WiFi Name:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.SSID().c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>i2c Bus:</td>\n");
  html.concat("<td>");
  html.concat("</td>\n");
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>MAC:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", WiFi.macAddress().c_str());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>WiFi RSSI:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", WiFi.RSSI());
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Uptime:</td>\n");
  sprintf(buffer, "<td>%s</td>\n", "---");
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Free Heap Memory:</td>\n");
  sprintf(buffer, "<td>%d</td>\n", ESP.getFreeHeap()); //https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/heap_debug.html
  html.concat(buffer);
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Firmware Update</td>\n");
  html.concat("<td><form action='update'><input class='button' type='submit' value='Update' /></form></td>\n");
  html.concat("</tr>\n");

  html.concat("<tr>\n");
  html.concat("<td>Device Reboot</td>\n");
  html.concat("<td><form action='reboot'><input class='button' type='submit' value='Reboot' /></form></td>\n");
  html.concat("</tr>\n");

  html.concat("</tbody>\n");
  html.concat("</table>\n");

  this->server->sendContent(html.c_str());
}

