 #include "OLED_menu.h"

void beginOled(OLED_CLASS& OLED, int addr){
  #ifdef USE_SSD1306
  while ( !OLED.begin(SSD1306_SWITCHCAPVCC, addr) ) {
    Serial.println(F("SSD1306 allocation failed"));
    delay(100);
  }
  #endif
  #ifdef USE_SH1106
  while ( !OLED.begin(addr, true) ) {
    Serial.println(F("SSD110X allocation failed"));
    delay(100);
  }
  #endif
}

void OLED_Display::setupOLED(int addr){
  // -- Set up OLED display.  Use internal 3.3v supply, and Adafruit trick
  //      SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  //    I2C address is 0x3C for the 128x32 display

  beginOled(OLED, addr);

  // -- Show Adafruit splash screen stored in image buffer.
  OLED.display();
  delay(1000);         //  Pause to allow user to read the display
  
  // -- Clear the splash screen, set default text mode and display a "ready" message.
  OLED.clearDisplay();
  OLED.setTextSize(1);
  OLED.setTextColor(OLED_WHITE);
  OLED.print(F("OLED is ready"));
  OLED.display();
  delay(1000);                         //  Pause to allow user to read the display
}

void OLED_Display::setCursor(int x, int y){
  OLED.setCursor(x,y);
}
void OLED_Display::printText(int textSize, char *message){
  OLED.setTextSize(textSize);
  OLED.print(message);
}
void OLED_Display::printText(int textSize, String message){
  OLED.setTextSize(textSize);
  OLED.print(message);
}
void OLED_Display::printText(int textSize, double message){
  OLED.setTextSize(textSize);
  OLED.print(message);
}
void OLED_Display::printText(int textSize, long message){
  OLED.setTextSize(textSize);
  OLED.print(message);
}
int OLED_Display::getSizeX(){
  return 128;
}
int OLED_Display::getSizeY(){
  return 64;
}
void OLED_Display::printRect(int x, int y, int w, int h){
  OLED.fillRect(x,y,w,h,OLED_WHITE);
}
void OLED_Display::printCircle(int x, int y, int r){
    OLED.fillCircle(x,y,r,OLED_WHITE);
}
void OLED_Display::clear(){
  OLED.clearDisplay();
}
void OLED_Display::display(){
  OLED.display();
}
