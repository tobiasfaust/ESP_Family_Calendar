#include "calendar.h"

/*****************************************************************************************
* Konstruktor
*****************************************************************************************/ 
calendar::calendar() {
  this->appointment  = new std::vector<appointment_t>{};
  this->CalURL = new std::vector<url_t>{};
  this->SetDayPeriod(7); //Default
  this->UpdateInterval = 1800; // Update alle 30min

  this->LoadJsonConfig();
}

void calendar::StoreJsonConfig(String* json) {
  //https://arduinojson.org/v5/api/jsonobject/begin_end/
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(*json);
    
  if (root.success()) {
    File configFile = SPIFFS.open("/CalendarConfig.json", "w");
    if (!configFile) {
      Serial.println("failed to open CalendarConfig.json file for writing");
    } else {  
      root.printTo(Serial);
      root.printTo(configFile);
      configFile.close();
  
      LoadJsonConfig();
    }
  }
}

void calendar::LoadJsonConfig() {
  bool loadDefaultConfig = false;
  char buffer[100] = {0};
  memset(buffer, 0, sizeof(buffer));

  CalURL->clear(); // leere den Calendar Vector bevor neu befüllt wird
  
  if (SPIFFS.exists("/CalendarConfig.json")) {
    File configFile = SPIFFS.open("/CalendarConfig.json", "r");
    if (configFile) {
      size_t size = configFile.size();
      // Allocate a buffer to store contents of the file.
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      Serial.println("Lade Calendar Config");
      json.printTo(Serial); Serial.println();
      if (json.success()) {
        uint8_t count = 0;
        if (json.containsKey("count")) { count = json["count"].as<int>(); }
        if(count == 0) {
          Serial.println("something went wrong with CalendarConfig, load default config");
          loadDefaultConfig = true;
        }
        
        for (uint8_t i=0; i<count; i++) {
          //String url = "";
          url_t u;
          sprintf(buffer, "url_%d", i);
          if (json.containsKey(buffer)) { u.url=json[buffer].as<String>(); } else { u.url="https://p24-caldav.icloud.com/published/2/MyCalendarSecretKey1"; }

          sprintf(buffer, "owner_%d", i);
          if (json.containsKey(buffer)) { u.owner=json[buffer].as<String>(); } else { u.owner=""; }
          
          sprintf(buffer, "enabled_%d", i);
          if (json[buffer] && json[buffer] == 1) {u.enabled = true;} else {u.enabled = false;}
 
          this->addCalendar(u);
        }
        
      } else {
        loadDefaultConfig = true;
      }
    } else {
      loadDefaultConfig = true;
    }
  } else {
    loadDefaultConfig = true;
  }

  if (loadDefaultConfig) {
    Serial.println("lade Calendar DefaultConfig");
    url_t u;
    u.url     = "https://p24-caldav.icloud.com/published/2/MyCalendarSecretKey0";
    u.enabled = false;
    u.owner   = "M";
    this->addCalendar(u);
  }
}

/*****************************************************************************************
* Anzahl der Tage die angezeigt werden sollen
*****************************************************************************************/
void calendar::SetDayPeriod(uint8_t days) {
  this->DayPeriod = days;
}

/*****************************************************************************************
* fügt einen neuen Kalender ein
*****************************************************************************************/ 
void calendar::addCalendar(url_t u) {
  char buf [200];  // large enough to hold expected string, or malloc it
  MatchState ms;
  ms.Target(const_cast<char*>(u.url.c_str()));
  char result = ms.Match("https://([^/]+)/(.+)");
  if (result == REGEXP_MATCHED) {
    u.lastupdate=0;
    if (u.enabled==NULL) { u.enabled = false; }
    if (!u.port)    { u.port = 443; }
    
    ms.GetCapture(buf,0); u.host=buf;
    ms.GetCapture(buf,1); u.path="/"; u.path.concat(buf);

    if (u.host.indexOf("icloud")>0)       { u.type=APPLE; }
    else if (u.host.indexOf("google")>0)  { u.type=GOOGLE; }
    else { u.type=UNKNOWN; }
    
    snprintf(buf, sizeof(buf), "Try to add new Calendar: %s:%d , %s", u.host.c_str(), u.port, u.path.c_str());
    Serial.println(buf);
      
    if(u.enabled && this->testConnection(&u)) {
      this->updateData(&u);
    } else {
      u.enabled = false;
    }
    
    Serial.println("Calendar URL added");
    this->CalURL->push_back(u);
  } else {
    snprintf(buf, sizeof(buf), "New Calendar couldn´t be parsed: %s", u.url.c_str());
    Serial.println(buf);
  }
}

