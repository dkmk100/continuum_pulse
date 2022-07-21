#include <Wire.h>               //  Wire.h provides I2C support
#include <Adafruit_GFX.h>       //  Generic graphics library: fonts, lines, effects
#include <Adafruit_SSD1306.h>   //  Library for the micro OLED display
#include <Adafruit_CircuitPlayground.h> 
#include <string>
#include <buildTime.h>

#include "menu.h"
#include "OLED_menu.h"

const unsigned char speaker = A0;
const unsigned char tempSensor = A3;

const unsigned char backPin = A2;
const unsigned char selectPin = A7;
const unsigned char advancePin = A6;

#define builtinMode false

OLED_Display* oled_display = new OLED_Display();
MenuManager screenManager(oled_display);
Adafruit_SSD1306& OLED = oled_display->getOled();

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//change this based on when you are compiling, it sets the default value of the DST setting.
const bool isDaylightSavings = true;

//declare everything used in custom menu types
void displayClock();
void updateOLED(unsigned long ts, String info);
void doCalibration();

struct MenuItem;

float aOffset;
int baseTemp = 700;
unsigned long tStart;
const boolean DEBUG_PRINT = false;
unsigned int nSteps = 0;
unsigned int daySteps = 0;
unsigned long dayStart = 0;
boolean joggingMode = false;
unsigned long joggingTime = 0;
unsigned long long totalSteps = 0;
boolean sleepMode = false;
unsigned long timerTime = 0;
boolean hasTimer = false;

boolean isTwelveHour();
boolean isDaylight();

unsigned long timeToMilli(int hours, int minutes, int seconds);
void milliToTime(unsigned long mili, int& hours, int& minutes, int& seconds);
void printClock(int hours, int minutes, int seconds, int mil, boolean twelveHour);

//uses the build time library to give us the time we compile the code, thus reducing the odds we have to set the clock manually on upload.
//not perfectly accurate but we don't care anyway lol
//used by clocks and stuff
//not constant because clock configuration changes it...
unsigned long startTime = timeToMilli(BUILD_HOUR + (isDaylightSavings ? -1 : 0),BUILD_MIN,BUILD_SEC + 30);

struct TimerScreen : public MenuScreen{
  int selected = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  unsigned long durationTicks = 0;
  TimerScreen(MenuManager& menuManager) : MenuScreen(menuManager){
    
  }
  void showClock(int mil){
    int dispHours = hours;
    OLED.clearDisplay();
    printClock(hours,minutes,seconds,mil,false);
    OLED.fillRect(21 + 37 * selected,45,4,7,SSD1306_WHITE);
    OLED.display();
  }
  boolean customBack() override{
    return true;
  }
  void onBack() override {
    if(!hasTimer && durationTicks > 0){
     timerTime = millis() + durationTicks;
     durationTicks = 0;
     hours = 0;
     minutes = 0;
     seconds = 0;
     hasTimer = true;
    }
    manager->setLast();
  }
  void displayScreen() override{
    //set out time values
    unsigned long dispTime;
    if(hasTimer){
      dispTime = timerTime - millis();
    }  
    else{
      dispTime = durationTicks;
    }
    
    milliToTime(dispTime,hours,minutes,seconds);
    //could be optimized  by using time we just calculated, but would require access to the setting value as well (for 12 vs 24 hour time);
    showClock(dispTime);
  }
  
  void onAdvance() override{
    if(hasTimer){
      return;
    }
    selected+=1;
    if(selected > 2){
      selected = 0;
    }
  }
  void onSelect() override{
    if(hasTimer){
      hasTimer = false;
    }
    switch(selected){
      case 0:
      hours++;
      if(hours > 24){
        hours -=24;
      }
      break;
      case 1:
      minutes++;
      if(minutes > 60){
        minutes -= 60;
      }
      break;
      case 2:
      seconds++;
      if(seconds > 60){
        default: 
        seconds -=60;
      }
      break;
    }
    durationTicks = timeToMilli(hours,minutes,seconds);
    showClock(durationTicks);
  }
};

