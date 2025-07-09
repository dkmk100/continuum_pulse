#ifndef menu_hpp
#define menu_hpp
#include "Arduino.h"

#include "display_wrapper.h"

enum class MenuButton{
  BACK, LEFT, RIGHT, SELECT
};

struct MenuButtonsState{
  bool tapped[4];
  bool pressed[4];
  bool supported[4];
};

struct MenuManager;

//a screen for the menu manager. Abstract, please extend to use. For a screen with menu items, see NavigationScreen.
struct MenuScreen{
  MenuManager* manager;
  String screenName = "test";

  virtual void tick(MenuButtonsState state) = 0;
  virtual void render(Display& display) = 0;
  
  MenuScreen(MenuManager& menuManager){
    manager = &menuManager;
  }
};

/*
//the menu manager class keeps track of screens open, as well as 
class MenuManager {
private:
  const int maxStack;
  MenuScreen* stack[maxStack];
  int count;
  int selected = 0;
  Display& display;
public:
  MenuManager(Display& disp){
    count = 0;
    display = disp;
  }
  ~MenuManager(){
    
  }
  inline Display& getDisplay(){
    return display;
  }
  void setScreen(MenuScreen& screen){
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
*/

/*

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
*/

/*
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

struct SliderMenuItem : public NavItem {
  SliderSetting* setting;
  SliderMenuItem(MenuManager* mana, SliderScreen* dest, String myName)
    : NavItem(mana, dest, myName) {
    setting = dest->setting;
  }
  virtual String getName() {
    String tempName = itemName.substring(0);
    String numText = String(setting->get());
    tempName.concat(" ");
    int l = itemName.length();
    int ml = 16 - numText.length();
    if (ml < 8) {
      ml = 8;
    }
    if (l > ml) {
      l = ml;
    }
    for (int i = 0; i < ml - l; i++) {
      tempName.concat("-");
    }
    tempName.concat(numText);
    return tempName;
  }
};

struct BoolMenuItem : public MenuItem {
private:
  ToggleSetting* setting;
public:
  BoolMenuItem(MenuManager* mana, ToggleSetting* sett, String myName)
    : MenuItem(mana, myName) {
    setting = sett;
  }
  void activate() override {
    //toggle setting
    setting->active = !(setting->active);
  }
  virtual String getName() {
    String tempName = itemName.substring(0);
    tempName.concat(" ");
    int l = itemName.length();
    if (l > 13) {
      l = 13;
    }
    for (int i = 0; i < 13 - l; i++) {
      tempName.concat("-");
    }
    if (setting->active) {
      tempName.concat("- on");
    } else {
      tempName.concat(" off");
    }
    return tempName;
  }
};
*/
#endif
