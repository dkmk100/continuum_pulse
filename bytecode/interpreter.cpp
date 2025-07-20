#include "Print.h"
#include "interpreter.h"

#include <cstring>
#include <sstream>
#include <cstring>

#include <Arduino.h>

inline bool ToBool(int x) {
  return x != 0;
}
inline int ToInt(bool x) {
  return x ? 1 : 0;
}

String ToHex(int x) {
  char c[20];
  int i = 0;
  while (x > 0) {
    c[20 - i - 1] = x % 16;
    x = x / 16;
  }
  return String(c + 20 - i - 1);
}

void Interpreter::addArgs(int a1, int a2, int a3, bool debug, bool verbose) {
  fArgs.add(a1);
  if (verbose) {
    Serial.print("add arg: ");
    Serial.print(String(frames[frame].vars[a1]).c_str());
    Serial.print(" (");
    Serial.print(String(a1).c_str());
    Serial.println(")");
  }
  if (a2 > 0) {
    fArgs.add(a2);
    if (verbose) {
      Serial.print("add arg: ");
      Serial.print(String(frames[frame].vars[a2]).c_str());
      Serial.print(" (");
      Serial.print(String(a2).c_str());
      Serial.println(")");
    }
  }
  if (a3 > 0) {
    fArgs.add(a3);
    if (verbose) {
      Serial.print("add arg: ");
      Serial.print(String(frames[frame].vars[a3]).c_str());
      Serial.print(" (");
      Serial.print(String(a3).c_str());
      Serial.println(")");
    }
  }
}

int Interpreter::callFunction(BytecodeFunc* func, int r1, int r2) {
  if (frame >= maxFrame) {
    Serial.println("Interpreter stack overflow!!");
    delay(100);
    return -1;
  }

  frame = frame + 1;
  stackRegCount += maxVar;

  frames[frame].ip = 0;
  frames[frame].func = func;

  frames[frame].fRet.fastClear();
  frames[frame].fRet.add(r1);
  frames[frame].fRet.add(r2);

  for (int i = 0; i < func->args; i++) {
    int v = fArgs[i];
    if (v < 0) {
      Serial.println("missing function argument...");
      delay(100);
      return -1;
    }
    frames[frame].vars[i] = frames[frame - 1].vars[v];
  }

  //clear args before next function call
  fArgs.fastClear();

  //start function at 0
  return 0;
}

int Interpreter::returnFunction(int r1, int r2, bool debug, bool verbose) {
  //TODO check this better
  if (frame == 0) {
    if (debug) {
      Serial.println("exiting last frame");
    }
    return -1;
  }

  int returns[] = { 0, 0 };
  int oldFRet[] = { -1, -1 };

  if (frames[frame].fRet.getCount() > 0 && frames[frame].fRet[0] >= 0) {
    returns[0] = frames[frame].vars[r1];
    oldFRet[0] = frames[frame].fRet[0];
  }
  if (frames[frame].fRet.getCount() > 1 && frames[frame].fRet[1] >= 0) {
    returns[1] = frames[frame].vars[r2];
    oldFRet[1] = frames[frame].fRet[1];
  }

  frame = frame - 1;
  stackRegCount -= maxVar;

  if (oldFRet[0] >= 0) {
    frames[frame].vars[oldFRet[0]] = returns[0];
    if (debug) {
      Serial.print("return1: ");
      Serial.print(returns[0]);
      Serial.print(" (");
      Serial.println(oldFRet[0]);
      Serial.println(")");
    }
  }
  if (oldFRet[1] >= 0) {
    frames[frame].vars[oldFRet[1]] = returns[1];
    if (debug) {
      Serial.print("return2: ");
      Serial.print(returns[1]);
      Serial.print(" (");
      Serial.println(oldFRet[1]);
      Serial.println(")");
    }
  }

  //return pos
  return frames[frame].ip + 1;
}

//we store stuff in the interpreter as integers
//almost nothing takes up more than one
int getSizeFor(PrimitiveType type) {
  switch (type) {
    default:
      return 1;
  }
}

//TODO just make a heap class...
int Interpreter::heapAlloc(int count) {
  int pos = heapEnd;
  heapEnd += count;
  int targetSize = heapLength;
  while (heapEnd < targetSize) {
    targetSize = 2 * targetSize;
  }
  resizeHeap(targetSize);

  return pos + stackSize;
}

void Interpreter::heapFree(int address) {
  //TODO actually add proper heap allocation/free system
}

