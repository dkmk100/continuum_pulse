#include <string>
#include <buildTime.h>
#include <Wire.h>
#include <Adafruit_GFX.h>

#include "config.h"

#include "OLED_menu.h"

OLED_Display display;
MenuManager screenManager(&display);

void setup() {
  Serial.begin();
  while(!Serial){
    delay(10);
  }
  Serial.println("Hello World");
  display.setup();
  display.display();
  delay(1000);
  display.clear();
  display.setCursor(20, 20);
  display.printText(1, "Hello World");
  display.display();
  Serial.println("Displayed text");
}

void loop() {

}
