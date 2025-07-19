#include <Adafruit_TinyUSB.h>

#include <Adafruit_CircuitPlayground.h>

#include "opcodes.h"
#include "interpreter.h"
#include "code.h"
#include "bytecode_reader.h"

#include "SPI.h"
#include "SdFat_Adafruit_Fork.h"
#include "Adafruit_SPIFlash.h"

// for flashTransport definition
#include "flash_config.h"

#include "Arduino.h"

Adafruit_SPIFlash flash(&flashTransport);

// file system object from SdFat
FatVolume fatfs;

FatFile root;
FatFile file;

// USB Mass Storage object
Adafruit_USBD_MSC usb_msc;

// Check if flash is formatted
bool fs_formatted = false;

// Set to true when PC write to flash
bool fs_changed = true;

// Time to resume running interpreter
long resumeTime;

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

void printFreeMemory() {
  unsigned int bytes = freeMemory();
  Serial.print(bytes / (float)1024);
  Serial.println("kb");
}

int leftButton(int* dat) {
  return CircuitPlayground.leftButton();
  //return false;
}

int rightButton(int* dat) {
  return CircuitPlayground.rightButton();
  //return false;
}

int setPixelColor(int* dat) {
  CircuitPlayground.setPixelColor(dat[0], dat[1], dat[2], dat[3]);
  /*
  for (int i = 0; i < 4; i++) {
    Serial.print(dat[0]);
  }
  Serial.println();
  */
  return 0;
}



int doDelay(int* dat) {
  //delay(dat[0]);
  //don't use builtin delays
  resumeTime = millis() + dat[0];
  return 0;
}

int numBuiltins = 3;
BuiltinFunc builtins[] = {
  { PrimitiveType::BOOL, 0, nullptr, &leftButton },
  { PrimitiveType::BOOL, 0, nullptr, &rightButton },
  { PrimitiveType::VOID, 4, (PrimitiveType[]){ PrimitiveType::INT, PrimitiveType::INT, PrimitiveType::INT, PrimitiveType::INT }, &setPixelColor },
  { PrimitiveType::VOID, 4, (PrimitiveType[]){ PrimitiveType::INT }, &doDelay },
};

void print(const char* str) {
  Serial.print(str);
  delay(20);
}

Interpreter interpreter(builtins, numBuiltins);
BytecodeProgram program;
bool programLoaded = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.print("Pre-init free memory: ");
  printFreeMemory();

  CircuitPlayground.begin();

  Serial.println(F("begin flash initialization"));
  delay(200);

  flash.begin();

  Serial.println(F("begin usb initialization"));
  delay(200);

  // Set disk vendor id, product id and revision with string up to 8, 16, 4 characters respectively
  usb_msc.setID("Adafruit", "External Flash", "1.0");

  // Set callback
  usb_msc.setReadWriteCallback(msc_read_cb, msc_write_cb, msc_flush_cb);

  // Set disk size, block size should be 512 regardless of spi flash page size
  usb_msc.setCapacity(flash.size() / 512, 512);

  // MSC is ready for read/write
  usb_msc.setUnitReady(true);

  Serial.println(F("begin usb mount"));
  Serial.flush();

  Serial.end();
  usb_msc.begin();  //important part
  Serial.begin(115200);
  while (!Serial) {
    //CircuitPlayground.setPixelColor(0, 255, 0, 0);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    //CircuitPlayground.setPixelColor(0, 0, 0, 0);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }

  Serial.println(F("begin usb remount"));
  Serial.flush();

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    Serial.println("full remount required");
    Serial.flush();
    delay(50);
    TinyUSBDevice.detach();
    delay(50);
    TinyUSBDevice.attach();
  } else {
    TinyUSBDevice.attach();
  }

  // Reconnect Serial to ensure communcation works
  while (!Serial) {
    //CircuitPlayground.setPixelColor(0, 255, 0, 0);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    //CircuitPlayground.setPixelColor(0, 0, 0, 0);
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
  }

  Serial.println(F("begin file system mount"));
  delay(200);

  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);

  Serial.println(F("file system mounted"));

  Serial.print("Post-init free memory: ");
  printFreeMemory();

  resumeTime = millis();

  ///*
  Serial.println("begin file stuff");
  root.open("/");
  fileStuff();
  root.close();
  Serial.println("end file stuff");
  //*/
}