void Interpreter::resizeHeap(int newSize) {
  if (newSize > heapLength) {
    int* old = heap;
    int oldSize = heapLength;
    heap = new int[newSize];
    heapLength = newSize;
    std::memcpy(heap, old, oldSize * sizeof(int));
    delete old;
  }
}



void Interpreter::setConstAddress(int num1, int num2, int num3) {
  addressValid = true;
  addressIsLiteral = true;
  a1 = num1;
  a2 = num2;
  a3 = num3;
}
void Interpreter::setVarAddress(int num1, int num2, int num3) {
  addressValid = true;
  addressIsLiteral = false;
  a1 = num1;
  a2 = num2;
  a3 = num3;
}

int Interpreter::getLabelPos(int id) {
  //TODO maybe use a sorted array?
  //or flag set?
  for (int i = 0; i < labelIds.getCount(); i++) {
    if (labelIds[i] == id) {
      return labelLocations[i];
    }
  }
  return -1;
}

void Interpreter::addLabel(int id, int pos, bool debug, bool verbose) {
  //this runs on every label, so it's gonna be a little slow...
  if (getLabelPos(id) < 0) {
    if (verbose) {
      Serial.print("adding label: ");
      Serial.print(id);
      Serial.print(" at ");
      Serial.println(pos);
    }
    labelIds.add(id);
    labelLocations.add(pos);
  }
}

int Interpreter::getJumpTarget(int id, int startPos, bool debug, bool verbose) {
  int label = getLabelPos(id);
  if (label >= 0) {
    if (verbose) {
      Serial.print("found label ");
      Serial.print(id);
      Serial.print(" at ");
      Serial.println(label);
    }
    return label;
  }

  if (debug) {
    Serial.print("searching for label: ");
    Serial.println(id);
  }

  //we've definitely seen the prior areas
  for (int i = startPos; i < frames[frame].func->codeLen; i++) {
    BytecodeInst* inst = frames[frame].func->code;
    if (inst[i].opCode == OpCodeI::LABEL && inst[i].num1 == id) {
      if (debug) {
        Serial.print("found label at: ");
        Serial.println(i);
      }
      return i;
    }
  }

  Serial.print("error: label not found: ");
  Serial.println(id);

  return -1;
}


int Interpreter::readFromAddress() {
  if (!addressValid) {
    return 0;
  }
  addressValid = false;
  int ptr = frames[frame].vars[a1];
  int offset = calcOffset();
  int scale = getSizeFor((PrimitiveType)a3);
  if (ptr < stackRegCount + maxVar) {
    //a register value
    if (offset != 0) {
      return 0;
    }

    if (ptr < stackRegCount) {
      int* sVars = frames[ptr / maxVar].vars;
      int i = ptr % maxVar;
      return sVars[i];
    } else {
      return frames[frame].vars[ptr - stackRegCount];
    }
  } else if (ptr >= stackSize) {
    int heapPtr = (ptr - stackSize) + offset * scale;
    if (heapPtr < heapEnd) {
      return heap[heapPtr];
    } else {
      Serial.print("Heap overflow at: ");
      Serial.print(ptr);
      Serial.print(", ");
      Serial.print(offset);
      Serial.print(", ");
      Serial.println(scale);
      valid = false;
      return 0;
    }
  } else {
    Serial.print("Illegal address at: ");
    Serial.print(ptr);
    Serial.print(", ");
    Serial.print(offset);
    Serial.print(", ");
    Serial.println(scale);
    valid = false;
    return 0;
  }
}
void Interpreter::writeToAddress(int val) {
  if (!addressValid) {
    Serial.println("Address not set");
    valid = false;
    return;
  }
  addressValid = false;
  int ptr = frames[frame].vars[a1];
  int offset = calcOffset();
  int scale = getSizeFor((PrimitiveType)a3);
  if (ptr < stackRegCount + maxVar) {
    //a register value
    if (offset != 0) {
      Serial.print("Illegal offset at: ");
      Serial.print(ptr);
      Serial.print(", ");
      Serial.print(offset);
      Serial.print(", ");
      Serial.println(scale);
      valid = false;
      return;
    }

    if (ptr < stackRegCount) {
      int* sVars = frames[ptr / maxVar].vars;
      int i = ptr % maxVar;
      sVars[i] = val;
    } else {
      frames[frame].vars[ptr - stackRegCount] = val;
    }
  } else if (ptr >= stackSize) {
    int heapPtr = (ptr - stackSize) + offset * scale;
    if (heapPtr < heapEnd) {
      heap[heapPtr] = val;
    } else {
      Serial.print("Heap overflow at: ");
      Serial.print(ptr);
      Serial.print(", ");
      Serial.print(offset);
      Serial.print(", ");
      Serial.println(scale);
      valid = false;
      return;
    }
  } else {
    Serial.print("Illegal address at: ");
    Serial.print(ptr);
    Serial.print(", ");
    Serial.print(offset);
    Serial.print(", ");
    Serial.println(scale);
    valid = false;
    return;
  }
}

