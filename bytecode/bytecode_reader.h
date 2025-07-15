#include <cstddef>
#include "opcodes.h"

#include "Arduino.h"

class InStream {
private:
  char* buff;
  int len;
  int count;
  std::size_t (*readFunc)(char*, std::size_t);
  inline bool fillMissing() {
    if (count == len) {
      return true;
    }
    int totalRead = (*readFunc)(buff + count, len - count);
    if (totalRead >= 0) {
      count = count + totalRead;
      return count > 0;
    } else {
      return false;
    }
  }
public:
  InStream(int len, std::size_t (*readFunc)(char*, std::size_t)) {
    this->readFunc = readFunc;
    this->len = len;
    this->buff = new char[len];
    this->count = 0;
  }
  ~InStream() {
    delete this->buff;
  }
  inline bool advance(int amount) {
    fillMissing();
    while (amount > 0) {
      //decide how much to move at once
      int shift = amount;
      if (shift > count) {
        shift = count;
      }
      //move forward
      for (int i = 0; i < count; i++) {
        buff[i] = buff[i + shift];
      }
      count -= shift;
      //subtract amount
      amount -= shift;
      fillMissing();
    }
    return count > 0;
  }
  inline int getCount() {
    return count;
  }
  inline char* read() {
    return buff;
  }

  inline bool valid() {
    return count > 0;
  }
  inline char readChar() {
    if (count < 1) {
      fillMissing();
    }
    char x = *buff;
    advance(1);
    return x;
  }
  inline short readShort() {
    if (count < 2) {
      fillMissing();
    }
    short s = *((short*)buff);
    advance(2);
    return s;
  }
  inline int readInt() {
    if (count < 4) {
      fillMissing();
    }
    int s = *((int*)buff);
    advance(4);
    return s;
  }
  inline char* allocReadStr() {
    short strLen = readShort();
    char* str = new char[strLen];
    int wrote = 0;
    fillMissing();
    while (wrote < strLen) {
      int next = strLen - wrote;
      if (next > count) {
        next = count;
      }
      for(int i=0;i<next;i++){
        str[wrote+i] = buff[i];
      }
      wrote += next;
      advance(next);
    }
    return str;
  }
};

class BytecodeReader {
private:
  bool eq(const char* a, const char* b){
    while(*a != 0 && *b != 0){
      if(*a != *b){
        return false;
      }
      a += 1;
      b += 1;
    }
    return *a == *b;
  }
public:
  inline bool readCode(BytecodeProgram& program, InStream& stream) {
    //setup stream for reading
    stream.advance(0);

    //skip a character due to bug in the compiler
    stream.advance(1);

    char magic = stream.readChar();
    Serial.println((int)magic);
    if (magic != 42) {
      return false;
    }
    char* str = stream.allocReadStr();
    Serial.println(str);
    if(!eq(str, "toyir")){
      return false;
    }
    char version = stream.readChar();

    return false;
  }
};