struct ClockConfigScreen : public MenuScreen{
  int selected = 0;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;
  ClockConfigScreen(MenuManager& menuManager) : MenuScreen(menuManager){
    
  }
  void showClock(int mil){
    int dispHours = hours;
    boolean twelveHour = isTwelveHour();
    if(isDaylight()){
    dispHours+=1;
    }
    OLED.clearDisplay();
    printClock(dispHours,minutes,seconds,mil,twelveHour);
    OLED.fillRect((twelveHour ? 11 : 21) + 37 * selected,45,4,7,SSD1306_WHITE);
    OLED.display();
  }
  void displayScreen() override{
    //set out time values
    unsigned long ourTime = startTime + millis();
    milliToTime(ourTime,hours,minutes,seconds);
    //could be optimized  by using time we just calculated, but would require access to the setting value as well (for 12 vs 24 hour time);
    showClock(ourTime);
  }
  void onAdvance() override{
    selected+=1;
    if(selected > 2){
      selected = 0;
    }
  }
  void onSelect() override{
    switch(selected){
      case 0:
      hours++;
      if(hours > 24){
        hours -=24;
      }
      break;
      case 1:
      minutes++;
      if(minutes > 60){
        minutes -= 60;
      }
      break;
      case 2:
      seconds++;
      if(seconds > 60){
        default: 
        seconds -=60;
      }
      break;
    }
    unsigned long mil = timeToMilli(hours,minutes,seconds);
    startTime = mil - millis();
    showClock(mil);
  }
};

struct BoolMenuItem : public MenuItem{
  private:
  ToggleSetting* setting;
  public:
  BoolMenuItem(MenuManager* mana, ToggleSetting* sett, String myName) : MenuItem(mana, myName){
    setting = sett;
  }
  void activate() override{
    //toggle setting
    setting->active = !(setting->active);
  }
  virtual String getName(){
    String tempName = itemName.substring(0);
    tempName.concat(" ");
    int l = itemName.length();
    if(l > 13){
       l = 13;
    }
    for(int i=0;i< 13 - l;i++){
          tempName.concat("-");
    }
    if(setting->active){
      tempName.concat("- on");
    }
    else{
      tempName.concat(" off");
    }
    return tempName;
  }
};

struct CalibrateMenuItem : public MenuItem{
  void activate() override{
    doCalibration();
  }
   CalibrateMenuItem(MenuManager* mana, String myName) : MenuItem(mana, myName){
    //nothing to do here
  }
};

struct SliderMenuItem : public NavItem{
  SliderSetting* setting;
  SliderMenuItem(MenuManager* mana, SliderScreen* dest, String myName) : NavItem(mana,dest,myName){
    setting = dest->setting;
  }
  virtual String getName(){
    String tempName = itemName.substring(0);
    String numText = String(setting->get());
    tempName.concat(" ");
    int l = itemName.length();
    int ml = 16 - numText.length();
    if(ml < 8){
      ml = 8;
    }
    if(l > ml){
       l = ml;
    }
    for(int i=0;i< ml - l;i++){
          tempName.concat("-");
    }
    tempName.concat(numText);
    return tempName;
  }
};

struct ClockMenuScreen : public MenuScreen{
  MenuScreen* hiddenNav = NULL;
  void displayScreen() override{
    if(!sleepMode){
      displayClock();
    }
  }

  //clock can go to the other screen when anything happens
  void onSelect() override{
    if(hiddenNav!=NULL){
      manager->setScreen(hiddenNav);
      manager->displayScreen();
    }
  }
  void onAdvance() override{
    if(hiddenNav!=NULL){
      manager->setScreen(hiddenNav);
      manager->displayScreen();
    }
  }
  void setNav(MenuScreen* nav){
    hiddenNav = nav;
  }
  ClockMenuScreen(MenuManager& menuManager) : MenuScreen(menuManager){
    
  }
};

//will change to the actual buttons later

