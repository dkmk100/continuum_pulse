#ifndef OPCODES_HPP
#define OPCODES_HPP
#include <Arduino.h>

enum class OpCodeI : unsigned char;

enum class PrimitiveType : unsigned char {
  VOID = 0,
  INT = 1,
  STRING = 2,
  BOOL = 3,
  PTR = 4,
  CHAR = 5
};

struct BytecodeInst {
  OpCodeI opCode;
  PrimitiveType type;
  int num1;
  int num2;
  int num3;
  BytecodeInst(OpCodeI opCode, PrimitiveType type, int num1 = -1, int num2 = -1, int num3 = -1) {
    this->opCode = opCode;
    this->type = type;
    this->num1 = num1;
    this->num2 = num2;
    this->num3 = num3;
  }
  BytecodeInst(OpCodeI opCode, int num1 = -1, int num2 = -1, int num3 = -1) {
    this->opCode = opCode;
    this->type = PrimitiveType::VOID;
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

  BytecodeProgram() {
  }
  BytecodeProgram(int funcsCount, BytecodeFunc* funcs, int funcTargetsCount, String* funcTargets, int stringsCount, String* strings) {
    this->funcsCount = funcsCount;
    this->funcs = funcs;
    this->funcTargetsCount = funcTargetsCount;
    this->funcTargets = funcTargets;
    this->stringsCount = stringsCount;
    this->strings = strings;
  }

  inline String getFuncTarget(int id) const {
    if (id < funcTargetsCount) {
      return funcTargets[id];
    }
    Serial.print("invalid func id: ");
    Serial.println(String(id));
    delay(100);
    return "";
  }

  inline BytecodeFunc* getFunc(String name) const {
    for (int i = 0; i < funcsCount; i++) {
      if (funcs[i].name == name) {
        return funcs + i;
      }
    }
    Serial.print("invalid func name: ");
    Serial.println(name);
    delay(100);
    return nullptr;
  }

  inline BytecodeFunc* entryPoint() const {
    return getFunc("main");
  }
};

enum class OpCodeI : unsigned char {
  NOP,
  //basics
  ADD_INT,
  ASSIGN_INT,
  MOVE,
  NEGATE_INT,
  MULT_INT,
  ASSIGN_STR,
  ASSIGN_CHAR,
  //printing
  PRINT_STR_CONST,
  PRINT_STR_VAR,
  PRINT_NUM,
  PRINT_BOOL,
  PRINT_CHAR,
  PRINT_ADDR,
  //comparisons
  COMPARE_LESS,
  COMPARE_EQ,
  COMPARE_LEQ,
  //boolean operations
  NOT_BOOL,
  AND_BOOL,
  OR_BOOL,
  BITWISE_XOR,
  //basic control statements
  LABEL,
  JMP,
  JMP_IF,
  FUNC_CALL,
  FUNC_ARGS,
  RETURN,
  //pointers
  ADDR_OF,
  CALC_ADDR_CONST,
  CALC_ADDR_VAR,
  MOVE_TO,
  MOVE_FROM,
  PTR_INC,

  //function call for a builtin function
  CALL_BUILTIN
};

#endif