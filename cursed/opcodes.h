#ifndef OPCODES_HPP
#define OPCODES_HPP
#include <Arduino.h>

enum class OpCodeI : unsigned char;

enum class PrimitiveType : unsigned char {
  NONE = 0
};

struct BytecodeInst {
  OpCodeI opCode;
  PrimitiveType type;
  int num1;
  int num2;
  int num3;
  BytecodeInst(OpCodeI opCode, PrimitiveType type, int num1 = -1, int num2 = -1, int num3 = -1){
    this->opCode = opCode;
    this->type = type;
    this->num1 = num1;
    this->num2 = num2;
    this->num3 = num3;
  }
  BytecodeInst(OpCodeI opCode, int num1 = -1, int num2 = -1, int num3 = -1){
    this->opCode = opCode;
    this->type = PrimitiveType::NONE;
    this->num1 = num1;
    this->num2 = num2;
    this->num3 = num3;
  }
};

struct BytecodeFunc {
  int codeLen;
  BytecodeInst* code;
  int args;
  String name;
  BytecodeFunc(int codeLen, BytecodeInst* code, int args, String name) {
    this->codeLen = codeLen;
    this->code = code;
    this->args = args;
    this->name = name;
  }
};

class BytecodeProgram {
public:
  int funcsCount;
  BytecodeFunc* funcs;
  int funcTargetsCount;
  String* funcTargets;
  int stringsCount;
  String* strings;

  BytecodeProgram(){
    
  }
  BytecodeProgram(int funcsCount, BytecodeFunc* funcs, int funcTargetsCount, String* funcTargets, int stringsCount, String* strings){
    this->funcsCount = funcsCount;
    this->funcs = funcs;
    this->funcTargetsCount = funcTargetsCount;
    this->funcTargets = funcTargets;
    this->stringsCount = stringsCount;
    this->strings = strings;
  }

  inline String getFuncTarget(int id){
    if(id < funcTargetsCount){
      return funcTargets[id];
    }
    return "";
  }

  inline BytecodeFunc* getFunc(String name) const {
    for (int i = 0; i < funcsCount; i++) {
      if (funcs[i].name == name) {
        return funcs + i;
      }
    }
    return nullptr;
  }

  inline BytecodeFunc* entryPoint() const {
    return getFunc("main");
  }
};

enum class OpCodeI : unsigned char {
  NOP,
};

#endif