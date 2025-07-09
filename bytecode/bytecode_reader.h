#include <cstddef>

class InStream {
private:
  char* buff;
  int len;
  int count;
  int offset;
  std::size_t (*readFunc)(char*, std::size_t);
  inline void home() {
    //copy everything to the left
    for (int i = 0; i < offset; i++) {
      buff[i] = buff[i + count];
    }
    offset = 0;
  }
  inline bool fillMissing() {
    int totalRead = (*readFunc)(buff + count + offset, len - count - offset);
    if (totalRead >= 0) {
      count = count + totalRead;
      return count > 0;
    }
    else{
      return false;
    }
  }
public:
  InStream(int len, std::size_t (*readFunc)(char*, std::size_t)) {
    this->readFunc = readFunc;
    this->len = len;
    this->buff = new char[len];
    this->count = 0;
    this->offset = 0;
  }
  ~InStream() {
    delete this->buff;
  }
  inline bool advance(int amount) {
    if (count == 0) {
      home();
      fillMissing();
    }
    while (amount > 0) {
      if (offset > len / 2) {
        //read in the rest of the text
        home();
      }
      if (count < len / 2) {
        if(!fillMissing()){
          return false;
        }
      }
      //decide how much to move at once
      int shift = amount;
      if (shift > count) {
        shift = count;
      }
      //move forward
      offset += shift;
      count -= shift;
      //subtract amount
      amount -= shift;
    }
    return count > 0;
  }
  inline int getCount() {
    return count;
  }
  inline char* read() {
    return buff + offset;
  }
};