#if builtinMode
boolean getBackButton(){
  return false;
}
boolean getAdvanceButton(){
  return CircuitPlayground.rightButton();
}
boolean getSelectButton(){
  return CircuitPlayground.leftButton();
}
#else
boolean getBackButton(){
  return digitalRead(backPin);
}
boolean getAdvanceButton(){
  return digitalRead(advancePin);
}
boolean getSelectButton(){
  return digitalRead(selectPin);
}
#endif
SliderSetting* stepSize = new SliderSetting(20,30,21);
void resetPedometer(){
  if(joggingMode){
    joggingEnd();
  }
  totalSteps += nSteps;
  nSteps = 0;
  tStart = millis();
}
void joggingStart(){
  if(joggingMode){
    screenManager.getDisplay().displayText(1,"already on jogging mode");
    delay(500);
  }
  daySteps = nSteps;
  nSteps = 0;
  joggingMode = true;
  joggingTime = millis();
  dayStart = tStart;
  tStart = 0;
  screenManager.getDisplay().displayText(1,"started jog!");
  delay(500);
}
void clearTimer(){
  hasTimer = false;
}
void joggingEnd(){
  if(!joggingMode){
    screenManager.getDisplay().displayText(1,"no jog to stop");
    delay(500);
  }
  OLED.clearDisplay();
  OLED.setCursor(0,0);
  OLED.setTextSize(1);
  OLED.print("jog ended!\n");
  OLED.print("Distance: ");
  float dist = nSteps * 0.1 * stepSize->get();
  OLED.print(dist);
  OLED.print("\n");
  float dur = millis() - joggingTime;
  float secs = dur/1000.0;
  float fps = dist/secs;
  float mph = fps/1.467;
  OLED.print("average speed: ");
  OLED.print(mph);
  OLED.print("mph");
  OLED.display();
  delay(600);
  bool done = false;
  while(!done){
    if(getBackButton() || getSelectButton() || getAdvanceButton()){
      done = true;
    }
    else{
      delay(10);
    }
  }
  nSteps = daySteps;
  tStart = dayStart;
  dayStart = 0;
  daySteps = 0;
  joggingMode = false;
  joggingTime = millis();
}

//I should just maek these take a func pointer at this point....
struct ResetPedometerItem : public MenuItem{
  void activate() override{
    manager->getDisplay().displayText(1,"resetting pedometer...");
    delay(500);
    resetPedometer();
  }
   ResetPedometerItem(MenuManager* mana, String myName) : MenuItem(mana, myName){
    //nothing to do here
  }
};
struct ClearTimerItem : public MenuItem{
  void activate() override{
    manager->getDisplay().displayText(1,"resetting timer...");
    delay(500);
    clearTimer();
  }
  ClearTimerItem(MenuManager* mana, String myName) : MenuItem(mana, myName){
    //nothing to do here
  }
};
struct JogItem : public MenuItem{
  void activate() override{
    if(joggingMode){
      joggingEnd();
    }
    else{
      joggingStart();
    }
  }
  JogItem(MenuManager* mana, String myName) : MenuItem(mana, myName){
    //nothing to do here
  }
  virtual String getName() override{
    if(joggingMode){
      return String("end jog");
    }
    return String("start jog");
  }
};

struct PedometerMenuScreen : public MenuScreen{
  void displayScreen() override{
    float tNow = float(millis() - tStart) / 1000.0;     //  Time in seconds since the start
     manager->getDisplay().displayTimedText(int(tNow), String(nSteps) + " steps");
  }
  //screens without any navigation should return on select button.
  void onSelect() override{
      manager->setLast();
      manager->displayScreen();
  }
  void onAdvance() override{
    //nothing to do here lol
  }
 PedometerMenuScreen(MenuManager& menuManager) : MenuScreen(menuManager){
  //nothing else to do here
  }
};