int Interpreter::getLocalAddress(int local) {
  return local + stackRegCount;
}


bool Interpreter::isBuiltinFunctionRaw(const char* func) {
  return !strcmp(func, "libc.malloc") || !strcmp(func, "libc.free");
}

void Interpreter::callBuiltinFunction(const char* func, int r1, int r2) {
  int* args = new int[fArgs.getCount()];
  for (int i = 0; i < fArgs.getCount(); i++) {
    int v = fArgs[i];
    if (v < 0) {
      break;
    }
    args[i] = frames[frame].vars[v];
  }

  //clear args before next function call
  fArgs.fastClear();
  if (!strcmp(func, "libc.malloc")) {
    frames[frame].vars[r1] = heapAlloc(args[0]);
  } else if (!strcmp(func, "libc.free")) {
    heapFree(args[0]);
  }
  delete args;
}

bool Interpreter::doStep(void (*print)(const char*), bool debug, bool verbose) {
  int i = frames[frame].ip;
  if (!valid || i < 0 || i > frames[frame].func->codeLen) {
    frames[frame].ip = -1;
    return false;
  }
  if (!valid) {
    return false;
  }

  ///*
  int instPointer = i + 1;

  BytecodeInst* inst = frames[frame].func->code;
  int* vars = frames[frame].vars;

  if (verbose) {
    (*print)("(ip: ");
    (*print)(String(i).c_str());
    (*print)(", opcode: ");
    (*print)(String((int)inst[i].opCode).c_str());
    (*print)(", func: ");
    (*print)(frames[frame].func->name);
    (*print)(")\n");
  }

  int val;
  BytecodeInst curInst = inst[i];

  //run instruction
  switch (curInst.opCode) {
    case OpCodeI::ADD_INT:
      vars[curInst.num1] = vars[curInst.num1] + vars[curInst.num2];
      break;
    case OpCodeI::MULT_INT:
      vars[curInst.num1] = vars[curInst.num1] * vars[curInst.num2];
      break;
    case OpCodeI::NEGATE_INT:
      vars[curInst.num1] = -vars[curInst.num1];
      break;
    case OpCodeI::MOVE:
      vars[curInst.num1] = vars[curInst.num2];
      break;
    case OpCodeI::ASSIGN_INT:
      vars[curInst.num1] = curInst.num2;
      break;
    case OpCodeI::ASSIGN_STR:
      vars[curInst.num1] = curInst.num2;
      break;
    case OpCodeI::ASSIGN_CHAR:
      vars[curInst.num1] = curInst.num2;
      break;
    case OpCodeI::BITWISE_XOR:
      vars[curInst.num1] = vars[curInst.num1] ^ vars[curInst.num2];
      break;
    case OpCodeI::COMPARE_EQ:
      //equal to comparison
      vars[curInst.num3] = ToInt(vars[curInst.num1] == vars[curInst.num2]);
      break;
    case OpCodeI::COMPARE_LESS:
      //less than comparison
      vars[curInst.num3] = ToInt(vars[curInst.num1] < vars[curInst.num2]);
      break;
    case OpCodeI::COMPARE_LEQ:
      //less than or equal to comparison
      vars[curInst.num3] = ToInt(vars[curInst.num1] <= vars[curInst.num2]);
      break;
    case OpCodeI::NOT_BOOL:
      //negation
      vars[curInst.num1] = ToInt(vars[curInst.num1] == 0);
      break;
    case OpCodeI::OR_BOOL:
      //boolean or
      vars[curInst.num1] = ToInt(ToBool(vars[curInst.num1]) || ToBool(vars[curInst.num2]));
      break;
    case OpCodeI::AND_BOOL:
      //boolean and
      vars[curInst.num1] = ToInt(ToBool(vars[curInst.num1]) && ToBool(vars[curInst.num2]));
      break;


    case OpCodeI::ADDR_OF:
      vars[curInst.num1] = getLocalAddress(curInst.num2);
      break;
    case OpCodeI::PTR_INC:
      //TODO throw error when doing this with non-heap variables
      vars[curInst.num1] += vars[curInst.num2] * getSizeFor(curInst.type);
      break;
    case OpCodeI::CALC_ADDR_CONST:
      setConstAddress(curInst.num1, curInst.num2, curInst.num3);
      break;
    case OpCodeI::CALC_ADDR_VAR:
      setVarAddress(curInst.num1, curInst.num2, curInst.num3);
      break;
    case OpCodeI::MOVE_TO:
      val = vars[curInst.num1];
      writeToAddress(val);
      break;
    case OpCodeI::MOVE_FROM:
      val = readFromAddress();
      vars[curInst.num1] = val;
      break;

    case OpCodeI::FUNC_CALL:
      {
        int id = curInst.num1;
        if (isBuiltinFunction(id)) {
          const char* name = program->getFuncTarget(id);
          if (debug) {
            Serial.print("going to call builtin func: ");
            Serial.println(name);
          }
          callBuiltinFunction(name, curInst.num2, curInst.num3);
        } else {
          BytecodeFunc* func = program->getFuncDirect(id);
          if (debug) {
            Serial.print("func found: ");
            Serial.print((int)func, HEX);
            Serial.print('\t');
            Serial.println(func->name);
          }
          instPointer = callFunction(func, curInst.num2, curInst.num3);
        }
      }
      break;
    case OpCodeI::CALL_BUILTIN:
      {
        //TODO return values lol
        int target = frames[frame].vars[curInst.num1];
        BuiltinFunc& func = builtins[target];
        int* args = new int[fArgs.getCount()];
        for (int i = 0; i < fArgs.getCount(); i++) {
          int v = fArgs[i];
          if (v < 0) {
            break;
          }
          args[i] = frames[frame].vars[v];
          if (verbose) {
            Serial.print("arg: ");
            Serial.print(String(args[i]).c_str());
            Serial.print(" (");
            Serial.print(String(v).c_str());
            Serial.println(")");
          }
        }
        if (debug) {
          Serial.print("Invoking func: ");
          Serial.println(target);
          Serial.print("Args count: ");
          Serial.println(fArgs.getCount());
          delay(100);
        }
        (*(func.ptr))(args);
        fArgs.fastClear();
        delete args;
      }
      break;
    case OpCodeI::FUNC_ARGS:
      addArgs(curInst.num1, curInst.num2, curInst.num3, debug, verbose);
      break;
    case OpCodeI::RETURN:
      instPointer = returnFunction(curInst.num1, curInst.num2, debug, verbose);
      break;

    case OpCodeI::LABEL:
      addLabel(curInst.num1, i, debug, verbose);
      break;
    case OpCodeI::JMP:
      instPointer = getJumpTarget(curInst.num1, i, debug, verbose);  //perform the jump
      break;
    case OpCodeI::JMP_IF:
      if (vars[curInst.num2] != 0) {
        instPointer = getJumpTarget(curInst.num1, i, debug, verbose);  //perform the jump
      }
      break;
    case OpCodeI::PRINT_STR_CONST:
      (*print)(program->strings[curInst.num1]);
      break;
    case OpCodeI::PRINT_STR_VAR:
      //get the string pointer at pos
      //and print that string
      (*print)(program->strings[vars[curInst.num1]]);
      break;
    case OpCodeI::PRINT_NUM:
      {
        itoa(vars[curInst.num1], strBuff, 10);
        (*print)(strBuff);
      }
      break;
    case OpCodeI::PRINT_ADDR:
      {
        itoa(vars[curInst.num1], strBuff, 8);
        (*print)(strBuff);
      }
      break;
    case OpCodeI::PRINT_BOOL:
      if (vars[curInst.num1] == 0) {
        (*print)("false");
      } else {
        (*print)("true");
      }
      break;
    case OpCodeI::PRINT_CHAR:
      {
        strBuff[0] = (char)vars[curInst.num1];
        strBuff[1] = 0;
        (*print)(strBuff);
      }
      break;
    default:
      (*print)("Invalid opcode: ");
      {
        itoa(vars[(int)curInst.opCode], strBuff, 10);
        (*print)(strBuff);
      }
      return false;
  }

  frames[frame].ip = instPointer;
  //*/
  return true;
}