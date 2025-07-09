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
  int bytes = freeMemory();
  Serial.print(bytes / (float)1024);
  Serial.println("kb");
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

void print(String str) {
  Serial.print(str.c_str());
  delay(20);
}

Interpreter interpreter(builtins, numBuiltins);
BytecodeProgram program;
bool programLoaded = false;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

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
  delay(200);

  usb_msc.begin();

  Serial.println(F("begin usb remount"));
  delay(200);

  // If already enumerated, additional class driverr begin() e.g msc, hid, midi won't take effect until re-enumeration
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(25);
    TinyUSBDevice.attach();
  }

  // Reconnect Serial to ensure communcation works
  Serial.begin(9600);
  while (!Serial) {
    CircuitPlayground.setPixelColor(0, 255, 0, 0);
    delay(100);
    CircuitPlayground.setPixelColor(0, 0, 0, 0);
    delay(100);
  }

  Serial.println(F("begin file system mount"));
  delay(200);

  // Init file system on the flash
  fs_formatted = fatfs.begin(&flash);

  Serial.println(F("file system mounted"));

  Serial.print("Post-init free memory: ");
  printFreeMemory();

  resumeTime = millis();

  Serial.println("begin file stuff");
  root.open("/");
  fileStuff();
  root.close();
  Serial.println("end file stuff");
}

void LoadProgram(BytecodeProgram& program) {
  Serial.println(F("Loading program..."));
  CreateDefaultProgram(program);
  Serial.println(F("Program loaded"));
  Serial.print("Free memory: ");
  printFreeMemory();
  programLoaded = true;
}

void RunProgram(const BytecodeProgram& program) {
  Serial.println(F("Initializing interpreter..."));
  interpreter.begin(program);
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
        InStream stream(10, &readFile);
        int num = 10;
        while (stream.advance(num)) {
          char buff[11];
          char* ch = stream.read();
          num = stream.getCount();
          for (int i = 0; i < num; i++) {
            buff[i] = ch[i];
            if (buff[i] == 0) {
              buff[i] = 126;
            }
          }
          buff[num] = 0;
          Serial.print(buff);
          Serial.println();
          delay(1000);
        }
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
  }

  if (!programLoaded) {
    if (!root.open("/")) {
      //Serial.println("open root failed");
    } else {
      //fileStuff();
      root.close();
    }
  }

  long startTime = millis();
  if (startTime < resumeTime) {
    return;
  }
  /*
  while (startTime - 10 < millis() && interpreter.ready()) {
    interpreter.step(&print, false, false);
  }
  */
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