struct PedometerAllMenuScreen : public MenuScreen{
  void displayScreen() override{
    float tNow = float(millis() - tStart) / 1000.0;     //  Time in seconds since the start
    manager->getDisplay().displayTimedText((millis()/1000), String((unsigned long)(nSteps + totalSteps)) + F(" steps"));
  }
  //screens without any navigation should return on select button.
  void onSelect() override{
      manager->setLast();
      manager->displayScreen();
  }
  void onAdvance() override{
    //nothing to do here lol
  }
 PedometerAllMenuScreen(MenuManager& menuManager) : MenuScreen(menuManager){
  //nothing else to do here
  }
};

struct DistanceMenuScreen : public MenuScreen{
  void displayScreen() override{
    float tNow = float(millis() - tStart) / 1000.0;     //  Time in seconds since the start
    manager->getDisplay().displayTimedText(int(tNow), String(nSteps * 0.1 * stepSize->get()) + F(" feet"));
  }
  //screens without any navigation should return on select button.
  void onSelect() override{
      manager->setLast();
      manager->displayScreen();
  }
  void onAdvance() override{
    //nothing to do here lol
  }
 DistanceMenuScreen(MenuManager& menuManager) : MenuScreen(menuManager){
  //nothing else to do here
  }
};

ClockMenuScreen* clockScreen = new ClockMenuScreen(screenManager);
NavigationMenuScreen* basicNavScreen = new NavigationMenuScreen(screenManager);

NavigationMenuScreen* clockSettings = new NavigationMenuScreen(screenManager);
NavigationMenuScreen* sensorSettings = new NavigationMenuScreen(screenManager);
NavigationMenuScreen* utilScreen = new NavigationMenuScreen(screenManager);

NavigationMenuScreen* pedometerSettingsScreen = new NavigationMenuScreen(screenManager);

PedometerMenuScreen* stepsScreen = new PedometerMenuScreen(screenManager);
DistanceMenuScreen* distScreen = new DistanceMenuScreen(screenManager);
PedometerAllMenuScreen* totalStepsScreen = new PedometerAllMenuScreen(screenManager);

ClockConfigScreen* clockSetScreen = new ClockConfigScreen(screenManager);
TimerScreen* timerScreen = new TimerScreen(screenManager);

ToggleSetting* twelveHourTime = new ToggleSetting(true);
ToggleSetting* doWaterReminders = new ToggleSetting(true);
ToggleSetting* doTempReminders = new ToggleSetting(true);
ToggleSetting* daylightSavings = new ToggleSetting(isDaylightSavings);

SliderSetting* waterDelay = new SliderSetting(1,12,5);
//step size is above because steps screen

SliderScreen* waterScreen = new SliderScreen(screenManager, waterDelay);// String("water reminder frequency")
SliderScreen* stepScreen = new SliderScreen(screenManager, stepSize);// String("water reminder frequency")

