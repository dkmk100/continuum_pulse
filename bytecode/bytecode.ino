#include <Adafruit_CircuitPlayground.h>

#include "opcodes.h"
#include "interpreter.h"
#include "code.h"

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else   // __ARM__
extern char* __brkval;
#endif  // __arm__

unsigned int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else   // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

int leftButton(int* dat) {
  return CircuitPlayground.leftButton();
}

int rightButton(int* dat) {
  return CircuitPlayground.rightButton();
}

int setPixelColor(int* dat) {
  CircuitPlayground.setPixelColor(dat[0], dat[1], dat[2], dat[3]);
  return 0;
}

int doDelay(int* dat){
  delay(dat[0]);
  return 0;
}

int numBuiltins = 3;
BuiltinFunc builtins[] = {
  { PrimitiveType::BOOL, 0, nullptr, &leftButton },
  { PrimitiveType::BOOL, 0, nullptr, &rightButton },
  { PrimitiveType::VOID, 4, (PrimitiveType[]){ PrimitiveType::INT, PrimitiveType::INT, PrimitiveType::INT, PrimitiveType::INT }, &setPixelColor },
  { PrimitiveType::VOID, 4, (PrimitiveType[]){ PrimitiveType::INT}, &doDelay },
};

void print(String str) {
  Serial.print(str.c_str());
  delay(20);
}

Interpreter interpreter(builtins, numBuiltins);

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println("Loading program...");
  BytecodeProgram program;
  CreateDefaultProgram(program);
  Serial.println("Running program...");
  Serial.println("=============================");
  delay(100);
  interpreter.run(program, &print, false, false);
  Serial.println("=============================");
  Serial.println("Ran program");
}

void loop() {
}