/*****************************************************************************************
* Setze das RootCA, nur beim ESP32 nötig
*****************************************************************************************/ 
void calendar::setRootCA(url_t* t) {
  #if defined(ESP8266)
    client.setInsecure();
  #else
    if (t->type == APPLE) {client.setCACert(root_ca_apple);}
    else if (t->type == GOOGLE) {client.setCACert(root_ca_google);}
  #endif

}

/*****************************************************************************************
* Teste ob die URL valide ist
*****************************************************************************************/ 
bool calendar::testConnection(url_t* t) {
  Serial.println("Try to connect...");
  this->setRootCA(t);
  if (!client.connect(t->host.c_str(), t->port)) // Starte SSL Verbindung 
  {
    Serial.println("connection failure!");
    return false;
  }  
  else {
    Serial.println("Connection successful, Yeaaa!!");
    client.stop();
    return true;
  }
}


/*****************************************************************************************
* Aktualisiert einen speziellen Kalender
*****************************************************************************************/ 
void calendar::updateData(url_t* Cal) {
  if (!Cal->enabled) { return; }
  
  Serial.print("Verbinde mit dem server...");
  this->setRootCA(Cal);
  if (!client.connect(Cal->host.c_str(), Cal->port)) {
    Serial.println("Nicht verbunden!");
  }  
  else {
    Serial.println("Verbunden!");
    client.println(String("GET ") + Cal->path + " HTTP/1.1"); // Bilde GET Anfrage über HTTPS
    client.println(String("Host: ") + Cal->host);
    client.println("Connection: close");
    client.println();
 
    while (client.connected()) {
      // Empfange Header
      String line = client.readStringUntil('\n');
        //Serial.println(line);
      if (line == "\r") { 
        //Serial.println("headers received");
        break;
      }
    }
    String rec="";   //int i=0;
    while(client.connected() && client.available()) {
      String line = client.readStringUntil('\n');
      //Serial.println(line);
      if (line.indexOf("END:VEVENT") >=0 ) {
        rec.concat(line);
        this->processCalendarObject(&rec);
      } else if (line.indexOf("BEGIN:VEVENT") >=0 ) {
        rec = line + "\n";
      }
      else {
        rec.concat(line);
        rec.concat("\n");
      }
    }
    client.stop();
    Cal->lastupdate = millis();
  }
}

/*****************************************************************************************
* Aktualisiert alle Kalender
*****************************************************************************************/ 
void calendar::updateData(bool force) {
  Serial.println("start update Data...");
  for (uint8_t i=0; i<this->CalURL->size(); i++) {
    if (this->CalURL->at(i).lastupdate - millis() > this->UpdateInterval || force) {
      this->updateData(&this->CalURL->at(i));
    }
  }
}

/*****************************************************************************************
* Splittet den gesamten String eines Kalenderobjekts pro Zeile in ein Array(Vektor)
*****************************************************************************************/ 
int calendar::splitString(std::vector<String>* v, String* s) {
  // http://wp.scalesoft.de/arduino-split/
  int parserCnt=0;
  int rFromIndex=0, rToIndex=-1;
  String delemiter("\n");
  
  while (true) {
    rFromIndex = rToIndex+1;
    rToIndex = s->indexOf(delemiter,rFromIndex);
    if (rToIndex == 0 || rToIndex == -1) {
      v->push_back(s->substring(rFromIndex));
      return parserCnt++;
    } else {
      v->push_back(s->substring(rFromIndex,rToIndex));
      parserCnt++;
    }
  }
}