void setup() {
  //basic setup
  Serial.begin(115200);
  CircuitPlayground.begin();
  delay(500);
  oled_display->setup();

  //setup screens
  NavItem* utilDest = new NavItem(&screenManager, utilScreen, String("utilities"));
  NavItem* settingsDest1 = new NavItem(&screenManager, clockSettings, String("clock settings"));
  NavItem* settingsDest2 = new NavItem(&screenManager, sensorSettings, String("sensor settings"));
  NavItem* pedometerDest = new NavItem(&screenManager, stepsScreen, String("pedometer"));
  NavItem* pedSetDest = new NavItem(&screenManager, pedometerSettingsScreen, String("pedometer settings"));

  basicNavScreen->addMenuItem(utilDest);
  basicNavScreen->addMenuItem(pedometerDest);
  basicNavScreen->addMenuItem(pedSetDest);
  basicNavScreen->addMenuItem(settingsDest1);
  basicNavScreen->addMenuItem(settingsDest2);

  BoolMenuItem* twelveHourSetting = new BoolMenuItem(&screenManager, twelveHourTime, String("12h time"));
  BoolMenuItem* daySavingSetting = new BoolMenuItem(&screenManager, daylightSavings, String("DST"));
  BoolMenuItem* waterSetting = new BoolMenuItem(&screenManager, doWaterReminders, String("water remind"));
  BoolMenuItem* tempSetting = new BoolMenuItem(&screenManager, doTempReminders, String("temp alerts"));
  SliderMenuItem* waterDelaySetting = new SliderMenuItem(&screenManager, waterScreen, String("water delay"));
  NavItem* setClockDest = new NavItem(&screenManager, clockSetScreen, String("set clock"));
  //recently split to 2 menus
  clockSettings->addMenuItem(twelveHourSetting);
  clockSettings->addMenuItem(daySavingSetting);
  clockSettings->addMenuItem(setClockDest);

  sensorSettings->addMenuItem(waterSetting);
  sensorSettings->addMenuItem(waterDelaySetting);
  sensorSettings->addMenuItem(tempSetting);

  CalibrateMenuItem* calibrateItem = new CalibrateMenuItem(&screenManager, String("calibrate"));
  SliderMenuItem* stepSizeSetting = new SliderMenuItem(&screenManager, stepScreen, String("step size"));
  NavItem* distDest = new NavItem(&screenManager, distScreen, String("distance"));
  NavItem* totalStepsDest = new NavItem(&screenManager, totalStepsScreen, String("total steps"));
  ResetPedometerItem* resetPedometer = new ResetPedometerItem(&screenManager, String("reset pedometer"));
  
  
  pedometerSettingsScreen->addMenuItem(calibrateItem);
  pedometerSettingsScreen->addMenuItem(distDest);
  pedometerSettingsScreen->addMenuItem(stepSizeSetting);
  pedometerSettingsScreen->addMenuItem(totalStepsDest);
  pedometerSettingsScreen->addMenuItem(resetPedometer);
  
  
  NavItem* timerDest = new NavItem(&screenManager, timerScreen, String("timer"));
  JogItem* jogItem = new JogItem(&screenManager, String("jog"));
  utilScreen->addMenuItem(timerDest);
  utilScreen->addMenuItem(jogItem);

  clockScreen->setNav(basicNavScreen);

  doCalibration();


  //open clock at the end
  screenManager.setScreen(clockScreen);

  //for pedometer
  tStart = millis();
}

void doCalibration(){
  //calibrate step counter
  float aSum;            //  Store accumulated sum during baseline calculation
  int tSum;              //  Store sum of temp sensor data 
  int   nave=350;        //  Number of readings used to obtain the baseline
  int   baseDelay=10;    //  Time in milliseconds to wait between baseline readings
  DisplayWrapper disp = screenManager.getDisplay();
  disp.clear();
  disp.displayText(1,"Preparing to calibrate \nplease hold upright");
  pinMode(tempSensor,INPUT);

  delay(100);

  screenManager.getDisplay().displayText(1,"Calibrating...");
  //Serial.print("Calibrating for ");  Serial.print(nave*baseDelay/1000);  Serial.println(" seconds");

  //for this time, add data to all our sums so we can find averages. That's... the entire point of calibration...
  aSum = 0.0;
  tSum = 0;
  for (int i=1; i<=nave; i++ ) {
    aSum += accelSmooth(0.5,DEBUG_PRINT);
    tSum += analogRead(tempSensor);
    delay(baseDelay);
  }
  
  aOffset = aSum / float(nave);//actual float, so float division is obv necessary
  baseTemp = tSum / float(nave);//do float division to include partial parts, then auto-cast to int which kills decimal point. Close enough.
  screenManager.getDisplay().displayText(1,"Done calibrating!"); 
  delay(500);
}


boolean isTwelveHour(){
  return twelveHourTime->get();
}
boolean isDaylight(){
  return daylightSavings->get();
}

int getWaterDelay(){
  int raw = waterDelay->get();
  int last = raw*raw*raw * 8 + 10;//extra 10 secs so low numbers are better
  return last;
}

