#include "world-map.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "soundtrack.hpp"
#include <nesdoug.h>

#pragma clang section text = ".prg_rom_2.text"
#pragma clang section rodata = ".prg_rom_2.rodata"

WorldMap::WorldMap(Board &board) : board(board) {
  banked_lambda(ASSETS_BANK, []() { load_map_assets(); });

  pal_bright(0);

  oam_clear();

  render_sprites();

  scroll(0, 0);

  ppu_on_all();

  banked_play_song(Song::Intro_music);

  pal_fade_to(0, 4);
}

WorldMap::~WorldMap() {
  pal_fade_to(4, 0);
  ppu_off();
}

void WorldMap::loop() {
  while (current_game_state == GameState::WorldMap) {
    ppu_wait_nmi();

    pad_poll(0);
    pad_poll(1);

    rand16();

    u8 pressed = get_pad_new(0) | get_pad_new(1);

    if (pressed) {
      banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
      current_game_state = GameState::Gameplay;
      banked_lambda(Board::MAZE_BANK, [this]() { board.generate_maze(); });
    }

    render_sprites();
  }
}

void WorldMap::render_sprites() {}
