#include "menu.h"

void MenuScreen::displayScreen(){
  DisplayWrapper disp = manager->getDisplay();
  disp.clear();
  disp.setCursor(10,10);
  disp.printText(1,"menu screen");
  disp.display();
}

void NavigationMenuScreen::displayScreen(){
  DisplayWrapper disp = manager->getDisplay();
  disp.clear();
  for(int i=0;i<count;i++){
    disp.setCursor(11,5 + 8*i);
    disp.printText(1,menuItems[i]->getName());
  }
  disp.setCursor(11,5 + 8*count);
  disp.printText(1,"back");
  disp.setCursor(0,0);
  disp.printRect(0,5 + 8*selected,8,6);
  disp.display();
}
