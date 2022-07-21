//extends the menu's display class to add support for an adafruit 
//we include this stuf fhere since... it's the entire point of this class
//this class would NOT be packaged with the menu library, it is the implementation details
#ifndef OLED_MENU_hpp
#define OLED_MENU_hpp

#include "menu.h"
#include <Adafruit_GFX.h>       //  Generic graphics library: fonts, lines, effects
#include <Adafruit_SSD1306.h>   //  Library for the micro OLED display

#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_WIDTH 128  // OLED display width in pixels
#define SCREEN_HEIGHT  64  // OLED display height in pixels
#define I2CADDR  0x3D      // I2C address is used in setupOLED()

class OLED_Display : public Display{
  protected:
  Adafruit_SSD1306* OLED;
  void setupOLED();
  public:
  OLED_Display(){
    OLED = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  }
  void setup(){
    setupOLED();
  }
  /*
  ~OLED_Display(){
    delete OLED;
  }
  */
  
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
  Adafruit_SSD1306& getOled(){
    return *OLED;
  }
};

#endif
