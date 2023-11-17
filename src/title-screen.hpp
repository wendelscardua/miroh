#pragma once

#include "board.hpp"
#include "common.hpp"
#include "soundtrack.hpp"
#include <neslib.h>

class TitleScreen {
public:
  static const u8 BANK = 2;
  static const s16 TITLE_SCROLL = 0x100;
  static const s16 HOW_TO_SCROLL = 0x0;
  static const s16 PALETTE_SWAP_POINT = 0xb0;
  static const s16 JR_X_POSITION = 0x1c8;
  static const u8 JR_Y_POSITION = 0x4f;
  static const s16 MAIN_MENU_CURSOR_X_POSITION = 0x148;
  static const s16 MODE_MENU_CURSOR_X_POSITION = 0x158;
  static const s16 HOW_TO_LEFT_X_POSITION = 0x38;
  static const u8 HOW_TO_LEFT_Y_POSITION = 0x4f;
  static const s16 HOW_TO_RIGHT_X_POSITION = 0xb8;
  static const u8 HOW_TO_RIGHT_Y_POSITION = 0x4f;
  static const u8 NEXT_TRACK_DELAY = 30;
  static const u16 TRACK_ID_POSITION = NTADR_A(28, 26);

  enum class State : u8 {
    MainMenu,
    ModeMenu,
    HowToPlay,
  };

  enum class MenuOption : u8 {
    OnePlayer,
    TwoPlayers,
    HowToPlay,
  };

  State state;
  MenuOption current_option;
  Song current_track;
  u8 next_track_delay;
  Board &board;
  s16 x_scroll;

  TitleScreen(Board &board);
  ~TitleScreen();
  void render_sprites();
  void loop();
};
