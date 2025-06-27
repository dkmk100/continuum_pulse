#ifndef LIST_HPP
#define LIST_HPP

template<typename T>
class List {
private:
  T* arr = nullptr;
  T smallArr[10];  //avoid dynamic allocation when possible
  int count = 0;
  int size = 10;
  void resize(int newSize) {
    if (newSize > size) {
      bool dynamic = false;
      T* old = smallArr;
      if (arr != nullptr) {
        old = arr;
        dynamic = true;
      }
      arr = new T[newSize];
      for (int i = 0; i < size; i++) {
        arr[i] = old[i];
      }
      if (dynamic) {
        delete old;
      }
    }
  }
public:
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
    }
  }

  T& operator[](int index) {
    return dest()[index];
  }
  const T& operator[](int index) const {
    return dest()[index];
  }
};

#endif