#ifndef LIST_HPP
#define LIST_HPP

template<typename T>
class List {
private:
  T* arr = nullptr;
  const static int smallSize = 4;
  T smallArr[smallSize];  //avoid dynamic allocation when possible
  int count = 0;
  int size = 4;
  void resize(int newSize) {
    if (newSize > size) {
      bool dynamic = false;
      T* old = smallArr;
      if (arr != nullptr) {
        old = arr;
        dynamic = true;
      }
      arr = new T[newSize];
      for (int i = 0; i < count; i++) {
        arr[i] = old[i];
      }
      size = newSize;
      if (dynamic) {
        delete old;
      }
    }
  }
public:
  List(){

  }
  ~List(){
    if (arr != nullptr) {
      delete arr;
    }
  }
  T* dest() {
    if (arr != nullptr) {
      return arr;
    } else {
      return smallArr;
    }
  }
  void add(T item) {
    count = count + 1;
    if (count >= size) {
      resize(2 * size);
    }
    dest()[count-1] = item;
  }
  int getCount() {
    return count;
  }
  void clear() {
    count = 0;
    if (arr != nullptr) {
      delete arr;
      arr = nullptr;
      size = smallSize;
    }
  }
  void fastClear(){
    count = 0;
  }

  T& operator[](int index) {
    return dest()[index];
  }
  const T& operator[](int index) const {
    return dest()[index];
  }
};

#endif