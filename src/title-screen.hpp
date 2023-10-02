#pragma once

#include "common.hpp"
class TitleScreen {
public:
  enum class State {
    PressStart,
    Options,
    HowToPlay,
  };

  State state;
  u8 current_option;

  TitleScreen();
  ~TitleScreen();
  void loop();
};
