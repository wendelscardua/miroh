#include "assets.hpp"
#include "board.hpp"
#include "log.hpp"
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
      polyomino(board), fruits(board, current_level),
      input_mode(InputMode::Player), current_location(Location::StarlitStables),
      y_scroll(INTRO_SCROLL_Y) {
  set_chr_bank(0);

  banked_lambda(ASSETS_BANK, [this]() {
    vram_adr(PPU_PATTERN_TABLE_0);
    donut_bulk_load(level_bg_tiles[(u8)current_location]);

    vram_adr(PPU_PATTERN_TABLE_1);
    donut_bulk_load(level_spr_tiles[(u8)current_location]);

    vram_adr(NAMETABLE_A);
    vram_unrle(level_nametables[(u8)current_location]);

    vram_adr(NAMETABLE_C);
    vram_unrle(level_alt_nametables[(u8)current_location]);

    Attributes::reset_shadow();
    vram_adr(NAMETABLE_A);

    pal_bg(level_bg_palettes[(u8)current_location]);
    pal_spr(level_spr_palettes[(u8)current_location]);
  });

  board.render();

  pal_bright(0);

  oam_clear();

  scroll(0, (unsigned int)y_scroll);

  player.refresh_energy_hud();
  player.refresh_score_hud();

  ppu_on_all();

  // TODO: pick based on stage
  banked_play_song(Song::Marshmallow_mountain);

  pal_fade_to(0, 4);

  for (u16 waiting_frames = 0; waiting_frames < INTRO_DELAY; waiting_frames++) {
    pad_poll(0);
    pad_poll(1);
    if (get_pad_new(0) | get_pad_new(1)) {
      break;
    }
    ppu_wait_nmi();
  }

  while (y_scroll < Gameplay::DEFAULT_Y_SCROLL) {
    ppu_wait_nmi();
    y_scroll++;
    if (y_scroll == -0x20) {
      y_scroll = 0;
    }
    scroll(0, (unsigned int)y_scroll);
  }
}

__attribute__((noinline)) Gameplay::~Gameplay() {
  pal_fade_to(4, 0);
  ppu_off();
}

void Gameplay::render() {
  scroll(0, (unsigned int)y_scroll);
  bool left_wall = false, right_wall = false;
  if (player.state == Player::State::Moving) {
    u8 row = (u8)(player.y.round() >> 4) + 1;
    u8 col = (u8)(player.x.round() >> 4);
    if (row < HEIGHT) {
      auto cell = board.cell_at(row, col);
      left_wall = cell.left_wall;
      right_wall = cell.right_wall;
    }
  }
  player.render(y_scroll, left_wall, right_wall);
  fruits.render(y_scroll);
  polyomino.render(y_scroll);
  oam_hide_rest();
}

void Gameplay::paused_render() {
  oam_clear();
  if (pause_option == 0) {
    banked_oam_meta_spr(0x30, 0x70, metasprite_UniRightIdle);
  } else {
    banked_oam_meta_spr(0x80, 0x70, metasprite_UniRightIdle);
  }
}

extern volatile char FRAME_CNT1;
static bool no_lag_frame = true;

void Gameplay::loop() {
  while (current_mode == GameMode::Gameplay) {
    ppu_wait_nmi();

    START_MESEN_WATCH(1);

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
        y_scroll = DEFAULT_Y_SCROLL;
        GGSound::resume();
      } else if (pressed &
                 (PAD_LEFT | PAD_RIGHT | PAD_UP | PAD_DOWN | PAD_SELECT)) {
        pause_option = 1 - pause_option;
      } else if (pressed & PAD_A) {
        GGSound::resume();
        input_mode = InputMode::Player;
        y_scroll = DEFAULT_Y_SCROLL;
        if (pause_option == 1) {
          current_mode = GameMode::TitleScreen;
        }
      }
    } else {
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

        banked_lambda(GET_BANK(polyominos), [pressed, this, held,
                                             &blocks_placed, &failed_to_place,
                                             &lines_filled]() {
          polyomino.handle_input(input_mode, pressed, held);
          polyomino.update(DROP_FRAMES_PER_LEVEL[current_level], blocks_placed,
                           failed_to_place, lines_filled);
        });

        fruits.update(player, blocks_placed, lines_filled);

        if (lines_filled) {
          u16 points = 10 * (2 * lines_filled - 1);
          player.score += points;
          if (player.score > 9999) {
            player.score = 9999;
          }
          player.lines += lines_filled;
          if (player.lines > 99) {
            player.lines = 99;
          }
          add_experience(points);
        } else if (blocks_placed) {
          player.score += 1;
          add_experience(1);
        }

        if (failed_to_place) {
          player.energy_upkeep(3 * Player::ENERGY_TICKS);
        }

        switch (input_mode) {
        case InputMode::Player:
          if (pressed & (PAD_SELECT | PAD_A | PAD_B)) {
            input_mode = InputMode::Polyomino;
            pressed &= ~(PAD_SELECT | PAD_A | PAD_B);
          } else if (pressed & PAD_START) {
            input_mode = InputMode::Pause;
            y_scroll = PAUSE_SCROLL_Y;
            GGSound::pause();
          }
          break;
        case InputMode::Polyomino:
          if (pressed & (PAD_SELECT)) {
            input_mode = InputMode::Player;
            pressed &= ~(PAD_SELECT);
          } else if (pressed & PAD_START) {
            input_mode = InputMode::Pause;
            y_scroll = PAUSE_SCROLL_Y;
            GGSound::pause();
          }
          break;
        default:
        }
      } else if (player.state == Player::State::Dead && (pressed & PAD_START)) {
        current_mode = GameMode::TitleScreen;
      }

      if (input_mode != old_mode) {
        banked_play_sfx(SFX::Number2, GGSound::SFXPriority::One);
      }
    }

    Attributes::flush_vram_update();

    extern u8 VRAM_INDEX;
    if (VRAM_INDEX + 16 < 64) {
      player.refresh_score_hud();
    }

    if (no_lag_frame) {
      START_MESEN_WATCH(2);
      render();
      STOP_MESEN_WATCH(2);
    } else {
#ifndef NDEBUG
      putchar('X');
      putchar('\n');
#endif
    }

    STOP_MESEN_WATCH(1);

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