/*****************************************************************************************
* Konvertiert ein iCal Datumsobjekt in ein time_t Objekt
*****************************************************************************************/ 
time_t calendar::iCalDate2Time(String* t) {
  char result;
  MatchState ms;
  
  ms.Target(const_cast<char*>(t->c_str()));
  
  if (ms.MatchCount("([^=]+)=([^:]+):(%d+)T(%d+)") > 0) {
    // DTSTART;TZID=Europe/Vienna:20200129T123000
    result = ms.Match("([^=]+)=([^:]+):(%d+)T(%d+)");
    if (result == REGEXP_MATCHED) {
      char datebuf[20]; char timebuf[20];
      ms.GetCapture(datebuf,2); ms.GetCapture(timebuf,3); //20190627T061100
      return(this->mkTime((String)datebuf, (String)timebuf));
    } else {return 0;}  
  } else if (ms.MatchCount("([^:]+):(%d+)T(%d+)") > 0) {
    // DTSTART:20190709T060000Z
    // DTSTART:20200214T173000
    result = ms.Match("([^:]+):(%d+)T(%d+)");
    if (result == REGEXP_MATCHED) {
      char datebuf[20]; char timebuf[20];
      ms.GetCapture(datebuf,1); ms.GetCapture(timebuf,2); 
      return(this->mkTime((String)datebuf, (String)timebuf));
    }   
  } else if (ms.MatchCount("([^:]+):([%d][%d][%d][%d][%d][%d][%d][%d])") > 0) {
    // DTSTART;VALUE=DATE:20200404 //ganztägig
    result = ms.Match("([^:]+):([%d][%d][%d][%d][%d][%d][%d][%d])");
    if (result == REGEXP_MATCHED) {
      char datebuf[20]; char timebuf[20];
      ms.GetCapture(datebuf,1); ms.GetCapture(timebuf,2); 
      return(this->mkTime((String)datebuf, "000000"));
    }
  } else {return 0;}
}

/*****************************************************************************************
* erstellt aus Datum und Zeit ein time_t Objekt
*****************************************************************************************/ 
time_t calendar::mkTime(String Date, String Time) {
  //20190627  -  061100
  tmElements_t tm  = {  Time.substring(4,6).toInt(), 
                        Time.substring(2,4).toInt(), 
                        Time.substring(0,2).toInt(), 
                        NULL, 
                        Date.substring(6,8).toInt(), 
                        Date.substring(4,6).toInt(), 
                        Date.substring(0,4).toInt()-1970
                      };
     
  //snprintf(buf, sizeof(buf), "maketime: %s => %d, %d, %d, %d, %d, %d", s.c_str(), s.substring(0,4).toInt(), s.substring(4,6).toInt(), s.substring(6,8).toInt(), s.substring(9,11).toInt(), s.substring(11,13).toInt(), s.substring(13,15).toInt());
  //Serial.println(buf);
  //snprintf(buf, sizeof(buf), "maketime: %s => %d, %d, %d, %d, %d, %d", s.c_str(), year(a.start), month(a.start), day(a.start), hour(a.start), minute(a.start), second(a.start));
  //Serial.println(buf);
  
  return(makeTime(tm));
}

/*****************************************************************************************
* Verarbeitet und Konvertiert ein Kalenderobjekt und fügt dieses in den Appointment Vektor ein
*****************************************************************************************/ 
void calendar::processCalendarObject(String* cal) {
  std::vector<String> obj;
  appointment_t a;
  MatchState ms;
  char buf [200];  // large enough to hold expected string, or malloc it
  char result;
  
  int cnt = this->splitString(&obj, cal);
  for (int i=0; i<cnt; i++) {
    if(obj.at(i).startsWith("DTSTART")) {
      a.start = this->iCalDate2Time(&obj.at(i));
    }

    if(obj.at(i).startsWith("DTEND")) {
      a.end = this->iCalDate2Time(&obj.at(i));
    }

    if(obj.at(i).startsWith("SUMMARY")) {
      ms.Target(const_cast<char*>(obj.at(i).c_str()));
    
      result = ms.Match("SUMMARY:(.+)");
      if (result == REGEXP_MATCHED) {
        ms.GetCapture(buf,0);
        a.summary=buf;
      } else {a.summary="";}
    }
  }

  Serial.println(" Calendar Object successful parsed");
  if (a.start>0 && a.end>0 && a.summary.length()>0) {
    time_t periodStart = now() - (hour(now())*3600) - (minute(now())*60) - second(now());
    time_t periodEnd = periodStart + (this->DayPeriod * 86400);
    if ((a.start > periodStart && a.end < periodEnd) || (a.start < periodStart && a.end > periodEnd)) {
      Serial.print("Event relevant und in Liste zugefügt: "); Serial.println(a.summary);   
      appointment->push_back(a);
    }
    
  } else {
    Serial.println("Error in parsing Calendar Object: ");
    Serial.println(*cal);
    snprintf(buf, sizeof(buf), "Start: %d , End: %d , Summary: %s", a.start, a.end, a.summary.c_str());
    Serial.println(buf);
  }  
}

