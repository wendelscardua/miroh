#pragma once

#include "board.hpp"
#include "common.hpp"
class TitleScreen {
public:
  static const u8 BANK = 0;
  static const s16 TITLE_SCROLL = 0x100;
  static const s16 HOW_TO_SCROLL = 0x0;
  static const s16 PALETTE_SWAP_POINT = 0xb0;
  static const s16 JR_X_POSITION = 0x1c8;
  static const u8 JR_Y_POSITION = 0x50;
  static const s16 CURSOR_X_POSITION = 0x148;
  static const s16 HOW_TO_LEFT_X_POSITION = 0x38;
  static const u8 HOW_TO_LEFT_Y_POSITION = 0x50;
  static const s16 HOW_TO_RIGHT_X_POSITION = 0xb8;
  static const u8 HOW_TO_RIGHT_Y_POSITION = 0x50;

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
  s16 x_scroll;

  TitleScreen(Board &board);
  ~TitleScreen();
  void loop();
};
