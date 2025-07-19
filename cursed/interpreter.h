#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP
#include "opcodes.h"
#include "list.h"

const int maxFrame = 20;
const int maxVar = 32;

struct StackFrame {
  int vars[maxVar];
  int ip;
  const BytecodeFunc* func;
  List<int> fRet;
};

struct BuiltinFunc {
  PrimitiveType type;
  int argCount;
  PrimitiveType* argTypes;
  int (*ptr)(int*);
  BuiltinFunc(PrimitiveType type, int argCount, PrimitiveType* argTypes, int (*ptr)(int*)) {
    this->type = type;
    this->argCount = argCount;
    this->argTypes = argTypes;
    this->ptr = ptr;
  }
};

class Interpreter {
private:
  BuiltinFunc* builtins;
  int builtinsCount;

  int frame = 0;
  StackFrame* frames = new StackFrame[maxFrame];
  const BytecodeProgram* program;

  int stackSize = maxFrame * maxVar;
  int stackRegCount = maxVar;

  int* heap = new int[512];
  int heapLength = 512;
  int heapEnd = 0;

  bool addressValid = false;
  bool addressIsLiteral = false;
  int a1, a2, a3;

public:
  Interpreter(BuiltinFunc* builtinFuncs, int builtinFuncsCount) {
    builtins = builtinFuncs;
    builtinsCount = builtinFuncsCount;
  }
};

#endif