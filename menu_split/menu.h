#ifndef menu_hpp
#define menu_hpp
#include "Arduino.h"


//the display class is meant as a wrapper for whatever physical display is being used, extend to use the appropriate library
class Display{
  protected:
  //unused ATM, the plan is to allow a toggle between relative and absolute coords for dealing with different screens. 
  //Still in beta though, so not implemented yet.
  bool absoluteMode = false;
  public:
  //does the entire display process for a piece of text.
  void displayText(int textSize, char *message){
    displayText(textSize,String(message));
  }
  void displayText(int textSize, String message){
    clear();
    setCursor(0,0);
    printText(textSize, message);
    display();
  }
  
  void displayTimedText(int seconds, String updateText){
    //absolute mode would provide a danger to this function, probably temp disable it...
    clear();
    setCursor(0,0);

    //The F( ) macro keeps constant strings from using program memory. Very nice to hav
    printText(1,F("t = "));     //  String constant is contained in F(...)
    printText(1,seconds); 
    printText(1,F("  s"));      //  String constant is contained in F(...)

    // -- Displays the other info
    setCursor(0,24); 
    printText(2,updateText);

    display();//actually show the text
  }
  //sets the text cursor for the display
  virtual void setCursor(int x, int y) = 0;
  //sends a message with a text size.
  virtual void printText(int textSize, char *message) = 0;
  virtual void printText(int textSize, String message) = 0;

  //other stuff is easily cast up lol
  virtual void printText(int textSize, double message) = 0;
  virtual void printText(int textSize, long message) = 0;

  //ez casting
  inline void printText(int textSize, int message){
    printText(textSize,(long)message);
  }

  //just some size functions for more info about the display.
  virtual int getSizeX() = 0;
  virtual int getSizeY() = 0;
  //prints a rectangle at pos
  virtual void printRect(int x, int y, int w, int h) = 0;
  //prints a circle at pos
  virtual void printCircle(int x, int y, int r) = 0;
  //clears everything on the display
  virtual void clear() = 0;
  //displays whatever is in the buffer
  virtual void display() = 0;
};

//so you don't have to deal with display pointers
//since arduino libraries are supposed to abstract away pointers from the end user
//does nothing else. at all.
class DisplayWrapper{
  protected:
  Display* disp;
  
  public: 
  DisplayWrapper(Display* disp){
    this->disp = disp;
  }
  inline void displayText(int textSize, char *message){
    disp->displayText(textSize, message);
  }
  inline void displayText(int textSize, String message){
    disp->displayText(textSize, message);
  }
  inline void displayTimedText(int seconds, String updateText){
    disp->displayTimedText(seconds,updateText);
  }
  inline void setCursor(int x, int y){
    disp->setCursor(x,y);
  }
  inline void printText(int textSize, char *message){
    disp->printText(textSize, message);
  }
  inline void printText(int textSize, String message){
    disp->printText(textSize, message);
  }
  inline void printText(int textSize, double message){
    disp->printText(textSize, message);
  }
  inline void printText(int textSize, long message){
    disp->printText(textSize, message);
  }
  inline int getSizeX(){
    return disp->getSizeX();
  }
  inline int getSizeY(){
    return disp->getSizeY();
  }
  inline void printRect(int x, int y, int w, int h){
    disp->printRect(x,y,w,h);
  }
  inline void printCircle(int x, int y, int r){
    disp->printCircle(x,y,r);
  }
  inline void clear(){
    disp->clear();
  }
  inline void display(){
    disp->display();
  }
};

struct MenuManager;

//a screen for the menu manager. Abstract, please extend to use. For a screen with menu items, see NavigationScreen.
struct MenuScreen{
  MenuManager* manager;
  String screenName = "test";
  virtual void onAdvance() = 0;
  virtual void onSelect() = 0;
  //not pure virtual since use can be disabled
  virtual void onAdvanceHold(){
    
  }
  virtual void onSelectHold(){
    
  }

  //default to false, thus not abstract.
  virtual boolean customBack(){
    return false;
  }
  virtual boolean customHold(){
    return false;
  }
  //not pure virtual since use can be disabled
  virtual void onBack(){
    
  }
  
  virtual void displayScreen();
  
  MenuScreen(MenuManager& menuManager){
    manager = &menuManager;
  }
};

