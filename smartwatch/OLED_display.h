#ifndef OLED_DISPLAY_hpp
#define OLED_DISPLAY_hpp

#include "display_config.h"

//Generic graphics library: fonts, lines, effects
#include <Adafruit_GFX.h>

//Abstract display class we are implementing
#include "display_wrapper.h"

const int OLED_RESET = -1;
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 64;

template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
class OLED_Display : public Display {
protected:
  OLED_CLASS OLED = OLED_CLASS(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
  void beginOled(OLED_CLASS& OLED, int addr);
  void setupOLED(int addr);
public:
  OLED_Display() {
  }
  void setup() {
    setupOLED(I2CADDR);
  }
  int getSizeX() override;
  int getSizeY() override;
  void clear() override;
  void display() override;
  void setCursor(int x, int y) override;
  void print(const char* message, int textSize) override;
  void print(double message, int textSize) override;
  void print(long message, int textSize) override;

  void drawRect(int x, int y, int w, int h, bool fill) override;
  void drawCircle(int x, int y, int r, bool fill) override;

  void drawLine(int x1, int y1, int x2, int y2) override;
  void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, bool fill) override;
  void drawEvenCircle(int x, int y, int r, bool fill) override;
  void drawRoundedRect(int x, int y, int w, int h, int r, bool fill) override;
  /*
  OLED_CLASS& getOLED() {
    return OLED;
  }
  */
};


template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::setupOLED(int addr) {
  //Connect to OLED via I2C
  beginOLED(OLED, addr);

  //Show Adafruit splash screen stored in image buffer.
  OLED.display();
  delay(1000);

  //Clear splash screen and set default settings
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(OLED_WHITE);
}

template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::setCursor(int x, int y) {
  OLED.setCursor(x, y);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::print(const char* message, int textSize) {
  OLED.setTextSize(textSize);
  OLED.print(message);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::print(double message, int textSize) {
  OLED.setTextSize(textSize);
  OLED.print(message);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::print(long message, int textSize) {
  OLED.setTextSize(textSize);
  OLED.print(message);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
int OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::getSizeX() {
  return SCREEN_WIDTH;
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
int OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::getSizeY() {
  return SCREEN_HEIGHT;
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawRect(int x, int y, int w, int h, bool fill) {
  if (fill) {
    OLED.fillRect(x, y, w, h, OLED_WHITE);
  } else {
    OLED.drawRect(x, y, w, h, OLED_WHITE);
  }
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawCircle(int x, int y, int r, bool fill) {
  if (fill) {
    OLED.fillCircle(x, y, r, OLED_WHITE);
  } else {
    OLED.drawCircle(x, y, r, OLED_WHITE);
  }
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawLine(int x1, int y1, int x2, int y2) {
  OLED.drawLine(x1, y1, x2, y2, OLED_WHITE);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, bool fill) {
  if (fill) {
    OLED.fillTriangle(x1, y1, x2, y2, x3, y3, OLED_WHITE);
  } else {
    OLED.drawTriangle(x1, y1, x2, y2, x3, y3, OLED_WHITE);
  }
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawEvenCircle(int x, int y, int r, bool fill) {
  if (fill) {
    OLED.fillRoundRect(x - r + 1, y - r + 1, 2 * r, 2 * r, r, OLED_WHITE);
  } else {
    OLED.drawRoundRect(x - r + 1, y - r + 1, 2 * r, 2 * r, r, OLED_WHITE);
  }
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::drawRoundedRect(int x, int y, int w, int h, int r, bool fill) {
  if (fill) {
    OLED.fillRoundRect(x, y, w, h, r, OLED_WHITE);
  } else {
    OLED.drawRoundRect(x, y, w, h, r, OLED_WHITE);
  }
}

template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::clear() {
  OLED.clearDisplay();
  OLED.setCursor(0, 0);
}
template<typename OLED_CLASS, void (*beginOLED)(OLED_CLASS&, int), int I2CADDR, int OLED_WHITE>
void OLED_Display<OLED_CLASS, beginOLED, I2CADDR, OLED_WHITE>::display() {
  OLED.display();
}

#endif