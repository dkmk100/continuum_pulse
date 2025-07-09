
class TextWriter {
public:
  virtual void print(const char* text) = 0;
  virtual void println() = 0;
  virtual void flush() = 0;

  void vprintf(const char* fmt, va_list& args) {
    int bufSize = 300;
    char buf[bufSize];

    //TODO allocate enough space or something
    int rslt = vsnprintf(buf, bufSize, fmt, args);
    if (rslt > 0) {
      print(buf);
    }
  }
  void printf(const char* fmt, ...) {
    va_list args;
    vprintf(fmt, args);
    va_end(args);
  }
};

class Logger {
private:
  TextWriter* writer;
public:
  Logger(TextWriter& writer) {
    this->writer = &writer;
  }
  inline void log(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    writer->vprintf(fmt, args);
    writer->println();
    writer->flush();
    va_end(args);
  }
};