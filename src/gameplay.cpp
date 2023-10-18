#include "assets.hpp"
#include <bank.h>
#ifndef NDEBUG
#include <cstdio>
#endif
#include <nesdoug.h>
#include <neslib.h>

#include "attributes.hpp"
#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "fixed-point.hpp"
#include "fruits.hpp"
#include "gameplay.hpp"
#include "ggsound.hpp"
#include "input-mode.hpp"
#include "metasprites.hpp"
#include "player.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

// TODO: variable current_location
__attribute__((noinline)) Gameplay::Gameplay()
    : experience(0), current_level(0), spawn_timer(SPAWN_DELAY_PER_LEVEL[0]),
      pause_option(0), board(BOARD_X_ORIGIN, BOARD_Y_ORIGIN),
      player(board, fixed_point(0x50, 0x00), fixed_point(0x50, 0x00)),
      polyomino(board), fruits(board), input_mode(InputMode::Player),
      current_location(Location::StarlitStables) {
  set_chr_bank(0);

  banked_lambda(ASSETS_BANK, [this]() {
    vram_adr(PPU_PATTERN_TABLE_0);
    Donut::decompress_to_ppu(level_bg_tiles[(u8)current_location],
                             PPU_PATTERN_TABLE_SIZE / 64);

    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu(level_spr_tiles[(u8)current_location],
                             PPU_PATTERN_TABLE_SIZE / 64);

    vram_adr(NAMETABLE_D);
    vram_write(game_over_nam, 1024);

    vram_adr(NAMETABLE_A);
    vram_write(level_nametables[(u8)current_location], 1024);

    Attributes::reset_shadow();
    vram_adr(NAMETABLE_A);

    pal_bg(level_bg_palettes[(u8)current_location]);
    pal_spr(level_spr_palettes[(u8)current_location]);
  });

  board.render();

  pal_bright(0);

  oam_clear();

  scroll(0, DEFAULT_SCROLL_Y);

  ppu_on_all();

  pal_fade_to(0, 4);
}

__attribute__((noinline)) Gameplay::~Gameplay() {
  pal_fade_to(4, 0);
  ppu_off();
}

void Gameplay::render() {
  oam_clear();
  if (player.state == Player::State::Dying ||
      player.state == Player::State::Dead) {
    player.render();
    return;
  }

  switch (get_frame_count() & 0b11) {
  case 0:
    player.render();
    fruits.render();
    polyomino.render();
    polyomino.render_next();
    break;
  case 1:
    polyomino.render_next();
    polyomino.render();
    player.render();
    fruits.render();
    break;
  case 2:
    player.render();
    polyomino.render();
    polyomino.render_next();
    fruits.render();
    break;
  default:
    polyomino.render_next();
    polyomino.render();
    fruits.render();
    player.render();
    break;
  }
}

void Gameplay::paused_render() {
  oam_clear();
  if (pause_option == 0) {
    banked_oam_meta_spr(0x30, 0x70, metasprite_MenuminoL);
  } else {
    banked_oam_meta_spr(0x80, 0x70, metasprite_MenuminoR);
  }
}

extern volatile char FRAME_CNT1;
static bool no_lag_frame = true;

