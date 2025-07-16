#include "interpreter.h"

#include <cstring>
#include <sstream>
#include <cstring>

#include <Arduino.h>

bool ToBool(int x) {
  return x != 0;
}
int ToInt(bool x) {
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

  frames[frame].fRet.clear();
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
  fArgs.clear();

  //start function at 0
  return 0;
}

int Interpreter::returnFunction(int r1, int r2) {
  //TODO check this better
  if (frame == 0) {
    return -1;
  }
  if (!strcmp(frames[frame].func->name, "main")) {
    return -1;
  }

  int returns[] = { 0, 0 };
  int oldFRet[] = { 0, 0 };

  if (frames[frame].fRet.getCount() > 0 && frames[frame].fRet[0] >= 0) {
    returns[0] = frames[frame].vars[r1];
    oldFRet[0] = frames[frame].fRet[0];
  }
  if (frames[frame].fRet.getCount() > 1 && frames[frame].fRet[1] >= 0) {
    returns[1] = frames[frame].vars[r2];
    oldFRet[1] = frames[frame].fRet[1];
  }

  frame = frame - 1;
  stackRegCount -= 256;

  if (oldFRet[1] >= 0) {
    frames[frame].vars[oldFRet[0]] = returns[0];
  }
  if (oldFRet[1] >= 0) {
    frames[frame].vars[oldFRet[1]] = returns[1];
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
  //LOL, just leaking memory
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

int Interpreter::calcOffset() {
  if (addressIsLiteral) {
    return a2;
  } else if (a2 < 0) {
    return 0;
  } else {
    return frames[frame].vars[a2];
  }
}

int Interpreter::getLabelPos(int id) {
  for (int i = 0; i < labelIds.getCount(); i++) {
    if (labelIds[i] == id) {
      return labelLocations[i];
    }
  }
  return -1;
}

void Interpreter::addLabel(int id, int pos) {
  if (getLabelPos(id) < 0) {
    labelIds.add(id);
    labelLocations.add(pos);
  }
}

int Interpreter::getJumpTarget(int id, int startPos) {
  int label = getLabelPos(id);
  if (label > 0) {
    return label;
  }

  for (int i = 0; i < frames[frame].func->codeLen; i++) {
    BytecodeInst* inst = frames[frame].func->code;
    if (inst[i].opCode == OpCodeI::LABEL && inst[i].num1 == id) {
      return i;
    }
  }
  return -1;
}

/*
int Interpreter::readFromAddress() {
  if (!addressValid) {
    return 0;
  }
  addressValid = false;
  int ptr = frames[frame].vars[address.Item1];
  int offset = calcOffset();
  int scale = getSizeFor((PrimitiveType)a3);
  if (ptr < stackRegCount + 256) {
    //a register value
    if (offset != 0) {
      return 0;
    }

    if (ptr < stackRegCount) {
      int i = ptr;
      foreach (var v in stack.Reverse()) {
        int[] sVars = v.Item3;
        if (i < 256) {
          return sVars[i];
        } else {
          i -= 256;
        }
      }
      return 0;
    } else {
      return vars[ptr - stackRegCount];
    }
  } else if (ptr >= stackSize) {
    int heapPtr = (ptr - stackSize) + offset * scale;
    if (heapPtr < heapEnd) {
      return heap[heapPtr];
    } else {
      logger.Error("Heap overflow at: " + ptr + ", " + offset + ", " + scale);
      validState = false;
      return 0;
    }
  } else {
    logger.Error("Illegal address at: " + ptr + ", " + offset + ", " + scale);
    validState = false;
    return 0;
  }
}
void WriteToAddress(int val, Logger logger) {
  if (!addressValid) {
    logger.Error("Address not set");
    validState = false;
    return;
  }
  addressValid = false;
  int ptr = vars[address.Item1];
  int offset = CalcOffset();
  int scale = address.Item3;
  if (ptr < stackRegCount + vars.Length) {
    //a register value
    if (offset != 0) {
      logger.Error("Illegal offset at: " + ptr + ", " + offset + ", " + scale);
      validState = false;
      return;
    }

    if (ptr < stackRegCount) {
      int i = ptr;
      foreach (var v in stack.Reverse()) {
        int[] sVars = v.Item3;
        if (i < sVars.Length) {
          sVars[i] = val;
          break;
        } else {
          i -= sVars.Length;
        }
      }
    } else {
      vars[ptr - stackRegCount] = val;
    }
  } else if (ptr >= stackSize) {
    int heapPtr = (ptr - stackSize) + offset * scale;
    if (heapPtr < heapEnd) {
      heap[heapPtr] = val;
    } else {
      logger.Error("Heap overflow at: " + ptr + ", " + offset + ", " + scale);
    }
  } else {
    logger.Error("Illegal address at: " + ptr + ", " + offset + ", " + scale);
    validState = false;
    return;
  }
}

int GetLocalAddress(int local) {
  return local + stackRegCount;
}

*/

bool Interpreter::isBuiltinFunction(const char* func) {
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
  fArgs.clear();
  if (!strcmp(func, "libc.malloc")) {
    frames[frame].vars[r1] = heapAlloc(args[0]);
  } else if (!strcmp(func, "libc.free")) {
    heapFree(args[0]);
  }
  delete args;
}

bool Interpreter::doStep(void (*print)(const char*), bool debug, bool verbose) {
  if (frames[frame].ip < 0 || frames[frame].ip > frames[frame].func->codeLen) {
    frames[frame].ip = -1;
    return false;
  }
  if (!valid) {
    return false;
  }

  int i = frames[frame].ip;
  frames[frame].ip++;

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

  //run instruction
  switch (inst[i].opCode) {
    case OpCodeI::ADD_INT:
      vars[inst[i].num1] = vars[inst[i].num1] + vars[inst[i].num2];
      break;
    case OpCodeI::MULT_INT:
      vars[inst[i].num1] = vars[inst[i].num1] * vars[inst[i].num2];
      break;
    case OpCodeI::NEGATE_INT:
      vars[inst[i].num1] = -vars[inst[i].num1];
      break;
    case OpCodeI::MOVE:
      vars[inst[i].num1] = vars[inst[i].num2];
      break;
    case OpCodeI::ASSIGN_INT:
      vars[inst[i].num1] = inst[i].num2;
      break;
    case OpCodeI::ASSIGN_STR:
      vars[inst[i].num1] = inst[i].num2;
      break;
    case OpCodeI::ASSIGN_CHAR:
      vars[inst[i].num1] = inst[i].num2;
      break;
    case OpCodeI::BITWISE_XOR:
      vars[inst[i].num1] = vars[inst[i].num1] ^ vars[inst[i].num2];
      break;
    case OpCodeI::COMPARE_EQ:
      //equal to comparison
      vars[inst[i].num3] = ToInt(vars[inst[i].num1] == vars[inst[i].num2]);
      break;
    case OpCodeI::COMPARE_LESS:
      //less than comparison
      vars[inst[i].num3] = ToInt(vars[inst[i].num1] < vars[inst[i].num2]);
      break;
    case OpCodeI::COMPARE_LEQ:
      //less than or equal to comparison
      vars[inst[i].num3] = ToInt(vars[inst[i].num1] <= vars[inst[i].num2]);
      break;
    case OpCodeI::NOT_BOOL:
      //negation
      vars[inst[i].num1] = ToInt(vars[inst[i].num1] == 0);
      break;
    case OpCodeI::OR_BOOL:
      //boolean or
      vars[inst[i].num1] = ToInt(ToBool(vars[inst[i].num1]) || ToBool(vars[inst[i].num2]));
      break;
    case OpCodeI::AND_BOOL:
      //boolean and
      vars[inst[i].num1] = ToInt(ToBool(vars[inst[i].num1]) && ToBool(vars[inst[i].num2]));
      break;

      /*
    case OpCodeI::ADDR_OF:
      vars[inst[i].num1] = GetLocalAddress(inst[i].num2);
      break;
    case OpCodeI::PTR_INC:
      //TODO throw error when doing this with non-heap variables
      vars[inst[i].num1] += vars[inst[i].num2] * GetSizeFor(inst[i].type);
      break;
    case OpCodeI::CALC_ADDR_CONST:
      SetConstAddress(inst[i].num1, inst[i].num2, inst[i].num3);
      break;
    case OpCodeI::CALC_ADDR_VAR:
      SetVarAddress(inst[i].num1, inst[i].num2, inst[i].num3);
      break;
    case OpCodeI::MOVE_TO:
      val = vars[inst[i].num1];
      WriteToAddress(val, logger);
      break;
    case OpCodeI::MOVE_FROM:
      val = ReadFromAddress(logger);
      vars[inst[i].num1] = val;
      break;
    */

    case OpCodeI::FUNC_CALL:
      {
        const char* name = program->getFuncTarget(inst[i].num1);
        Serial.print("going to call func: ");
        Serial.println(name);
        if (isBuiltinFunction(name)) {
          callBuiltinFunction(name, inst[i].num2, inst[i].num3);
        } else {
          BytecodeFunc* func = program->getFunc(name);
          instPointer = callFunction(func, inst[i].num2, inst[i].num3);
        }
      }
      break;
    case OpCodeI::CALL_BUILTIN:
      {
        //TODO return values lol
        int target = frames[frame].vars[inst[i].num1];
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
        fArgs.clear();
        delete args;
      }
      break;
    case OpCodeI::FUNC_ARGS:
      addArgs(inst[i].num1, inst[i].num2, inst[i].num3, debug, verbose);
      break;
    case OpCodeI::RETURN:
      instPointer = returnFunction(inst[i].num1, inst[i].num2);
      break;

    case OpCodeI::LABEL:
      addLabel(inst[i].num1, i);
      break;
    case OpCodeI::JMP:
      instPointer = getJumpTarget(inst[i].num1, i);  //perform the jump
      break;
    case OpCodeI::JMP_IF:
      if (vars[inst[i].num2] != 0) {
        instPointer = getJumpTarget(inst[i].num1, i);  //perform the jump
      }
      break;
    case OpCodeI::PRINT_STR_CONST:
      (*print)(program->strings[inst[i].num1]);
      break;
    case OpCodeI::PRINT_STR_VAR:
      //get the string pointer at pos
      //and print that string
      (*print)(program->strings[vars[inst[i].num1]]);
      break;
    case OpCodeI::PRINT_NUM:
      (*print)(String(vars[inst[i].num1]).c_str());
      break;
    case OpCodeI::PRINT_ADDR:
      (*print)(ToHex(vars[inst[i].num1]).c_str());
      break;
    case OpCodeI::PRINT_BOOL:
      if (vars[inst[i].num1] == 0) {
        (*print)("false");
      } else {
        (*print)("true");
      }
      break;
    case OpCodeI::PRINT_CHAR:
      (*print)(String(vars[inst[i].num1]).c_str());
      break;
    default:
      (*print)("Invalid opcode: ");
      (*print)(String((int)inst[i].opCode).c_str());
      return false;
  }

  frames[frame].ip = instPointer;

  return true;
}

bool Interpreter::step(void (*print)(const char*), bool debug, bool verbose) {
  bool rslt = doStep(print, debug, verbose);
  if (rslt == false) {
    valid = false;
  }
  return rslt;
}