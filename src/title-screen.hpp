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
  s8 how_to_animation_step;
  s16 how_to_animation_framecount;

  TitleScreen();
  ~TitleScreen();
  void loop();
};
