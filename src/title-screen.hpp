#pragma once

#include "common.hpp"
#include "soundtrack.hpp"
#include <neslib.h>

class TitleScreen {
public:
  static constexpr u8 BANK = 2;
  static constexpr s16 TITLE_SCROLL = 0x100;
  static constexpr s16 HOW_TO_SCROLL = 0x0;
  static constexpr s16 PALETTE_SWAP_POINT = 0xb0;
  static constexpr s16 JR_X_POSITION = 0x1c8;
  static constexpr u8 JR_Y_POSITION = 0x4f;
  static constexpr s16 MAIN_MENU_CURSOR_X_POSITION = 0x148;
  static constexpr s16 MODE_MENU_CURSOR_X_POSITION = 0x158;
  static constexpr s16 HOW_TO_LEFT_X_POSITION = 0x38;
  static constexpr u8 HOW_TO_LEFT_Y_POSITION = 0x4f;
  static constexpr s16 HOW_TO_RIGHT_X_POSITION = 0xb8;
  static constexpr u8 HOW_TO_RIGHT_Y_POSITION = 0x4f;
  static constexpr u8 NEXT_TRACK_DELAY = 30;
  static constexpr u16 TRACK_ID_POSITION = NTADR_A(28, 26);

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
  s16 x_scroll;
  u8 bgm_test_index;

  TitleScreen();
  ~TitleScreen();
  void render_sprites();
  void loop();
};
