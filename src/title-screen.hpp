#pragma once

#include "common.hpp"
class TitleScreen {
public:
  enum class State : u8 {
    PressStart,
    Options,
    HowToPlay,
    Credits,
  };

  enum class MenuOption : u8 {
    Controls = 0,
    Credits = 1,
    Start = 2,
    Settings = 3,
  };

  State state;
  MenuOption current_option;

  TitleScreen();
  ~TitleScreen();
  void loop();
};
