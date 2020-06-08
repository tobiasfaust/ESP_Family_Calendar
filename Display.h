#ifndef DISPLAY_H
#define DISPLAY_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// GxEPD library - https://github.com/ZinggJM/GxEPD
#include <GxEPD.h>
#include <GxGDEW075T7/GxGDEW075T7.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <TimeLib.h>

// Adafruit GFX
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>

#include "BaseConfig.h"
#include "Weather.h"

extern BaseConfig* Config;
extern Weather* owW;

class Display {
  //enum        IconSize_t {SMALL = 4, LARGE = 6};
  enum        IconSize_t {SMALL = 6, LARGE = 17};

  String dayName[7]   = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Sonnabend"};
  String dayShortName[7] = {"So", "Mo", "Die", "Mi", "Do", "Fr", "Sa"};
  String monthName[12] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
    
  public:
    Display();

  private:
    GxIO_Class* io = NULL; //(SPI, /*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16); // arbitrary selection of 17, 16
    GxEPD_Class* display = NULL; //(io, /*RST=*/ 16, /*BUSY=*/ 4); // arbitrary selection of (16), 4

    uint16_t  DisplaySizeX = GxEPD_HEIGHT;
    uint16_t  DisplaySizeY = GxEPD_WIDTH;

    void      initialize();
    void      showInfoText(String s);
    void      DisplayWeather();
    void      DisplayCalendar();
    void      GenerateCalendar(int x, int y);
    
    void      showWeatherCondition(int x, int y, String IconName, IconSize_t IconSize);
    void      addcloud(int x, int y, int scale, int linesize);
    void      addraindrop(int x, int y, int scale);
    void      addrain(int x, int y, int scale, IconSize_t IconSize);
    void      addsnow(int x, int y, int scale, IconSize_t IconSize);
    void      addtstorm(int x, int y, int scale);
    void      addsun(int x, int y, int scale, IconSize_t IconSize);
    void      addfog(int x, int y, int scale, int linesize, IconSize_t IconSize);
    void      Sunny(int x, int y, IconSize_t IconSize, String IconName);
    void      MostlySunny(int x, int y, IconSize_t IconSize, String IconName);
    void      MostlyCloudy(int x, int y, IconSize_t IconSize, String IconName);
    void      Cloudy(int x, int y, IconSize_t IconSize, String IconName);
    void      Rain(int x, int y, IconSize_t IconSize, String IconName);
    void      ExpectRain(int x, int y, IconSize_t IconSize, String IconName);
    void      ChanceRain(int x, int y, IconSize_t IconSize, String IconName);
    void      Tstorms(int x, int y, IconSize_t IconSize, String IconName);
    void      Snow(int x, int y, IconSize_t IconSize, String IconName) ;
    void      Fog(int x, int y, IconSize_t IconSize, String IconName);
    void      Haze(int x, int y, IconSize_t IconSize, String IconName);
    void      CloudCover(int x, int y, int CCover);
    void      Visibility(int x, int y, String Visi);
    void      addmoon(int x, int y, int scale, IconSize_t IconSize);
    void      Nodata(int x, int y, IconSize_t IconSize);
 
};

#endif
