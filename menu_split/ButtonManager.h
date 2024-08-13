template<int N, bool TAP_ON_RELEASE, bool (*buttonFunc[N])()>
struct ButtonManager {
protected:
  bool pressed[N] = { 0 };
  bool tapped[N] = { 0 };

public:
  virtual void Update() {
    bool previous[N];
    for (int i = 0; i < N; i++) {
      previous[i] = pressed[i];
    }
    for (int i = 0; i < N; i++) {
      pressed[i] = buttonFunc[i]();
    }

    if (TAP_ON_RELEASE) {
      for (int i = 0; i < N; i++) {
        tapped[i] = !pressed[i] && previous[i];
      }
    } else {
      for (int i = 0; i < N; i++) {
        tapped[i] = pressed[i] && !previous[i];
      }
    }
  }

  bool getTapped(int button) {
    if (button < 0 || button >= N) {
      return false;
    }
    return tapped[button];
  }
  bool getPressed(int button) {
    if (button < 0 || button >= N) {
      return false;
    }
    return pressed[button];
  }

  bool getAnyPressed() {
    for (int i = 0; i < N; i++) {
      if (pressed[i]) {
        return true;
      }
    }
    return false;
  }
};