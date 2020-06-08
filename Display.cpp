#include "Display.h"

//#########################################################################################
Display::Display() {
  io = new GxIO_Class (SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
  display = new GxEPD_Class (*io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

  display->init();
  this->initialize();
  this->showInfoText("Initializing...");

  this->DisplayWeather();

}


//#########################################################################################
void Display::initialize() {
  display->setRotation(0);
  display->fillScreen(GxEPD_WHITE);
  display->drawFastVLine(this->DisplaySizeX / 2   , 0                     , this->DisplaySizeY      , GxEPD_BLACK); // Trenner Kalender/Wetter links / Termine rechts
  display->drawFastHLine(0                        , this->DisplaySizeY / 2, this->DisplaySizeX / 2  , GxEPD_BLACK); // Trenner Links: Kalender oben, Wetter unten
  display->drawFastVLine(this->DisplaySizeX / 4   , this->DisplaySizeY / 2, this->DisplaySizeY /2   , GxEPD_BLACK); // Trenner Wetter: links aktuell, rechts Vorschau
  display->drawFastHLine(this->DisplaySizeX / 4   , this->DisplaySizeY / 4, this->DisplaySizeY /4   , GxEPD_BLACK); // Trenner Wetter Vorschau: Morgen oben, Übermorgen unten
  display->update();
}

//#########################################################################################
// Zeige Infotext unten im Footer
//#########################################################################################
void Display::showInfoText(String s) {
  // Box für footer 400x300, hochkant: 300x400
  display->fillRect(0, this->DisplaySizeY-20, this->DisplaySizeX, 20, GxEPD_BLACK);
  display->setTextColor(GxEPD_WHITE);
  display->setFont(&FreeSansBold9pt7b);
  display->setCursor(4, this->DisplaySizeY-5); 
  display->println(s.c_str());
  display->updateWindow(0, this->DisplaySizeY-20, this->DisplaySizeX, 20, false); // fast update footer  
}

//#########################################################################################
void Display::DisplayWeather() {
  this->showWeatherCondition(20                           , (this->DisplaySizeY / 2) + 20, owW->GetIcon(0), (IconSize_t)LARGE); // Weather today
  this->showWeatherCondition((this->DisplaySizeX / 4) + 20, (this->DisplaySizeY / 2) + 10, owW->GetIcon(1), (IconSize_t)SMALL); // Weather tomorrow
  this->showWeatherCondition((this->DisplaySizeX / 4) + 20, (this->DisplaySizeY / 4) + 10, owW->GetIcon(2), (IconSize_t)SMALL); // Weather day after tomorrow
}

//#########################################################################################
void Display::DisplayCalendar() {
  display->setTextColor(GxEPD_BLACK);
  
  display->setFont(&FreeSans12pt7b);
  display->setCursor(4, 15);
  display->println(this->dayName[weekday()]);
  
  display->setFont(&FreeSansBold24pt7b);
  display->setCursor(this->DisplaySizeX / 8, 40);
  display->println(day());

  display->setFont(&FreeSansBold18pt7b);
  display->setCursor(this->DisplaySizeX / 8, 50);
  display->println(this->monthName[month()]+" " + year());

  this->GenerateCalendar(5, 50);
  display->updateWindow((this->DisplaySizeX / 2) -2 ,0, (this->DisplaySizeX / 2) -2, (this->DisplaySizeY / 2) -2, false);
}

//#########################################################################################
void Display::GenerateCalendar(int x, int y) {
  display->setFont(&FreeSansBold9pt7b);
  for (uint8_t d=0; d<7; d++) {
    display->setCursor(x + (i*10), y);
    display->println(day());
  }
  
  time_t t = now() - (day() * 86400); // zeit am ersten Tag des Monats
  int d_x = x; d_y = y + 10;
  display->setFont(&FreeSans9pt7b);
  for (uint8_t d=0; d<40; d++) {
    if (day(t) == d) { 
     display->setCursor(d_x, d_y);
     display->println(day());
     t += 86400; 
    }
    if (modulo(d,7)==0) { //wochenumbruch
      d_x += x; d_y += 10;
    } else { d_x += 10; }
    }
  }
}

//#########################################################################################
void Display::showWeatherCondition(int x, int y, String IconName, IconSize_t IconSize) {
  if      (IconName == "01d" || IconName == "01n")  this->Sunny       (x, y, IconSize, IconName);
  else if (IconName == "02d" || IconName == "02n")  this->MostlySunny (x, y, IconSize, IconName);
  else if (IconName == "03d" || IconName == "03n")  this->Cloudy      (x, y, IconSize, IconName);
  else if (IconName == "04d" || IconName == "04n")  this->MostlySunny (x, y, IconSize, IconName);
  else if (IconName == "09d" || IconName == "09n")  this->ChanceRain  (x, y, IconSize, IconName);
  else if (IconName == "10d" || IconName == "10n")  this->Rain        (x, y, IconSize, IconName);
  else if (IconName == "11d" || IconName == "11n")  this->Tstorms     (x, y, IconSize, IconName);
  else if (IconName == "13d" || IconName == "13n")  this->Snow        (x, y, IconSize, IconName);
  else if (IconName == "50d")                       this->Haze        (x, y, IconSize, IconName);
  else if (IconName == "50n")                       this->Fog         (x, y, IconSize, IconName);
  else                                              this->Nodata      (x, y, IconSize);
  
}

//#########################################################################################
// Symbols are drawn on a relative 10x10grid and 1 scale unit = 1 drawing unit
void Display::addcloud(int x, int y, int scale, int linesize) {
  //Draw cloud outer
  display->fillCircle(x - scale * 3, y, scale, GxEPD_BLACK);                // Left most circle
  display->fillCircle(x + scale * 3, y, scale, GxEPD_BLACK);                // Right most circle
  display->fillCircle(x - scale, y - scale, scale * 1.4, GxEPD_BLACK);    // left middle upper circle
  display->fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75, GxEPD_BLACK); // Right middle upper circle
  display->fillRect(x - scale * 3 - 1, y - scale, scale * 6, scale * 2 + 1, GxEPD_BLACK); // Upper and lower lines
  //Clear cloud inner
  display->fillCircle(x - scale * 3, y, scale - linesize, GxEPD_WHITE);            // Clear left most circle
  display->fillCircle(x + scale * 3, y, scale - linesize, GxEPD_WHITE);            // Clear right most circle
  display->fillCircle(x - scale, y - scale, scale * 1.4 - linesize, GxEPD_WHITE);  // left middle upper circle
  display->fillCircle(x + scale * 1.5, y - scale * 1.3, scale * 1.75 - linesize, GxEPD_WHITE); // Right middle upper circle
  display->fillRect(x - scale * 3 + 2, y - scale + linesize - 1, scale * 5.9, scale * 2 - linesize * 2 + 2, GxEPD_WHITE); // Upper and lower lines
}
//#########################################################################################
void Display::addraindrop(int x, int y, int scale) {
  display->fillCircle(x, y, scale / 2, GxEPD_BLACK);
  display->fillTriangle(x - scale / 2, y, x, y - scale * 1.2, x + scale / 2, y , GxEPD_BLACK);
  x = x + scale * 1.6; y = y + scale / 3;
  display->fillCircle(x, y, scale / 2, GxEPD_BLACK);
  display->fillTriangle(x - scale / 2, y, x, y - scale * 1.2, x + scale / 2, y , GxEPD_BLACK);
}
//#########################################################################################
void Display::addrain(int x, int y, int scale, IconSize_t IconSize) {
  if (IconSize == SMALL) scale *= 1.34;
  for (int d = 0; d < 4; d++) {
    addraindrop(x + scale * (7.8 - d * 1.95) - scale * 5.2, y + scale * 2.1 - scale / 6, scale / 1.6);
  }
}
//#########################################################################################
void Display::addsnow(int x, int y, int scale, IconSize_t IconSize) {
  int dxo, dyo, dxi, dyi;
  for (int flakes = 0; flakes < 5; flakes++) {
    for (int i = 0; i < 360; i = i + 45) {
      dxo = 0.5 * scale * cos((i - 90) * 3.14 / 180); dxi = dxo * 0.1;
      dyo = 0.5 * scale * sin((i - 90) * 3.14 / 180); dyi = dyo * 0.1;
      display->drawLine(dxo + x + flakes * 1.5 * scale - scale * 3, dyo + y + scale * 2, dxi + x + 0 + flakes * 1.5 * scale - scale * 3, dyi + y + scale * 2, GxEPD_BLACK);
    }
  }
}
//#########################################################################################
void Display::addtstorm(int x, int y, int scale) {
  y = y + scale / 2;
  IconSize_t Small = SMALL;
  for (int i = 0; i < 5; i++) {
    display->drawLine(x - scale * 4 + scale * i * 1.5 + 0, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 0, y + scale, GxEPD_BLACK);
    if (scale != Small) {
      display->drawLine(x - scale * 4 + scale * i * 1.5 + 1, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 1, y + scale, GxEPD_BLACK);
      display->drawLine(x - scale * 4 + scale * i * 1.5 + 2, y + scale * 1.5, x - scale * 3.5 + scale * i * 1.5 + 2, y + scale, GxEPD_BLACK);
    }
    display->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 0, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 0, GxEPD_BLACK);
    if (scale != Small) {
      display->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 1, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 1, GxEPD_BLACK);
      display->drawLine(x - scale * 4 + scale * i * 1.5, y + scale * 1.5 + 2, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5 + 2, GxEPD_BLACK);
    }
    display->drawLine(x - scale * 3.5 + scale * i * 1.4 + 0, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 0, y + scale * 1.5, GxEPD_BLACK);
    if (scale != Small) {
      display->drawLine(x - scale * 3.5 + scale * i * 1.4 + 1, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 1, y + scale * 1.5, GxEPD_BLACK);
      display->drawLine(x - scale * 3.5 + scale * i * 1.4 + 2, y + scale * 2.5, x - scale * 3 + scale * i * 1.5 + 2, y + scale * 1.5, GxEPD_BLACK);
    }
  }
}
//#########################################################################################
void Display::addsun(int x, int y, int scale, IconSize_t IconSize) {
  int linesize = 3;
  if (IconSize == SMALL) linesize = 1;
  display->fillRect(x - scale * 2, y, scale * 4, linesize, GxEPD_BLACK);
  display->fillRect(x, y - scale * 2, linesize, scale * 4, GxEPD_BLACK);
  display->drawLine(x - scale * 1.3, y - scale * 1.3, x + scale * 1.3, y + scale * 1.3, GxEPD_BLACK);
  display->drawLine(x - scale * 1.3, y + scale * 1.3, x + scale * 1.3, y - scale * 1.3, GxEPD_BLACK);
  if (IconSize == LARGE) {
    display->drawLine(1 + x - scale * 1.3, y - scale * 1.3, 1 + x + scale * 1.3, y + scale * 1.3, GxEPD_BLACK);
    display->drawLine(2 + x - scale * 1.3, y - scale * 1.3, 2 + x + scale * 1.3, y + scale * 1.3, GxEPD_BLACK);
    display->drawLine(3 + x - scale * 1.3, y - scale * 1.3, 3 + x + scale * 1.3, y + scale * 1.3, GxEPD_BLACK);
    display->drawLine(1 + x - scale * 1.3, y + scale * 1.3, 1 + x + scale * 1.3, y - scale * 1.3, GxEPD_BLACK);
    display->drawLine(2 + x - scale * 1.3, y + scale * 1.3, 2 + x + scale * 1.3, y - scale * 1.3, GxEPD_BLACK);
    display->drawLine(3 + x - scale * 1.3, y + scale * 1.3, 3 + x + scale * 1.3, y - scale * 1.3, GxEPD_BLACK);
  }
  display->fillCircle(x, y, scale * 1.3, GxEPD_WHITE);
  display->fillCircle(x, y, scale, GxEPD_BLACK);
  display->fillCircle(x, y, scale - linesize, GxEPD_WHITE);
}
//#########################################################################################
void Display::addfog(int x, int y, int scale, int linesize, IconSize_t IconSize) {
  if (IconSize == SMALL) {
    y -= 10;
    linesize = 1;
  }
  for (int i = 0; i < 6; i++) {
    display->fillRect(x - scale * 3, y + scale * 1.5, scale * 6, linesize, GxEPD_BLACK);
    display->fillRect(x - scale * 3, y + scale * 2.0, scale * 6, linesize, GxEPD_BLACK);
    display->fillRect(x - scale * 3, y + scale * 2.5, scale * 6, linesize, GxEPD_BLACK);
  }
}
//#########################################################################################
void Display::Sunny(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize;
  if (IconSize == SMALL) y = y - 3; // Shift up small sun icon
  if (IconName.endsWith("n")) addmoon(x, y + 3, scale, IconSize);
  scale = scale * 1.6;
  addsun(x, y, scale, IconSize);
}
//#########################################################################################
void Display::MostlySunny(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3, offset = 5;
  if (IconSize == LARGE) {
    offset = 10;
  }
  if (IconSize==SMALL) linesize = 1;
  if (IconName.endsWith("n")) addmoon(x, y + offset, scale, IconSize);
  addcloud(x, y + offset, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8 + offset, scale, IconSize);
}
//#########################################################################################
void Display::MostlyCloudy(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == LARGE) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
}
//#########################################################################################
void Display::Cloudy(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
    linesize = 1;
    addcloud(x, y, scale, linesize);
  }
  else {
    y += 10;
    if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
    addcloud(x + 30, y - 45, 5, linesize); // Cloud top right
    addcloud(x - 20, y - 30, 7, linesize); // Cloud top left
    addcloud(x, y, scale, linesize);       // Main cloud
  }
}
//#########################################################################################
void Display::Rain(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}
//#########################################################################################
void Display::ExpectRain(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}
//#########################################################################################
void Display::ChanceRain(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addsun(x - scale * 1.8, y - scale * 1.8, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addrain(x, y, scale, IconSize);
}
//#########################################################################################
void Display::Tstorms(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addtstorm(x, y, scale);
}
//#########################################################################################
void Display::Snow(int x, int y, IconSize_t IconSize, String IconName) {
  int scale = IconSize, linesize = 3;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addcloud(x, y, scale, linesize);
  addsnow(x, y, scale, IconSize);
}
//#########################################################################################
void Display::Fog(int x, int y, IconSize_t IconSize, String IconName) {
  int linesize = 3, scale = IconSize;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addcloud(x, y - 5, scale, linesize);
  addfog(x, y - 5, scale, linesize, IconSize);
}
//#########################################################################################
void Display::Haze(int x, int y, IconSize_t IconSize, String IconName) {
  int linesize = 3, scale = IconSize;
  if (IconSize == SMALL) {
    linesize = 1;
  }
  if (IconName.endsWith("n")) addmoon(x, y, scale, IconSize);
  addsun(x, y - 5, scale * 1.4, IconSize);
  addfog(x, y - 5, scale * 1.4, linesize, IconSize);
}
//#########################################################################################
void Display::CloudCover(int x, int y, int CCover) {
  IconSize_t Small = SMALL;
  addcloud(x - 9, y - 3, Small * 0.5, 2); // Cloud top left
  addcloud(x + 3, y - 3, Small * 0.5, 2); // Cloud top right
  addcloud(x, y,         Small * 0.5, 2); // Main cloud
  //u8g2Fonts.setFont(u8g2_font_helvB08_tf);
  //drawString(x + 15, y - 5, String(CCover) + "%", LEFT);
}
//#########################################################################################
void Display::Visibility(int x, int y, String Visi) {
  y = y - 3; //
  float start_angle = 0.52, end_angle = 2.61;
  int r = 10;
  for (float i = start_angle; i < end_angle; i = i + 0.05) {
    display->drawPixel(x + r * cos(i), y - r / 2 + r * sin(i), GxEPD_BLACK);
    display->drawPixel(x + r * cos(i), 1 + y - r / 2 + r * sin(i), GxEPD_BLACK);
  }
  start_angle = 3.61; end_angle = 5.78;
  for (float i = start_angle; i < end_angle; i = i + 0.05) {
    display->drawPixel(x + r * cos(i), y + r / 2 + r * sin(i), GxEPD_BLACK);
    display->drawPixel(x + r * cos(i), 1 + y + r / 2 + r * sin(i), GxEPD_BLACK);
  }
  display->fillCircle(x, y, r / 4, GxEPD_BLACK);
  //u8g2Fonts.setFont(u8g2_font_helvB08_tf);
  //drawString(x + 12, y - 3, Visi, LEFT);
}
//#########################################################################################
void Display::addmoon(int x, int y, int scale, IconSize_t IconSize) {
  if (IconSize == LARGE) {
    display->fillCircle(x - 62, y - 68, scale, GxEPD_BLACK);
    display->fillCircle(x - 43, y - 68, scale * 1.6, GxEPD_WHITE);
  }
  else
  {
    display->fillCircle(x - 25, y - 15, scale, GxEPD_BLACK);
    display->fillCircle(x - 18, y - 15, scale * 1.6, GxEPD_WHITE);
  }
}
//#########################################################################################
void Display::Nodata(int x, int y, IconSize_t IconSize) {
  if (IconSize = LARGE) display->setTextSize(3); else display->setTextSize(1);
  display->setCursor(x, y);
  display->println("?");
  display->setTextSize(1);
}
//#########################################################################################