void loop() {
   //low delay so everything is accurate
  const float delayDur = 10;
  static int backTicks = 0;
  const float timeTillSleep = 10;
  static unsigned int lastMove = tStart;
  static unsigned int lastWater = millis();
  const float timeToTempAlert = 3 * 1000/delayDur;
  static int ticksHot = 0; 
  static int ticksCold = 0;
  const int hotLevel = 70;
  const int coldLevel = 22;
  int tempRead = analogRead(tempSensor);
  
  float aStepThreshold = 1.1;
  //button setup:
  static bool backWasPressed = false;
  static bool advanceWasPressed = false;
  static bool selectWasPressed = false;
  
  bool backPressed = getBackButton();
  bool advancePressed = getAdvanceButton();
  bool selectPressed = getSelectButton();
  
  bool backTapped = false;
  bool advanceTapped = false;
  bool selectTapped = false;
  if(backPressed && !backWasPressed){
    backTapped = true;
  }
  if(advancePressed && !advanceWasPressed){
    advanceTapped = true;
  }
  if(selectPressed && !selectWasPressed){
    selectTapped = true;
  }
  backWasPressed = backPressed;
  advanceWasPressed = advancePressed;
  selectWasPressed = selectPressed;

  if(millis() > lastWater + getWaterDelay() * 1000 && doWaterReminders->active){
    doWaterReminder();
    lastWater = millis();
    lastMove = millis();
    sleepMode = false;
  }
  if(hasTimer && millis() > timerTime){
    displayAlert("timer!");
    hasTimer = false;
  }
  Serial.println(doTempReminders->get());
  if(tempRead > baseTemp + hotLevel && doTempReminders->get()){
    ticksHot++;
    ticksCold = 0;
  }
  else if(tempRead < baseTemp - coldLevel && doTempReminders->get()){
    ticksCold++;
    ticksHot = 0;
  }
  else{
    ticksHot = 0;
    ticksCold = 0;
  }

  if(ticksHot > timeToTempAlert){
    displayAlert("too hot!");
    ticksHot = timeToTempAlert/2;
  }
  if(ticksCold > timeToTempAlert){
    displayAlert("too cold!");
    ticksCold = timeToTempAlert/2;
  }
  

  static float lastCount = 0;
  const float countTime = 0.1;
  
  //count steps
  float tNow = float(millis() - tStart) / 1000.0;     //  Time in seconds since the start
  if(tNow > lastCount + countTime){
  float a = accelSmooth(0.6,DEBUG_PRINT) - aOffset;   //  Total acceleration relative to stationary
  nSteps += count_step(a, aStepThreshold);      //  Add to step count if a step was detected
  lastCount = tNow; //continue counting steps
  }
  
  if(sleepMode){
    float sleepA = awakeSmooth(1.0,DEBUG_PRINT) - aOffset;   //  Total acceleration relative to stationary    
    
    if(sleepA > 9.0 && millis() > lastMove + timeTillSleep*1000 + 2000){
      lastMove = millis();
      sleepMode = false;
    }
      if(backPressed || advancePressed || selectPressed){
        lastMove = millis();
        sleepMode = false;
      }
  }
  else{
    float sleepA = awakeSmooth(1.0,DEBUG_PRINT) - aOffset;   //  Total acceleration relative to stationary    
    if(sleepA > 9.0){
      lastMove = millis();
      sleepMode = false;
    }
  if(backTapped){
    screenManager.back();
    backTicks = 0;
    lastMove = millis();//keep watch awake
  }
  if(advancePressed){
    backTicks++;
    if(backTicks > 20 && builtinMode){
      screenManager.back();
      backTicks = 0;
      lastMove = millis();//keep watch awake
    }
    else if(advanceTapped){
      screenManager.advance();
      lastMove = millis();//keep watch awake
    }
  }
  else{
    backTicks = 0;
  }
  if(selectTapped){
    screenManager.select();
    lastMove = millis();//keep watch awake
  }

  //the screen manager
  screenManager.displayScreen();


  if(millis() > lastMove + timeTillSleep*1000){
    sleepMode = true;
    OLED.clearDisplay();
    OLED.display();
  }
  }
  delay(delayDur);
}

void doWaterReminder(){
  displayAlert("Drink water!");
}

