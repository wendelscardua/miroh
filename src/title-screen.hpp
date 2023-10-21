#pragma once

#include "common.hpp"
class TitleScreen {
public:
  static constexpr u8 BANK = 0;

  enum class State : u8 {
    PressStart,
    Options,
    HowToPlay,
    Credits,
    Settings,
  };

  enum class MenuOption : u8 {
    Start = 0,
    Controls = 1,
    Settings = 2,
    Credits = 3,
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
