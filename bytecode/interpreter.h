#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP
#include <cstring>
#include "opcodes.h"
#include "list.h"

//TODO optimize variable storage
const int maxFrame = 15;
const int maxVar = 16;
const int startHeap = 256;

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
  StackFrame* frames;
  const BytecodeProgram* program;

  List<int> fArgs;
  List<int> labelIds;
  List<int> labelLocations;

  void addArgs(int a1, int a2, int a3, bool, bool);
  int callFunction(BytecodeFunc* func, int r1, int r2);
  int returnFunction(int r1, int r2, bool, bool);
  int heapAlloc(int count);
  void heapFree(int address);
  void resizeHeap(int newSize);
  void setConstAddress(int num1, int num2, int num3);
  void setVarAddress(int num1, int num2, int num3);

  inline int calcOffset() {
    if (addressIsLiteral) {
      return a2;
    } else if (a2 < 0) {
      return 0;
    } else {
      return frames[frame].vars[a2];
    }
  }

  bool* builtinCache = nullptr;

  int readFromAddress();
  void writeToAddress(int val);
  int getLocalAddress(int local);
  bool isBuiltinFunctionRaw(const char* func);
  void callBuiltinFunction(const char* func, int r1, int r2);
  inline bool isBuiltinFunction(int id) {
    return builtinCache[id];
  }

  int getJumpTarget(int id, int startPos, bool, bool);
  void addLabel(int id, int pos, bool, bool);
  int getLabelPos(int id);

  int stackSize = maxFrame * maxVar;
  int stackRegCount = maxVar;

  int* heap;
  int heapLength;
  int heapEnd = 0;

  bool addressValid = false;
  bool addressIsLiteral = false;
  int a1, a2, a3;

  bool valid = false;

  int completedCount = 0;
  char strBuff[16];

  bool doStep(void (*print)(const char*), bool debug, bool verbose);

public:
  Interpreter(BuiltinFunc* builtinFuncs, int builtinFuncsCount) {
    builtins = builtinFuncs;
    builtinsCount = builtinFuncsCount;
    heap = new int[startHeap];
    heapLength = startHeap;
    frames = new StackFrame[maxFrame];
  }
  ~Interpreter() {
    delete frames;
    delete heap;
    delete builtinCache;
  }

  inline BytecodeInst nextInst(){
    return frames[frame].func->code[frames[frame].ip];
  }

  inline bool ready() {
    return valid && frames[frame].ip < frames[frame].func->codeLen;
  }

  inline void begin(const BytecodeProgram& program) {
    frame = 0;
    frames[0].ip = 0;
    frames[0].func = program.entryPoint();
    this->program = &program;
    valid = true;
    completedCount = 0;
    builtinCache = new bool[program.funcTargetsCount];
    for (int i = 0; i < program.funcTargetsCount; i++) {
      builtinCache[i] = isBuiltinFunctionRaw(program.funcTargets[i]);
    }
  }

  inline void halt() {
    valid = false;
    completedCount = 0;
  }

  inline int getCompleted() {
    return completedCount;
  }
  inline void resetCompleted() {
    completedCount = 0;
  }

  inline bool step(void (*print)(const char*), bool debug, bool verbose) {
    valid = doStep(print, debug, verbose);
    completedCount += 1;
    return valid;
  }

  inline void run(const BytecodeProgram& program, void (*print)(const char*), bool debug, bool verbose) {
    begin(program);
    while (frames[frame].ip < frames[frame].func->codeLen) {
      if (!step(print, debug, verbose)) {
        return;
      }
    }
    valid = false;
  }
};

#endif