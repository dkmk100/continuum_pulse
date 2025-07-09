#include <string>
#include <buildTime.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include "OLED_display.h"
#include "logger.h"


const unsigned int clockDelay = 12;

//adafruit display
#define USE_SSD1306
#define I2CADDR 0x3D

//got on amazon
//#define USE_SH1106
//#define I2CADDR  0x3C

#ifdef USE_SSD1306
#include <Adafruit_SSD1306.h>
#ifndef I2CADDR
#define I2CADDR 0x3D
#endif
#define OLED_CLASS Adafruit_SSD1306
#define OLED_WHITE SSD1306_WHITE
#endif
#ifdef USE_SH1106
#include <Adafruit_SH110X.h>
#ifndef I2CADDR
#define I2CADDR 0x3C
#endif
#define OLED_CLASS Adafruit_SH1106G
#define OLED_WHITE SH110X_WHITE
#endif

void beginOled(OLED_CLASS& OLED, int addr) {
#ifdef USE_SSD1306
  while (!OLED.begin(SSD1306_SWITCHCAPVCC, addr)) {
    Serial.println(F("SSD1306 allocation failed"));
    delay(100);
  }
#endif
#ifdef USE_SH1106
  while (!OLED.begin(addr, true)) {
    Serial.println(F("SSD110X allocation failed"));
    delay(100);
  }
#endif
}

OLED_Display<OLED_CLASS, &beginOled, I2CADDR, OLED_WHITE> display;

//MenuManager screenManager(&display);

class SerialWriter : public TextWriter {
  void print(const char* text) override {
    Serial.print(text);
  }
  void println(){
    Serial.println();
  }
  void flush(){
    Serial.flush();
  }
};

unsigned long timeToMilli(int hours, int minutes, int seconds) {
  if (hours < 0) {
    hours += 24;
  }
  unsigned long mil = seconds * 1000;
  mil += minutes * 1000 * 60;
  mil += hours * 1000 * 3600;
  return mil;
}

void milliToTime(unsigned long mili, int& hours, int& minutes, int& seconds) {
  int totalSec = mili / 1000;
  //remove days and above
  totalSec = totalSec % (3600 * 24);

  hours = totalSec / 3600;
  totalSec = totalSec % 3600;
  minutes = totalSec / 60;
  totalSec = totalSec % 60;
  seconds = totalSec;
}

unsigned long startTime = timeToMilli(BUILD_HOUR, BUILD_MIN, BUILD_SEC + clockDelay);

void drawHand(Display* disp, float angleFromStart, int r) {
  if (r <= 0) {
    return;
  }
  float x = -sin(angleFromStart + PI);
  float y = cos(angleFromStart + PI);

  int x1 = 63;
  int y1 = 31;

  if (x > 0) {
    x1 += 1;
  }
  if (y > 0) {
    y1 += 1;
  }

  int x2 = x1 + round(x * r);
  int y2 = y1 + round(y * r);

  disp->drawLine(x1, y1, x2, y2);
}

void printClockFace(Display* disp, int hours, int minutes, int seconds, int mil) {
  bool pm = hours >= 12;
  hours = hours % 12;
  mil = mil % 1000;

  float angle1 = TWO_PI * minutes / 60.0;
  float angle2 = TWO_PI * hours / 12.0;

  //face
  disp->drawEvenCircle(63, 31, 30, false);

  //hands
  drawHand(disp, angle1, 25);
  drawHand(disp, angle2, 14);

  //notches
  int notchPos = 3;
  disp->drawEvenCircle(63, notchPos, 1, false);
  disp->drawEvenCircle(63, 62 - notchPos, 1, false);
  disp->drawEvenCircle(32 + notchPos, 31, 1, false);
  disp->drawEvenCircle(94 - notchPos, 31, 1, false);

  if (mil < 500) {
    int rSize = 8;
    disp->drawRect(0, 63 - rSize, rSize, rSize, true);
  }

  disp->setCursor(102, 50);
  if (pm) {
    disp->print("PM", 1);
  } else {
    disp->print("AM", 1);
  }
}

SerialWriter writer;
Logger logger(writer);

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  logger.log("Hello World!");

  display.setup();
  display.display();
  delay(1000);
  display.clear();
  display.setCursor(20, 20);
  display.print("Hello World", 1);
  display.display();
  logger.log("Displayed text!");
  logger.log("Number: %i lol", 7);
  delay(1000);
}

void loop() {
  display.clear();
  unsigned long mil = millis();
  unsigned long ourTime = startTime + mil;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  milliToTime(ourTime, hours, minutes, seconds);

  printClockFace(&display, hours, minutes, seconds, mil);
  display.display();
}