void displayAlert(char* alert){
  screenManager.getDisplay().displayText(2,alert);
  bool awoke = false;
  while(!awoke){
    makeTone(speaker,150,25);
    if(getBackButton() || getAdvanceButton() || getSelectButton()){
      awoke = true;
    }
    delay(100);
  }
}

//calculates the milliseconds from a time
unsigned long timeToMilli(int hours, int minutes, int seconds){
  if(hours < 0){
    hours+=24;
  }
  unsigned long mil = seconds * 1000;
  mil += minutes * 1000 * 60;
  mil += hours * 1000 * 3600;
  return mil;
}

//uses var references to return the time for a given milliseconds. 
void milliToTime(unsigned long mili, int& hours, int& minutes, int& seconds){
  int totalSec = mili / 1000;
  //remove days and above
  totalSec = totalSec % (3600 * 24);

  hours = totalSec / 3600;
  totalSec = totalSec % 3600;
  minutes = totalSec / 60;
  totalSec = totalSec % 60;
  seconds = totalSec;
}

//quick display shortcut to use the correct settings
void displayClock(){
  displayClock(twelveHourTime->active);
}

//displays the clock using calculated time.
void displayClock(boolean twelveHour){
  unsigned long mil = millis();
  unsigned long ourTime = startTime + mil;
  int hours = 0;
  int minutes = 0;
  int seconds = 0;

  milliToTime(ourTime,hours,minutes,seconds);

  if(daylightSavings->get()){
    hours+=1;
  }

  OLED.clearDisplay();
  printClock(hours,minutes,seconds,mil,twelveHour);
  OLED.display();
}

//displays a clock with correct formatting options
void printClock(int hours, int minutes, int seconds, int mil, boolean twelveHour){
  boolean pm = hours > 12;
  int mildiv = mil % 1000;
  if(pm && twelveHour){
    hours -= 12;
  }
  if(twelveHour){
  OLED.setCursor(2,24);
  }
  else{
    OLED.setCursor(14,24);
  }
  OLED.setTextSize(2);
  if(hours < 10){
    OLED.print("0");
  }
  OLED.print(hours);
  if(mildiv < 500){
  OLED.print(":");
  }
  else{
  OLED.print(" ");
  }
  if(minutes < 10){
    OLED.print("0");
  }
  OLED.print(minutes);
  if(mildiv < 500){
  OLED.print(":");
  }
  else{
  OLED.print(" ");
  }
  if(seconds < 10){
    OLED.print("0");
  }
  OLED.print(seconds);
  if(twelveHour){
  if(hours == 12 || pm){
    OLED.print("pm");
  }
  else{
    OLED.print("am");
  }
  }
  OLED.setTextSize(1);
}

// ------------------------------------------------------------------------------------
//  Given the current acceleration and a threshold value for acceleration return 1 if
//  a step was likely (according to the decision rules) or 0 otherwise.  The threshold
//  value is determined by experimentation.  A good start for the threshold is about 1 m/s^2
//
//  Gerald Recktenwald, gerry@pdx.edu,  2022-06-16
int count_step(float a, float threshold) {

  int n;                            //  Local variable, number of steps detected:  1 or 0
  static boolean lastWasLow=true;   //  Was the last acceleration level was above the threshold?

  n = 0;
  if ( a>threshold  ) {      //  A step occurred if accleration was above the threshold
    if ( lastWasLow ) {      //  AND if the last acceleration value was NOT above the threshold
      n = 1;
    }
    lastWasLow = false;      //  Threshold was crossed, so last step was not low
  } else {
    lastWasLow = true;       //  Threshold was not crossed, so last step was low
  }
  return(n);
}

