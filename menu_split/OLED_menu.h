//extends the menu's display class to add support for an adafruit 
//we include this stuf fhere since... it's the entire point of this class
//this class would NOT be packaged with the menu library, it is the implementation details
#ifndef OLED_MENU_hpp
#define OLED_MENU_hpp

#define OLED_RESET -1
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT  64
//#define USE_SSD1306
#define USE_SH1106

#include "menu.h"
#include <Adafruit_GFX.h>       //  Generic graphics library: fonts, lines, effects

#ifdef USE_SSD1306
#include <Adafruit_SSD1306.h>
#define I2CADDR  0x3D
#define OLED_CLASS Adafruit_SSD1306
#define OLED_WHITE SSD1306_WHITE
#endif
#ifdef USE_SH1106
#include <Adafruit_SH110X.h>
#define I2CADDR  0x3C
#define OLED_CLASS Adafruit_SH1106G
#define OLED_WHITE SH110X_WHITE
#endif


class OLED_Display : public Display{
  protected:
  OLED_CLASS OLED = OLED_CLASS(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  void setupOLED(int addr);
  public:
  OLED_Display(){
    
  }
  void setup(){
    setupOLED(I2CADDR);
  }
  
  void setCursor(int x, int y) override;
  void printText(int textSize, char *message) override;
  void printText(int textSize, String message) override;
  void printText(int textSize, double message) override;
  void printText(int textSize, long message) override;
  int getSizeX() override;
  int getSizeY() override;
  void printRect(int x, int y, int w, int h) override;
  void printCircle(int x, int y, int r) override;
  void clear() override;
  void display() override;
  OLED_CLASS& getOled(){
    return OLED;
  }
};

#endif
