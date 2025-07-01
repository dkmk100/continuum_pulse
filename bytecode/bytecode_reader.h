#include <cstddef>

class InStream{
  char* buff;
  int len;
  int count;
  std::size_t (*readFunc)(char*, std::size_t);
public:
  InStream(int len, std::size_t (*readFunc)(char*, std::size_t)){
    this->readFunc = readFunc;
    this->len = len;
    this->buff = new char[len];
    this->count = 0;
  }
  ~InStream(){
    delete this->buff;
  }
  inline bool advance(int amount){
    //copy stuff over
    int shift = amount;
    if(shift > count){
      shift = count;
    }
    int remaining = count - shift;
    for(int i=0;i<remaining;i++){
      buff[i] = buff[i+shift];
    }

    int reading = len - remaining;
    int rd = readFunc(buff + remaining, reading);
    if(rd < 0){
      count = 0;
      return false;
    }
    count = rd + remaining;
    return count > 0;
  }
  inline int getCount(){
    return count;
  }
  inline char* read(){
    return buff;
  }
};