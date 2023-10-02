#pragma once

#include "common.hpp"
class TitleScreen {
public:
  enum class State : u8 {
    PressStart,
    Options,
    HowToPlay,
    Credits,
    Settings,
  };

  enum class MenuOption : u8 {
    Controls = 0,
    Credits = 1,
    Start = 2,
    Settings = 3,
  };

  enum class SettingsOption : u8 {
    LineGravity = 0,
    Maze = 1,
    Return = 2,
  };

  State state;
  MenuOption current_option;
  SettingsOption current_setting;

  TitleScreen();
  ~TitleScreen();
  void loop();
};
