#pragma once

#include "board.hpp"
#include "common.hpp"
class TitleScreen {
public:
  static constexpr u8 BANK = 0;

  enum class State : u8 {
    MainMenu,
    HowToPlay,
  };

  enum class MenuOption : u8 {
    OnePlayer,
    TwoPlayers,
    HowToPlay,
  };

  State state;
  MenuOption current_option;
  Board &board;

  TitleScreen(Board &board);
  ~TitleScreen();
  void loop();
};