void LoadDefaultProgram(BytecodeProgram& program) {
  Serial.println(F("Loading default program..."));
  CreateDefaultProgram(program);
  Serial.println(F("Default program loaded"));
  Serial.print("Free memory: ");
  printFreeMemory();
  programLoaded = true;
}

bool LoadProgram(BytecodeProgram& program, InStream& stream) {
  Serial.println("Loading code file...");
  BytecodeReader reader;
  bool success = reader.readCode(program, stream);
  Serial.println("Loaded code file");
  Serial.print("Free memory: ");
  printFreeMemory();
  programLoaded = success;
  return success;
}

void RunProgram(const BytecodeProgram& program) {
  Serial.println(F("Initializing interpreter..."));
  interpreter.begin(program);
  Serial.print("functions count: ");
  Serial.println(program.funcsCount);
  Serial.print("entrypoint: ");
  Serial.println(program.entryPoint()->name);
  Serial.print("Free memory: ");
  printFreeMemory();
  Serial.println(F("Running program..."));
  Serial.println(F("============================="));
  delay(100);
}

std::size_t readFile(char* buff, std::size_t count) {
  return file.read(buff, count);
}

void fileStuff() {
  while (file.openNext(&root, O_RDONLY)) {
    if (!file.isDir()) {
      const int len = 16;
      char name[len];
      file.getName(name, len);
      Serial.println(name);
      int end = 0;
      for (int i = 0; i < len; i++) {
        if (name[i] == 0) {
          end = i - 1;
          break;
        }
      }
      if (end >= 3 && name[end - 3] == '.' && name[end - 2] == 'd' && name[end - 1] == 'a' && name[end] == 't') {
        Serial.print("found possible code file: ");
        Serial.println(name);

        InStream stream(20, *readFile);
        bool loaded = LoadProgram(program, stream);
      }
    }
    file.close();
  }
}

void loop() {
  if (!fs_formatted) {
    fs_formatted = fatfs.begin(&flash);

    if (!fs_formatted) {
      Serial.println(F("Failed to init files system, flash may not be formatted"));
      Serial.println();

      delay(1000);
      return;
    }
  }

  if (fs_changed) {
    fs_changed = false;
    Serial.println("file system changed");
  }

  /*
  if (!programLoaded && !interpreter.ready()) {
    if (!root.open("/")) {
      //Serial.println("open root failed");
    } else {
      //fileStuff();
      //Serial.println("open root");
      root.close();
    }
    delay(1000);
  }
  //*/

  ///*
  //Serial.println("pre interpreter");

  if (programLoaded && CircuitPlayground.leftButton()) {
    programLoaded = false;
    RunProgram(program);
  }

  if (interpreter.ready()) {
    //Serial.println("begin interpreter brust");
    long startTime = millis();
    long timeout = 1000;
    while (millis() - timeout < startTime && millis() >= resumeTime && interpreter.ready()) {
      interpreter.step(&print, false, false);
    }
    /*
    if (interpreter.ready()) {
      Serial.println("end interpreter burst");
    } else {
      Serial.println("interpreter halted!");
    }
    //*/
  }

  //*/
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and
// return number of copied bytes (must be multiple of block size)
int32_t msc_read_cb(uint32_t lba, void* buffer, uint32_t bufsize) {
  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.readBlocks(lba, (uint8_t*)buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and
// return number of written bytes (must be multiple of block size)
int32_t msc_write_cb(uint32_t lba, uint8_t* buffer, uint32_t bufsize) {
#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, HIGH);
#endif

  // Note: SPIFLash Block API: readBlocks/writeBlocks/syncBlocks
  // already include 4K sector caching internally. We don't need to cache it, yahhhh!!
  return flash.writeBlocks(lba, buffer, bufsize / 512) ? bufsize : -1;
}

// Callback invoked when WRITE10 command is completed (status received and accepted by host).
// used to flush any pending cache.
void msc_flush_cb(void) {
  // sync with flash
  flash.syncBlocks();

  // clear file system's cache to force refresh
  fatfs.cacheClear();

  fs_changed = true;

#ifdef LED_BUILTIN
  digitalWrite(LED_BUILTIN, LOW);
#endif
}
