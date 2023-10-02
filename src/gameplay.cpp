
#include <bank.h>
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "chr-data.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"
#include "nametables.hpp"
#include "palettes.hpp"
#include "player.hpp"
#include "gameplay.hpp"

Gameplay::Gameplay() :
  spawn_timer(INITIAL_SPAWN_DELAY),
  board(BOARD_X_ORIGIN, BOARD_Y_ORIGIN),
  player(board, fixed_point(0x50, 0x00), fixed_point(0x50, 0x00)),
  polyomino(board, false),
  fruits(board),
  input_mode(InputMode::Player) {
    set_chr_bank(0);

    set_prg_bank(GET_BANK(bg_chr));
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu((void *)&bg_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(sprites_chr));
    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)&sprites_chr, PPU_PATTERN_TABLE_SIZE / 64);

    set_prg_bank(GET_BANK(title_nam));

    vram_adr(NAMETABLE_D);
    vram_write(game_over_nam, 1024);

    vram_adr(NAMETABLE_A);
    vram_write(gameplay_nam, 1024);

    board.render();

    set_prg_bank(GET_BANK(bg_palette));
    pal_bg(bg_palette);
    set_prg_bank(GET_BANK(sprites_palette));
    pal_spr(sprites_palette);

    pal_bright(0);

    oam_clear();

    scroll(0, 0);

    ppu_on_all();

    pal_fade_to(0, 4);
}

Gameplay::~Gameplay() {
    pal_fade_to(4, 0);
    ppu_off();
}

void Gameplay::render() {
  oam_clear();
  player.render();
  if (player.state == Player::State::Dying ||
      player.state == Player::State::Dead) return;

  if (get_frame_count() & 0b1) {
    fruits.render();
    polyomino.render();
  } else {
    polyomino.render();
    fruits.render();
  }
}

extern volatile char FRAME_CNT1;
static bool no_lag_frame;

void Gameplay::loop() {
  while(current_mode == GameMode::Gameplay) {
    ppu_wait_nmi();
    u8 frame = FRAME_CNT1;

    // we only spawn when there's no line clearing going on
    if (!board.ongoing_line_clearing()
        && !polyomino.active
        && --spawn_timer == 0) {
      polyomino.spawn();
      spawn_timer = INITIAL_SPAWN_DELAY; // TODO make this go faster
    }

    pad_poll(0);

    u8 pressed = get_pad_new(0);
    u8 held = pad_state(0);

    if (pressed & PAD_SELECT) {
      switch(input_mode) {
      case InputMode::Player:
        input_mode = InputMode::Polyomino;
        break;
      case InputMode::Polyomino:
        input_mode = InputMode::Player;
        break;
      }
    }

    player.update(input_mode, pressed, held);
    if (player.state != Player::State::Dying && player.state != Player::State::Dead) {
      bool blocks_placed = false;
      u8 lines_filled = 0;
      polyomino.update(input_mode, pressed, held, blocks_placed, lines_filled);
      fruits.update(player, blocks_placed);
    } else if (player.state == Player::State::Dead && (pressed & PAD_START)) {
      current_mode = GameMode::TitleScreen;
    }

    if (no_lag_frame) {
      render();
    } else {
#ifndef NDEBUG
      putchar('X');
#endif
    }

    no_lag_frame = frame == FRAME_CNT1;

    #ifndef NDEBUG
    gray_line();
    #endif
  }
}
