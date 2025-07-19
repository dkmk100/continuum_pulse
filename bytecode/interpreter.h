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
  int calcOffset();
  int readFromAddress();
  void writeToAddress(int val);
  int getLocalAddress(int local);
  bool isBuiltinFunction(const char* func);
  void callBuiltinFunction(const char* func, int r1, int r2);

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
  }

  inline bool ready(){
    return valid && frames[frame].ip < frames[frame].func->codeLen;
  }

  inline void begin(const BytecodeProgram& program) {
    frame = 0;
    frames[0].ip = 0;
    frames[0].func = program.entryPoint();
    this->program = &program;
    valid = true;
  }

  inline void halt(){
    valid = false;
  }

  bool step(void (*print)(const char*), bool debug, bool verbose);

  inline void run(const BytecodeProgram& program, void (*print)(const char*), bool debug, bool verbose) {
    begin(program);
    while (frames[frame].ip < frames[frame].func->codeLen) {
      if(!step(print, debug, verbose)){
        return;
      }
    }
    valid = false;
  }
};

#endif