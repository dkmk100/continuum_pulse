#ifndef DISPLAY_WRAPPER_hpp
#define DISPLAY_WRAPPER_hpp

//the display class is meant as a wrapper for whatever physical display is being used, extend to use the appropriate library
class Display {
protected:
public:
  //sets the text cursor for the display
  virtual void setCursor(int x, int y) = 0;
  
  virtual void print(const char *message, int textSize) = 0;
  virtual void print(double message, int textSize) = 0;
  virtual void print(long message, int textSize) = 0;

  void print(const char *message){
    print(message, 1);
  }
  void print(double message){
    print(message, 1);
  }
  void print(long message){
    print(message, 1);
  }

  //just some size functions for more info about the display.
  virtual int getSizeX() = 0;
  virtual int getSizeY() = 0;
  //used to initialize the display
  virtual void setup() = 0;
  //clears everything on the display
  virtual void clear() = 0;
  //displays whatever is in the buffer
  virtual void display() = 0;

  //prints a rectangle at pos
  virtual void drawRect(int x, int y, int w, int h, bool fill) = 0;
  //prints a circle at pos
  virtual void drawCircle(int x, int y, int r, bool fill) = 0;

  virtual void drawLine(int x1, int y1, int x2, int y2) = 0;
  virtual void drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, bool fill) = 0;
  virtual void drawRoundedRect(int x, int y, int w, int h, int r, bool fill) = 0;
  virtual void drawEvenCircle(int x, int y, int r, bool fill) = 0;
};

//so you don't have to deal with display pointers
//since arduino libraries are supposed to abstract away pointers from the end user
//does nothing else. at all.
/*
class DisplayWrapper {
protected:
  Display *disp;

public:
  DisplayWrapper(Display *disp) {
    this->disp = disp;
  }
  inline void setCursor(int x, int y) {
    disp->setCursor(x, y);
  }
  inline void printText(int textSize, const char *message) {
    disp->printText(textSize, message);
  }
  inline void printText(int textSize, double message) {
    disp->printText(textSize, message);
  }
  inline void printText(int textSize, long message) {
    disp->printText(textSize, message);
  }
  inline int getSizeX() {
    return disp->getSizeX();
  }
  inline int getSizeY() {
    return disp->getSizeY();
  }
  inline void drawRect(int x, int y, int w, int h, bool fill) {
    disp->drawRect(x, y, w, h, fill);
  }
  inline void drawCircle(int x, int y, int r, bool fill) {
    disp->drawCircle(x, y, r, fill);
  }
  inline void clear() {
    disp->clear();
  }
  inline void display() {
    disp->display();
  }
};
*/
#endif