// ------------------------------------------------------------------------------------
//  Make a new measurement of total acceleration and return the exponentially-weighted
//  average of this measurement and recent measurements.
//  The smoothing acts as a low-pass filter to reduce high frequency noise.
//  The smoothing parameter is alfa:  0 < alfa <= 1.0.  When alfa = 1, no smoothing
//  is applied -- the most recent reading of total acceleration is returned.
//  As alfa is reduced toward zero more smoothing is applied. You will need to
//  experiment to determine a value of alfa that works with the acceleration you
//  are measuring.  A value in the range 0.3 < alfa < 0.6 is a usually first guess.
//
//  Gerald Recktenwald, gerry@pdx.edu,  2022-06-16
float accelSmooth(float alfa, boolean printDetail) {
  float ax, ay, az, aTot, aTotSmooth;   //  acceleration components and magnitudes
  static float aTotOld=-99.0;           //  Value retained for the next call to accelSmooth

  // -- Retrieve acceleration components and compute the total
  ax = CircuitPlayground.motionX();
  ay = CircuitPlayground.motionY();
  az = CircuitPlayground.motionZ();
  aTot = sqrt(ax*ax + ay*ay + az*az);         //  could also use pow(ax,2), but ax*ax is faster

  // -- Use exponential smoothing of the total acceleration value (magnitude of acceleration vector).
  //    The "if" statement tests whether this is the first measurement.  aTotOld is initialized
  //    to -99 because for the very first measurement, there is no true "old" value, and
  //    we simply use the current reading.  This test avoids causing the exponential average to
  //    include a bogus intial reading.  Without this test the first few values returned by
  //    this (accelSmooth) function would include a reading of zero, which would cause
  //    the trend in values to start at an artificially low value.
  if ( aTotOld<0.0 ) {
    aTotSmooth = aTot;
  } else {
    aTotSmooth = aTot*alfa + (1-alfa)*aTotOld;
  }
  aTotOld = aTotSmooth;  //  Save for next measurement

  // -- Print acceleration values without extra text so that Serial Plotter can be used
  //    When everything is working, there is no need to print these details
  if ( printDetail ) {
    Serial.print(ax);
    Serial.print("\t");   Serial.print(ay);
    Serial.print("\t");   Serial.print(az);  
    Serial.print("\t");   Serial.print(aTot);
    Serial.print("\t");   Serial.println(aTotSmooth);
  }

  return(aTotSmooth);
}

//basically the same thing as the above function, but with a different aTotOld to track disabling sleep mode instead of a step.
//TERRIBLE way to organize it, these should be pass-by-reference variables, but not time for futher refactoring...
//
// Copied from the original by Gerald Recktenwald, gerry@pdx.edu,  2022-06-16
float awakeSmooth(float alfa, boolean printDetail) {
  float ax, ay, az, aTot, aTotSmooth;   //  acceleration components and magnitudes
  static float aTotOld=-99.0;           //  Value retained for the next call to accelSmooth

  ax = CircuitPlayground.motionX();
  ay = CircuitPlayground.motionY();
  az = CircuitPlayground.motionZ();
  aTot = sqrt(ax*ax + ay*ay + az*az);         //  could also use pow(ax,2), but ax*ax is faster

  if ( aTotOld<0.0 ) {
    aTotSmooth = aTot;
  } else {
    aTotSmooth = aTot*alfa + (1-alfa)*aTotOld;
  }
  aTotOld = aTotSmooth;  //  Save for next measurement

  if ( printDetail ) {
    Serial.print(ax);
    Serial.print("\t");   Serial.print(ay);
    Serial.print("\t");   Serial.print(az);  
    Serial.print("\t");   Serial.print(aTot);
    Serial.print("\t");   Serial.println(aTotSmooth);
  }
  return(aTotSmooth);
}

// ------------------------------------------------------------------------------------
// makes a tone using a speaker pin
void makeTone (unsigned char speakerPin, int frequencyInHertz, long timeInMilliseconds) {
  int x;   
  long delayAmount = (long)(1000000/frequencyInHertz);
  long loopTime = (long)((timeInMilliseconds*1000)/(delayAmount*2));
  for (x=0; x<loopTime; x++) {        // the wave will be symetrical (same time high & low)
     digitalWrite(speakerPin,HIGH);   // Set the pin high
     delayMicroseconds(delayAmount);  // and make the tall part of the wave
     digitalWrite(speakerPin,LOW);    // switch the pin back to low
     delayMicroseconds(delayAmount);  // and make the bottom part of the wave
  }  
}
