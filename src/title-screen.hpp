#pragma once

#include "common.hpp"
class TitleScreen {
public:
  enum class State {
    PressStart,
    Options,
    HowToPlay,
    Credits,
  };

  State state;
  u8 current_option;

  TitleScreen();
  ~TitleScreen();
  void loop();
};
