#ifndef LIST_HPP
#define LIST_HPP

template<typename T>
class List {
private:
  T* arr = nullptr;
  T smallArr[10];  //avoid dynamic allocation when possible
  int count = 0;
  int size = 10;
};

#endif