void Gameplay::loop() {
  while (current_mode == GameMode::Gameplay) {
    ppu_wait_nmi();

    u8 frame = FRAME_CNT1;

    Attributes::enable_vram_buffer();

    InputMode old_mode = input_mode;

    pad_poll(0);

    u8 pressed = get_pad_new(0);
    u8 held = pad_state(0);

    if (input_mode == InputMode::Pause) {
      if (pressed & (PAD_START | PAD_B)) {
        input_mode = InputMode::Player;
        pressed &= ~(PAD_START | PAD_B);
        set_scroll_y(DEFAULT_SCROLL_Y);
        banked_lambda(GET_BANK(song_list), []() { GGSound::resume(); });
      } else if (pressed &
                 (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN | PAD_SELECT)) {
        pause_option = 1 - pause_option;
      } else if (pressed & PAD_A) {
        banked_lambda(GET_BANK(song_list), []() { GGSound::resume(); });
        input_mode = InputMode::Player;
        set_scroll_y(DEFAULT_SCROLL_Y);
        if (pause_option == 1) {
          current_mode = GameMode::TitleScreen;
        }
      }
    }

    if (input_mode == InputMode::Pause) {
      paused_render();
      continue;
    }

    // we only spawn when there's no line clearing going on
    if (polyomino.state == Polyomino::State::Inactive &&
        !board.ongoing_line_clearing() && --spawn_timer == 0) {
      banked_lambda(GET_BANK(polyominos), [this]() { polyomino.spawn(); });
      spawn_timer = SPAWN_DELAY_PER_LEVEL[current_level];
    }
    banked_lambda(PLAYER_BANK, [this, pressed, held]() {
      player.update(input_mode, pressed, held);
    });

    if (player.state != Player::State::Dying &&
        player.state != Player::State::Dead) {
      bool blocks_placed = false;
      bool failed_to_place = false;
      u8 lines_filled = 0;
      banked_lambda(GET_BANK(polyominos), [pressed, this, held, &blocks_placed,
                                           &failed_to_place, &lines_filled]() {
        polyomino.handle_input(input_mode, pressed, held);
        polyomino.update(DROP_FRAMES_PER_LEVEL[current_level], blocks_placed,
                         failed_to_place, lines_filled);
      });
      fruits.update(player, blocks_placed, lines_filled);

      if (lines_filled) {
        u16 points = 10 * (2 * lines_filled - 1);
        player.score += points;
        add_experience(points);
      } else if (blocks_placed) {
        player.score += 1;
        add_experience(1);
      }

      if (failed_to_place) {
        player.hunger_upkeep(3 * HUNGER_TICKS);
      }

      switch (input_mode) {
      case InputMode::Player:
        if (pressed & (PAD_SELECT | PAD_A | PAD_B)) {
          input_mode = InputMode::Polyomino;
          pressed &= ~(PAD_SELECT | PAD_A | PAD_B);
        } else if (pressed & PAD_START) {
          input_mode = InputMode::Pause;
          set_scroll_y(PAUSE_SCROLL_Y);
          banked_lambda(GET_BANK(song_list), []() { GGSound::pause(); });
        }
        break;
      case InputMode::Polyomino:
        if (pressed & (PAD_SELECT)) {
          input_mode = InputMode::Player;
          pressed &= ~(PAD_SELECT);
        } else if (pressed & PAD_START) {
          input_mode = InputMode::Pause;
          set_scroll_y(PAUSE_SCROLL_Y);
          banked_lambda(GET_BANK(song_list), []() { GGSound::pause(); });
        }
        break;
      default:
      }
    } else if (player.state == Player::State::Dead && (pressed & PAD_START)) {
      current_mode = GameMode::TitleScreen;
    }

    if (input_mode != old_mode) {
      banked_lambda(GET_BANK(sfx_list), []() {
        GGSound::play_sfx(SFX::Toggle_input, GGSound::SFXPriority::One);
      });
    }

    Attributes::flush_vram_update();

    extern u8 VRAM_INDEX;
    if (VRAM_INDEX + 16 < 64) {
      player.refresh_score_hud();
    }

    if (no_lag_frame) {
      render();
    } else {
#ifndef NDEBUG
      putchar('X');
      putchar('\n');
#endif
    }

    no_lag_frame = frame == FRAME_CNT1;
#ifndef NDEBUG
    gray_line();
#endif
  }
}

void Gameplay::add_experience(u16 exp) {
  if (current_level < MAX_LEVEL) {
    experience += exp;
    while (experience >= LEVEL_UP_POINTS && current_level < MAX_LEVEL) {
      experience -= LEVEL_UP_POINTS;
      current_level++;
      // TODO: level up fanfare ?
    }
  }
}
