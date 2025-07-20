#include <cstddef>
#include "opcodes.h"
#include "Arduino.h"
#include <cstring>

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
    short s = (short)buff[1] + ((short)buff[0] << 8);
    advance(2);
    return s;
  }
  inline int readInt() {
    if (count < 4) {
      fillMissing();
    }

    int s = (int)buff[3] + ((int)buff[2] << 8) + ((int)buff[1] << 16) + ((int)buff[0] << 24);
    advance(4);
    return s;
  }
  inline char* allocReadStr() {
    short strLen = readShort();

    char* str = new char[strLen+1];
    int wrote = 0;
    fillMissing();
    while (wrote < strLen) {
      int next = strLen - wrote;
      if (next > count) {
        next = count;
      }
      for (int i = 0; i < next; i++) {
        str[wrote + i] = buff[i];
      }
      wrote += next;
      advance(next);
    }

    //null terminator
    str[strLen] = '\0';

    return str;
  }
};

class BytecodeReader {
private:
  bool eq(const char* a, const char* b) {
    while (*a != 0 && *b != 0) {
      if (*a != *b) {
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

    char magic = stream.readChar();
    Serial.print("Magic number: ");
    Serial.println((int)magic);
    if (magic != 42) {
      return false;
    }
    char* str = stream.allocReadStr();
    Serial.print("Magic string: ");
    Serial.println(str);
    if (!eq(str, "toyir")) {
      return false;
    }

    char fVersion = stream.readChar();
    Serial.print("Bytecode format version: ");
    Serial.println((int)fVersion);
    if (fVersion != 0) {
      return false;
    }

    short bVersion = stream.readShort();
    Serial.print("Bytecode instruction version: ");
    Serial.println((int)bVersion);
    if (bVersion != 0) {
      return false;
    }

    int funcsCount = stream.readInt();
    Serial.print("num funcs: ");
    Serial.println(funcsCount);

    BytecodeFunc* funcs = new BytecodeFunc[funcsCount];
    for (int i = 0; i < funcsCount; i++) {
      Serial.print("loading function: ");
      funcs[i].name = stream.allocReadStr();

      Serial.println(funcs[i].name);
      Serial.flush();

      funcs[i].args = (int)stream.readChar();
      int debugFormat = (int)stream.readChar();
      int codeLen = stream.readInt();
      Serial.print("instruction count: ");
      Serial.println(codeLen);
      funcs[i].codeLen = codeLen;
      BytecodeInst* code = new BytecodeInst[codeLen];
      for (int j = 0; j < codeLen; j++) {
        code[j].opCode = (OpCodeI)stream.readShort();
        code[j].type = (PrimitiveType)stream.readShort();
        code[j].num1 = stream.readInt();
        code[j].num2 = stream.readInt();
        code[j].num3 = stream.readInt();
        
        Serial.print((int)code[j].opCode);
        Serial.print("\t");
        Serial.print((int)code[j].type);
        Serial.print("\t");
        Serial.print((int)code[j].num1);
        Serial.print("\t");
        Serial.print((int)code[j].num2);
        Serial.print("\t");
        Serial.print((int)code[j].num3);
        Serial.println();
      }
      funcs[i].code = code;

      Serial.println("end function");
      Serial.flush();
    }

    int funcTargetsCount = stream.readInt();
    Serial.print("num func targets: ");
    Serial.println(funcTargetsCount);
    const char** funcTargets = new const char*[funcTargetsCount];
    for(int i=0;i<funcTargetsCount;i++){
      const char* str = stream.allocReadStr();
      Serial.println(str);
      Serial.flush();
      funcTargets[i] = str;
    }
    Serial.flush();

    int stringsCount = stream.readInt();
    Serial.print("num strings: ");
    Serial.println(stringsCount);
    const char** strings = new const char*[stringsCount];
    for(int i=0;i<stringsCount;i++){
      const char* str = stream.allocReadStr();
      Serial.println(str);
      Serial.flush();
      strings[i] = str;
    }
    Serial.flush();

    program.dynamic = true;
    program.funcsCount = funcsCount;
    program.funcs = funcs;
    program.funcTargetsCount = funcTargetsCount;
    program.funcTargets = funcTargets;
    program.stringsCount = stringsCount;
    program.strings = strings;
    program.init();

    return true;
  }
};
