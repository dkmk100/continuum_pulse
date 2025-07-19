#include "opcodes.h"
#include "interpreter.h"
#include <Adafruit_CircuitPlayground.h>

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else   // __ARM__
extern char *__brkval;
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

void setup() {
  // put your setup code here, to run once:
  CircuitPlayground.begin();
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.println(F("Creating interpreter..."));
  Interpreter interpreter(nullptr, 0);
}

void loop() {
  char y;
  Serial.print(((unsigned int)&y) / 8);
  Serial.print(", ");
  Serial.println((freeMemory()) / 8);
}