//the menu manager class keeps track of screens open, as well as 
class MenuManager {
private:
  int maxStack;
  MenuScreen** stack;
  int count;
  int selected = 0;
  Display* display;
public:
  MenuManager(Display* disp){
    maxStack = 10;
    stack = new MenuScreen*[maxStack];
    count = 0;
    display = disp;
  }
  ~MenuManager(){
    delete[] stack;
  }
  inline DisplayWrapper getDisplay(){
    return DisplayWrapper(display);
  }
  void setScreen(MenuScreen* screen){
    Serial.println("Setting screen");
    if(count >= maxStack){
      display->clear();
      Serial.println("ERROR:: STACK LIMIT REACHED");
      display->setCursor(0,24);
      display->printText(2,"STACK ERROR");
      display->display();
      delay(500);
    }
    else{
      stack[count] = screen;
      count ++;
    }
  }
  //displays the current screen on the stack
  void displayScreen(){
    Serial.println("displaying screen");
    MenuScreen* screen = stack[count-1];
    screen->displayScreen();
  }
  //returns to the previous screen
  void setLast(){
    if(count > 1){
      count --;
    }
  }

  //handle a back button, calls custom screen back functionality
  void back(){
    MenuScreen* screen = stack[count-1];
    if(screen->customBack()){
      screen->onBack();
    }
    else{
      setLast();
    }
  }

  //handles select button
  void select(){
    MenuScreen* screen = stack[count-1];
    screen->onSelect();
  }

  //handles advance button
  void advance(){
    MenuScreen* screen = stack[count-1];
    screen->onAdvance();
  }
};

//menu items, for the nav menu screen. The things that make options menus possible.
struct MenuItem{
  protected:
  MenuManager* manager;
  String itemName;
  public:
  MenuItem(MenuManager* mana, String myName){
    manager = mana;
    itemName = myName;
  }
  virtual void activate(){
    
  }
  virtual String getName(){
    return itemName;
  }
};

//navigation items lead to other screens, thus allowing you to chain screens in order and actually use multiple menus.
struct NavItem : public MenuItem{
  MenuScreen* destination;
  NavItem(MenuManager* mana, MenuScreen* dest, String myName) : MenuItem(mana, myName){
    destination = dest;
  }
  void activate() override{
    manager->setScreen(destination);
    manager->displayScreen();
  }
};


//the navigation menu screen, the core of the settings and menu navigation system. The only default inheritor of menu screen. 
struct NavigationMenuScreen : public MenuScreen{
  MenuItem** menuItems;
  int maxSize = 6;
  int count = 0;
  int selected = 0;
  
  void onAdvance() override{
    selected +=1;
    if(selected > count){
      selected = 0;
    }
  }
  void onSelect() override{
    if(selected >= count){
      manager->setLast();
      manager->displayScreen();
    }
    else{
      menuItems[selected]->activate();
    }
  }
  
  void displayScreen() override;
  
  NavigationMenuScreen(MenuManager& menuManager) : MenuScreen(menuManager){
    menuItems = new MenuItem*[maxSize];
  }
  ~NavigationMenuScreen(){
    delete[] menuItems;
  }
  
  void addMenuItem(MenuItem* item){
     if(count >= maxSize){
      Serial.print("ERROR:: MENU ITEM LIMIT REACHED");
    }
    else{
      menuItems[count] = item;
      count++;
    }
  }
};

struct ToggleSetting{
  boolean active = false;
  ToggleSetting(boolean act){
    active = act;
  }
  bool get(){
    return active;
  }
};
struct SliderSetting{
  int low = 0;
  int high = 1000;
  int value = 500;
  SliderSetting(int val){
    value = val;
  }
  SliderSetting(int l, int h, int val){
    low = l;
    high = h;
    value = val;
  }
  int get(){
    return value;
  }
};

struct SliderScreen : public MenuScreen {
  char* extraText;
  SliderSetting* setting;
  SliderScreen(MenuManager& menuManager, SliderSetting* sett) : MenuScreen(menuManager){
    setting = sett;
    extraText = "";
  }
  SliderScreen(MenuManager& menuManager, SliderSetting* sett, char* extra) : MenuScreen(menuManager){
    setting = sett;
    extraText = extra;
  }
  void displayScreen() override{
    DisplayWrapper disp = manager->getDisplay();
    disp.clear();
    disp.setCursor(0,0);
    disp.printText(1,extraText);
    disp.printRect(4,30,120,4);
    float percent = (setting->value - setting->low) / ((float)(setting->high - setting->low));
    disp.printCircle(4 + (int)round(120 * percent),32,8);
    disp.setCursor(64,44);
    disp.printText(1,(long)setting->value);
    disp.display();
  }
  void onAdvance() override{
    if(setting->value > setting->low){
      setting->value -= 1;
    }
    displayScreen();
  }
  void onSelect() override{
    if(setting->value < setting->high){
      setting->value += 1;
    }
    displayScreen();
  }
};

#endif
