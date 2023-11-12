#include <mapper.h>
#include <nesdoug.h>
#include <neslib.h>

#include "assets.hpp"
#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "metasprites.hpp"
#include "soundtrack.hpp"
#include "title-screen.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

const TitleScreen::MenuOption previous_option[] = {
    TitleScreen::MenuOption::HowToPlay,  // OnePlayer
    TitleScreen::MenuOption::OnePlayer,  // TwoPlayers
    TitleScreen::MenuOption::TwoPlayers, // HowToPlay
};

const TitleScreen::MenuOption next_option[] = {
    TitleScreen::MenuOption::TwoPlayers, // OnePlayer
    TitleScreen::MenuOption::HowToPlay,  // TwoPlayers
    TitleScreen::MenuOption::OnePlayer,  // HowToPlay
};

const u8 menu_y_position[] = {
    0x96, // OnePlayer
    0xa6, // TwoPlayers
    0xb6, // HowToPlay
};

__attribute__((noinline)) TitleScreen::TitleScreen(Board &board)
    : state(State::MainMenu), current_option(MenuOption::OnePlayer),
      board(board), x_scroll(TITLE_SCROLL) {
  set_chr_bank(0);

  set_mirroring(MIRROR_VERTICAL);

  banked_lambda(ASSETS_BANK, []() {
    vram_adr(PPU_PATTERN_TABLE_0);
    donut_bulk_load((void *)title_bg_tiles);

    vram_adr(PPU_PATTERN_TABLE_1);
    donut_bulk_load((void *)spr_tiles);

    vram_adr(NAMETABLE_A);
    vram_unrle(title_alt_nametable);

    vram_adr(NAMETABLE_B);
    vram_unrle(title_nametable);

    pal_bg(title_bg_palette);
    pal_spr(title_spr_palette);
  });

  pal_bright(0);

  oam_clear();

  scroll((u16)x_scroll, 0);

  ppu_on_all();

  // TODO: pick title song
  banked_play_song(Song::Baby_bullhead);

  pal_fade_to(0, 4);
}

__attribute__((noinline)) TitleScreen::~TitleScreen() {
  pal_fade_to(4, 0);
  ppu_off();
}

__attribute__((noinline)) void TitleScreen::loop() {
  while (current_mode == GameMode::TitleScreen) {
    ppu_wait_nmi();

    pad_poll(0);

    rand16();

    u8 pressed = get_pad_new(0);
    bool bobbing_flag = get_frame_count() & 0b10000;

    switch (state) {
    case State::MainMenu:
      if (pressed & (PAD_UP | PAD_LEFT)) {
        current_option = previous_option[(u8)current_option];
      } else if (pressed & (PAD_DOWN | PAD_RIGHT | PAD_SELECT | PAD_B)) {
        current_option = next_option[(u8)current_option];
      } else if (pressed & (PAD_START | PAD_A)) {
        switch (current_option) {
        case MenuOption::OnePlayer:
        case MenuOption::TwoPlayers:
          // TODO: pick between 1p and 2p modes
          current_mode = GameMode::Gameplay;
          banked_lambda(Board::MAZE_BANK, [this]() { board.generate_maze(); });
          break;
        case MenuOption::HowToPlay:
          // TODO: scroll to how to play
          state = State::HowToPlay;
          break;
        }
      }
      banked_oam_meta_spr(0x48, menu_y_position[(u8)current_option],
                          bobbing_flag ? metasprite_AvocadoHigh
                                       : metasprite_AvocadoLow);
      break;
    case State::HowToPlay:
      break;
    }

    banked_oam_meta_spr_horizontal(JR_X_POSITION - x_scroll, JR_Y_POSITION,
                                   metasprite_TitleJR);

    oam_hide_rest();
  }
}