/*****************************************************************************************
* WebContent
*****************************************************************************************/ 
void calendar::GetWebContent(ESPWebServer* server) {
  char buffer[400] = {0};
  memset(buffer, 0, sizeof(buffer));
  String html = "";

  html.concat("<p><input type='button' value='&#10010; add new Port' onclick='addrow(\"maintable\")'></p>\n");
  html.concat("<form id='DataForm'>\n");
  html.concat("<table id='maintable' class='editorDemoTable'>\n");
  html.concat("<thead>\n");
  html.concat("<tr>\n");
  html.concat("<td style='width: 25px;'>Nr</td>\n");
  html.concat("<td style='width: 25px;'>Active</td>\n");
  html.concat("<td style='width: 25px;'>Kürzel</td>\n");
  html.concat("<td style='width: 250px;'>Url</td>\n");
  html.concat("<td style='width: 25px;'>Delete</td>\n");
  html.concat("</tr>\n");
  html.concat("</thead>\n");
  server->sendContent(html.c_str()); html = "";

  for(uint8_t i=0; i < this->CalURL->size(); i++) {
    html.concat("<tr>\n");
    sprintf(buffer, "  <td>%d</td>\n", i+1);
    html.concat(buffer);
    html.concat("  <td>\n");
    html.concat("    <div class='onoffswitch'>\n");
    sprintf(buffer, "      <input type='checkbox' name='enabled_%d' class='onoffswitch-checkbox' id='myonoffswitch_%d' %s>\n", i, i, (this->CalURL->at(i).enabled?"checked":""));
    html.concat(buffer);
    sprintf(buffer, "      <label class='onoffswitch-label' for='myonoffswitch_%d'>\n", i);
    html.concat(buffer);
    html.concat("        <span class='onoffswitch-inner'></span>\n");
    html.concat("        <span class='onoffswitch-switch'></span>\n");
    html.concat("      </label>\n");
    html.concat("    </div>\n");
    html.concat("  </td>\n");
    
    sprintf(buffer, "  <td><input maxlength='1' size='1' name='owner_%d' type='text' value='%s'/></td>\n", i, this->CalURL->at(i).owner.c_str());
    html.concat(buffer);
    
    snprintf(buffer, sizeof(buffer), "  <td><input size='100' name='url_%d' type='text' value='%s'/></td>\n", i, this->CalURL->at(i).url.c_str());
    html.concat(buffer);

    html.concat("  <td><input type='button' value='&#10008;' onclick='delrow(this)'></td>\n");

    html.concat("</tr>\n");
    server->sendContent(html.c_str()); html = "";
  }
  
  html.concat("</tbody>\n");
  html.concat("</table>\n");
  html.concat("</form>\n\n<br />\n");
  html.concat("<form id='jsonform' action='StoreCalendarConfig' method='POST' onsubmit='return onSubmit(\"DataForm\", \"jsonform\")'>\n");
  html.concat("  <input type='text' id='json' name='json' />\n");
  html.concat("  <input type='submit' value='Speichern' />\n");
  html.concat("</form>\n\n");
  html.concat("<div id='ErrorText' class='errortext'></div>\n");
  server->sendContent(html.c_str()); html = "";
}

