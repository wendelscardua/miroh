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
#include "player.hpp"

#pragma clang section text = ".prg_rom_0.text"
#pragma clang section rodata = ".prg_rom_0.rodata"

const unsigned char pause_menu_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x12, 0x04, 0x17, 0x15, 0x08, 0x07, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char pause_menu_options_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x16, 0x14, 0x1b, 0x02,
    0x02, 0x02, 0x14, 0x08, 0x15, 0x17, 0x0f, 0x08, 0x02, 0x02, 0x02,
    0x02, 0x08, 0x1a, 0x0c, 0x16, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char exit_confirmation_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x16, 0x17, 0x14,
    0x10, 0x02, 0x16, 0x11, 0x02, 0x19, 0x11, 0x14, 0x0e, 0x07, 0x02,
    0x0f, 0x04, 0x12, 0x3f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char yes_no_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x1b, 0x08,
    0x15, 0x2f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x10, 0x11,
    0x3f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char story_mode_failure_text_1[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x17, 0x0b, 0x1c, 0x11, 0x0b, 0x2f, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char story_mode_failure_text_2[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x16, 0x14, 0x1b, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x08, 0x1a, 0x0c, 0x16, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char non_story_mode_match_ending_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x0a, 0x11, 0x11, 0x07, 0x02, 0x16, 0x14, 0x1b, 0x2f, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char story_mode_victory[][32 * 1] = {
    // Starlit Stables
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x6f, 0x12,
     0x04, 0x14, 0x0d, 0x0e, 0x08, 0x16, 0x04, 0x15, 0x16, 0x0c, 0x06,
     0x2f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    // Rainbow Retreat
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
     0x6f, 0x08, 0x0f, 0x04, 0x14, 0x0d, 0x04, 0x05, 0x0e, 0x08, 0x2f,
     0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    // Fairy Forest
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
     0x02, 0x6f, 0x04, 0x05, 0x17, 0x0e, 0x11, 0x17, 0x15, 0x2f, 0x02,
     0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    // Glittery Grotto
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x6f,
     0x0e, 0x0c, 0x16, 0x16, 0x08, 0x14, 0x0c, 0x09, 0x0c, 0x06, 0x2f,
     0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    // Marshmallow Mountain
    {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
     0x6f, 0x04, 0x14, 0x18, 0x08, 0x0e, 0x11, 0x17, 0x15, 0x2f, 0x02,
     0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
};

const unsigned char continue_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x06, 0x11, 0x10, 0x16, 0x0c, 0x10, 0x17, 0x08, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char confirm_retry_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x15, 0x16,
    0x04, 0x14, 0x16, 0x02, 0x16, 0x0b, 0x0c, 0x15, 0x02, 0x15, 0x16,
    0x04, 0x0a, 0x08, 0x3f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char empty_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

// TODO: variable current_stage
__attribute__((noinline)) Gameplay::Gameplay(Board &board)
    : experience(0), current_level(0), spawn_timer(SPAWN_DELAY_PER_LEVEL[0]),
      board(board),
      player(board, fixed_point(0x50, 0x00), fixed_point(0x50, 0x00)),
      polyomino(board), fruits(board), gameplay_state(GameplayState::Playing),
      input_mode(InputMode::Player), y_scroll(INTRO_SCROLL_Y) {
  set_chr_bank(0);

  set_mirroring(MIRROR_HORIZONTAL);

  banked_lambda(ASSETS_BANK, []() {
    vram_adr(PPU_PATTERN_TABLE_0);
    donut_bulk_load(level_bg_tiles[(u8)current_stage]);

    vram_adr(PPU_PATTERN_TABLE_1);
    donut_bulk_load((void *)spr_tiles);

    vram_adr(NAMETABLE_A);
    vram_unrle(level_nametables[(u8)current_stage]);

    vram_adr(NAMETABLE_C);
    vram_unrle(level_alt_nametables[(u8)current_stage]);

    Attributes::reset_shadow();
    vram_adr(NAMETABLE_A);

    pal_bg(level_bg_palettes[(u8)current_stage]);
    pal_spr(level_spr_palettes[(u8)current_stage]);
  });

  board.render();

  pal_bright(0);

  oam_clear();

  scroll(0, (unsigned int)y_scroll);

  player.refresh_score_hud();

  ppu_on_all();

  // TODO: pick based on stage
  banked_play_song(Song::Starlit_stables);

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
  fruits.render_below_player(y_scroll, player.y.whole + board.origin_y);
  player.render(y_scroll, left_wall, right_wall);
  fruits.render_above_player(y_scroll, player.y.whole + board.origin_y);
  polyomino.render(y_scroll);
  player.refresh_energy_hud(y_scroll);
  oam_hide_rest();
}

void Gameplay::pause_handler(PauseOption &pause_option, bool &yes_no_option) {
  auto pressed = get_pad_new(0) | get_pad_new(1);

  static const PauseOption NEXT_OPTION[] = {
      Gameplay::PauseOption::Resume,
      Gameplay::PauseOption::Exit,
      Gameplay::PauseOption::Retry,
  };

  static const PauseOption PREV_OPTION[] = {
      Gameplay::PauseOption::Exit,
      Gameplay::PauseOption::Retry,
      Gameplay::PauseOption::Resume,
  };

  if (pressed & (PAD_START | PAD_B)) {
    pause_option = PauseOption::Resume;
    gameplay_state = GameplayState::Playing;
    y_scroll = DEFAULT_Y_SCROLL;
    GGSound::resume();
  } else if (pressed & (PAD_RIGHT | PAD_DOWN | PAD_SELECT)) {
    pause_option = NEXT_OPTION[(u8)pause_option];
  } else if (pressed & (PAD_LEFT | PAD_UP)) {
    pause_option = PREV_OPTION[(u8)pause_option];
  } else if (pressed & PAD_A) {
    switch (pause_option) {
    case PauseOption::Exit:
      gameplay_state = GameplayState::ConfirmExit;
      multi_vram_buffer_horz(exit_confirmation_text,
                             sizeof(exit_confirmation_text),
                             PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(yes_no_text, sizeof(yes_no_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      yes_no_option = false;
      return;
      break;
    case PauseOption::Resume:
      gameplay_state = GameplayState::Playing;
      y_scroll = DEFAULT_Y_SCROLL;
      GGSound::resume();
      break;
    case PauseOption::Retry:
      // with the pause option being "Retry", this will restart the gameplay
      // loop still inside the gameplay state
      gameplay_state = GameplayState::Playing;
      break;
    }
  }

  bool toggle = (get_frame_count() & 0b10000) != 0;

  one_vram_buffer(
      pause_option == Gameplay::PauseOption::Retry
          ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
          : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(4, 5));

  one_vram_buffer(
      pause_option == Gameplay::PauseOption::Resume
          ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
          : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(12, 5));

  one_vram_buffer(
      pause_option == Gameplay::PauseOption::Exit
          ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
          : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(22, 5));
}

void Gameplay::confirm_exit_handler(bool &yes_no_option) {
  auto pressed = get_pad_new(0) | get_pad_new(1);

  bool toggle = (get_frame_count() & 0b10000) != 0;

  if (pressed & (PAD_START | PAD_B)) {
    pause_game();
    return;
  } else if (pressed &
             (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_LEFT | PAD_UP)) {
    yes_no_option = !yes_no_option;
  } else if (pressed & PAD_A) {
    if (yes_no_option) {
      current_game_state = GameState::TitleScreen;
      // TODO: go to world map instead
    } else {
      pause_game();
      return;
    }
  }

  one_vram_buffer(
      yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
                    : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(8, 5));

  one_vram_buffer(!yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE
                                           : GAMEPLAY_CURSOR_ALT_TILE)
                                 : GAMEPLAY_CURSOR_CLEAR_TILE,
                  NTADR_C(19, 5));
}

void Gameplay::gameplay_handler() {
  auto p1_pressed = get_pad_new(0);
  auto any_pressed = p1_pressed | get_pad_new(1);

  // we only spawn when there's no line clearing going on
  if (polyomino.state == Polyomino::State::Inactive &&
      !board.ongoing_line_clearing() && --spawn_timer == 0) {
    banked_lambda(GET_BANK(polyominos), [this]() { polyomino.spawn(); });
    spawn_timer = SPAWN_DELAY_PER_LEVEL[current_level];
  }

  banked_lambda(PLAYER_BANK, [this]() { player.update(input_mode); });

  if (player.state != Player::State::Dying &&
      player.state != Player::State::Dead) {
    bool blocks_placed = false;
    bool failed_to_place = false;
    u8 lines_filled = 0;

    banked_lambda(GET_BANK(polyominos), [this, &blocks_placed, &failed_to_place,
                                         &lines_filled]() {
      polyomino.handle_input(input_mode);
      polyomino.update(DROP_FRAMES_PER_LEVEL[current_level], blocks_placed,
                       failed_to_place, lines_filled);
    });

    START_MESEN_WATCH(3);
    fruits.update(player);
    STOP_MESEN_WATCH(3);

    if (current_controller_scheme == ControllerScheme::OnePlayer &&
        polyomino.state != Polyomino::State::Active) {
      if (input_mode == InputMode::Polyomino) {
        input_mode = InputMode::Player;
      }
    }

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

    if (any_pressed & PAD_START) {
      pause_game();
    } else if (p1_pressed & PAD_SELECT) {
      if (input_mode == InputMode::Player) {
        input_mode = InputMode::Polyomino;
      } else {
        input_mode = InputMode::Player;
      }
    }
  } else if (player.state == Player::State::Dead && (any_pressed & PAD_START)) {
    current_game_state = GameState::TitleScreen;
  }
}

void Gameplay::pause_game() {
  gameplay_state = GameplayState::Paused;
  y_scroll = PAUSE_SCROLL_Y;
  GGSound::pause();
  multi_vram_buffer_horz(pause_menu_text, sizeof(pause_menu_text),
                         PAUSE_MENU_POSITION);
  multi_vram_buffer_horz(pause_menu_options_text,
                         sizeof(pause_menu_options_text),
                         PAUSE_MENU_OPTIONS_POSITION);
}

void Gameplay::loop() {
  static bool no_lag_frame = true;
  extern volatile char FRAME_CNT1;
  PauseOption pause_option = PauseOption::Resume;
  bool yes_no_option = false;

  while (current_game_state == GameState::Gameplay) {
    ppu_wait_nmi();

    START_MESEN_WATCH(1);

    u8 frame = FRAME_CNT1;

    Attributes::enable_vram_buffer();

    InputMode old_mode = input_mode;

    pad_poll(0);
    pad_poll(1);

    switch (gameplay_state) {
    case GameplayState::Playing:
      if (pause_option == PauseOption::Retry) {
        return; // escapes from gameplay loop; causing gameplay to restart since
                // we're still on the same game state
      }
      Gameplay::gameplay_handler();
      break;
    case GameplayState::Paused:
      Gameplay::pause_handler(pause_option, yes_no_option);
      break;
    case GameplayState::ConfirmExit:
      Gameplay::confirm_exit_handler(yes_no_option);
      break;
    case GameplayState::ConfirmRetry:
    case GameplayState::ConfirmContinue:
      break;
    }

    if (input_mode != old_mode) {
      banked_play_sfx(SFX::Number2, GGSound::SFXPriority::One);
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
