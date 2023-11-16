#include "animation.hpp"
#include "assets.hpp"
#include "board.hpp"
#include "log.hpp"
#include "polyomino.hpp"
#include "soundtrack.hpp"
#include "zx02.hpp"
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
#include "unicorn.hpp"

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

const unsigned char story_mode_failure_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x17, 0x0b, 0x1c, 0x11, 0x0b, 0x2f, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char retry_exit_confirmation_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x16, 0x14, 0x1b, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x08, 0x1a, 0x0c, 0x16, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char non_story_mode_match_ending_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x0a, 0x11, 0x11, 0x07, 0x02, 0x16, 0x14, 0x1b, 0x2f, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const unsigned char story_mode_victory_text_per_stage[][32 * 1] = {
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

const unsigned char retry_confirmation_text[] = {
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x08, 0x15, 0x16,
    0x04, 0x14, 0x16, 0x02, 0x16, 0x0b, 0x0c, 0x15, 0x02, 0x15, 0x16,
    0x04, 0x0a, 0x08, 0x3f, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

const Song song_per_stage[] = {
    Song::Starlit_stables,      // StarlitStables
    Song::Rainbow_retreat,      // RainbowRetreat
    Song::Fairy_flight,         // FairyForest
    Song::Glitter_grotto,       // GlitteryGrotto
    Song::Marshmallow_mountain, // MarshmallowMountain
};

__attribute__((noinline)) Gameplay::Gameplay(Board &board)
    : experience(0), current_level(0), spawn_timer(SPAWN_DELAY_PER_LEVEL[0]),
      board(board),
      unicorn(board, fixed_point(0x50, 0x00), fixed_point(0x50, 0x00)),
      polyomino(board), fruits(board), gameplay_state(GameplayState::Playing),
      input_mode(current_controller_scheme == ControllerScheme::OnePlayer
                     ? InputMode::Unicorn
                     : InputMode::Polyomino),
      yes_no_option(false), pause_option(PauseOption::Resume),
      y_scroll(INTRO_SCROLL_Y), goal_counter(0) {
  set_chr_bank(0);

  set_mirroring(MIRROR_HORIZONTAL);

  banked_lambda(ASSETS_BANK, []() {
    vram_adr(PPU_PATTERN_TABLE_0);
    u8 bg_blocks = level_bg_tile_blocks[(u8)current_stage];
    Donut::decompress_to_ppu((void *)base_bg_tiles, 4096 / 64 - bg_blocks);
    if (bg_blocks > 0) {
      Donut::decompress_to_ppu(level_bg_tiles[(u8)current_stage], bg_blocks);
    }

    // mode labels start at tile $84, both require 1 donut block (64 bytes)
    if (current_game_mode == GameMode::TimeTrial) {
      vram_adr(PPU_PATTERN_TABLE_0 + 0x84 * 0x10);
      Donut::decompress_to_ppu((void *)time_label_tiles, 1);
    } else if (current_game_mode == GameMode::Endless) {
      vram_adr(PPU_PATTERN_TABLE_0 + 0x84 * 0x10);
      Donut::decompress_to_ppu((void *)level_label_tiles, 1);
    }

    vram_adr(PPU_PATTERN_TABLE_1);
    Donut::decompress_to_ppu((void *)spr_tiles, 4096 / 64);

    set_chr_bank(1);
    vram_adr(0);
    zx02_decompress_to_vram(level_nametables[(u8)current_stage], 0);

    for (u16 i = 0; i < 1024; i += 64) {
      vram_adr(i);
      vram_read(donut_block_buffer, 64);
      vram_adr(NAMETABLE_A + i);
      vram_write(donut_block_buffer, 64);
    }
    for (u16 i = 0; i < 1024; i += 64) {
      vram_adr(1024 + i);
      vram_read(donut_block_buffer, 64);
      vram_adr(NAMETABLE_C + i);
      vram_write(donut_block_buffer, 64);
    }

    set_chr_bank(0);

    if (current_game_mode == GameMode::TimeTrial) {
      vram_adr(NTADR_C(6, 21));
      vram_write(time_trial_prompt[0], 20);
      vram_adr(NTADR_C(6, 23));
      vram_write(time_trial_prompt[1], 20);
      vram_adr(NTADR_C(6, 25));
      vram_write(time_trial_prompt[2], 20);
    } else if (current_game_mode == GameMode::Endless) {
      vram_adr(NTADR_C(6, 21));
      vram_write(endless_prompt[0], 20);
      vram_adr(NTADR_C(6, 23));
      vram_write(endless_prompt[1], 20);
      vram_adr(NTADR_C(6, 25));
      vram_write(endless_prompt[2], 20);
    }

    Attributes::reset_shadow();
    vram_adr(NAMETABLE_A);

    pal_bg(level_bg_palettes[(u8)current_stage]);
    pal_spr(level_spr_palettes[(u8)current_stage]);
  });

  board.render();

  pal_bright(0);

  oam_clear();

  scroll(0, (unsigned int)y_scroll);

  unicorn.refresh_score_hud();

  initialize_goal();

  ppu_on_all();

  banked_play_song(song_per_stage[(u8)current_stage]);

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
  Animation::paused = (gameplay_state != GameplayState::Playing &&
                       gameplay_state != GameplayState::Swapping);
  scroll(0, (unsigned int)y_scroll);
  bool left_wall = false, right_wall = false;
  if (unicorn.state == Unicorn::State::Moving) {
    u8 row = (u8)(unicorn.y.round() >> 4) + 1;
    u8 col = (u8)(unicorn.x.round() >> 4);
    if (row < HEIGHT) {
      auto cell = board.cell_at(row, col);
      left_wall = cell.left_wall;
      right_wall = cell.right_wall;
    }
  }
  fruits.render_below_player(y_scroll, unicorn.y.whole + board.origin_y);
  if (gameplay_state != GameplayState::Swapping ||
      swap_frames[swap_index].display_unicorn) {
    unicorn.render(y_scroll, left_wall, right_wall);
  }
  fruits.render_above_player(y_scroll, unicorn.y.whole + board.origin_y);
  if (gameplay_state != GameplayState::Swapping ||
      swap_frames[swap_index].display_polyomino) {
    polyomino.render(y_scroll);
  }
  unicorn.refresh_energy_hud(y_scroll);

  if (SPRID) {
    // if we rendered 64 sprites already, SPRID will have wrapped around back to
    // zero. in that case oam_hide_rest() would've hidden everyone

    oam_hide_rest();
  }
}

void Gameplay::initialize_goal() {
  /*
   * Story mode goals:
   *  - Starlit Stables: clear 12 lines
   *  - Rainbow Retreat: eat 24 snacks
   *  - Fairy Forest: place 60 blocks
   *  - Glittery Grotto: score 200 points
   *  - Marshmallow Mountain: defeat Miroh Jr.
   */

  switch (current_game_mode) {
  case GameMode::Story:
    switch (current_stage) {
    case Stage::StarlitStables:
      lines_left = LINES_GOAL;
      break;
    case Stage::RainbowRetreat:
      snacks_left = SNACKS_GOAL;
      break;
    case Stage::FairyForest:
      blocks_left = BLOCKS_GOAL;
      break;
    case Stage::GlitteryGrotto:
      points_left = SCORE_GOAL;
    case Stage::MarshmallowMountain:
      // TODO
      break;
    }
  case GameMode::Endless:
    break;
  case GameMode::TimeTrial:
    time_trial_frames = 0;
    time_trial_seconds = TIME_TRIAL_DURATION;
    break;
  }
  u8 goal_counter_text[2];
  if (current_game_mode == GameMode::Endless ||
      current_stage == Stage::GlitteryGrotto) {
    u8_to_text(goal_counter_text, current_level + 1);
    multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));
  } else {
    u8_to_text(goal_counter_text, (u8)goal_counter);
    multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));
  }
}

void Gameplay::ease_scroll(const int target) {
  if (y_scroll < target) {
    y_scroll += 0x08;
  } else if (y_scroll > target) {
    y_scroll -= 0x08;
  }
}

void Gameplay::pause_handler() {
  ease_scroll(PAUSE_SCROLL_Y);

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

  if (any_pressed & (PAD_START | PAD_B)) {
    pause_option = PauseOption::Resume;
    gameplay_state = GameplayState::Playing;
    GGSound::resume();
    banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::One);
  } else if (any_pressed & (PAD_RIGHT | PAD_DOWN | PAD_SELECT)) {
    pause_option = NEXT_OPTION[(u8)pause_option];
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
  } else if (any_pressed & (PAD_LEFT | PAD_UP)) {
    pause_option = PREV_OPTION[(u8)pause_option];
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
  } else if (any_pressed & PAD_A) {
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
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
      GGSound::resume();
      break;
    case PauseOption::Retry:
      gameplay_state = GameplayState::ConfirmRetry;
      multi_vram_buffer_horz(retry_confirmation_text,
                             sizeof(retry_confirmation_text),
                             PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(yes_no_text, sizeof(yes_no_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      yes_no_option = false;
      return;
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

void Gameplay::yes_no_cursor() {
  bool toggle = (get_frame_count() & 0b10000) != 0;

  one_vram_buffer(
      yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
                    : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(8, 5));

  one_vram_buffer(!yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE
                                           : GAMEPLAY_CURSOR_ALT_TILE)
                                 : GAMEPLAY_CURSOR_CLEAR_TILE,
                  NTADR_C(19, 5));
}

void Gameplay::confirm_exit_handler() {
  ease_scroll(PAUSE_SCROLL_Y);

  if (any_pressed & PAD_B) {
    banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::One);
    pause_game();
    return;
  } else if (any_pressed &
             (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_LEFT | PAD_UP)) {
    yes_no_option = !yes_no_option;
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
  } else if (any_pressed & (PAD_A | PAD_START)) {
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
    if (yes_no_option) {
      current_game_state = GameState::TitleScreen;
      // TODO: go to world map instead
    } else {
      pause_game();
    }
    return;
  }

  yes_no_cursor();
}

void Gameplay::confirm_retry_handler() {
  if (any_pressed & PAD_B) {
    pause_game();
    return;
  } else if (any_pressed &
             (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_LEFT | PAD_UP)) {
    yes_no_option = !yes_no_option;
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
  } else if (any_pressed & (PAD_A | PAD_START)) {
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
    if (yes_no_option) {
      gameplay_state = GameplayState::Retrying;
    } else {
      pause_game();
    }
    return;
  }

  yes_no_cursor();
}

void Gameplay::confirm_continue_handler() {
  ease_scroll(PAUSE_SCROLL_Y);

  if (any_pressed & (PAD_A | PAD_START)) {
    // TODO: world map
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
    current_game_state = GameState::TitleScreen;
    return;
  }

  bool toggle = (get_frame_count() & 0b10000) != 0;

  one_vram_buffer((toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE),
                  NTADR_C(11, 5));
}

void Gameplay::retry_exit_handler() {
  ease_scroll(PAUSE_SCROLL_Y);

  if (any_pressed & (PAD_RIGHT | PAD_DOWN | PAD_SELECT | PAD_LEFT | PAD_UP)) {
    banked_play_sfx(SFX::Uioptionscycle, GGSound::SFXPriority::One);
    yes_no_option = !yes_no_option;
  } else if (any_pressed & (PAD_A | PAD_START)) {
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
    if (yes_no_option) {
      gameplay_state = GameplayState::Retrying;
    } else {
      // TODO: world map instead
      current_game_state = GameState::TitleScreen;
    }
    return;
  }

  bool toggle = (get_frame_count() & 0b10000) != 0;

  one_vram_buffer(
      yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE : GAMEPLAY_CURSOR_ALT_TILE)
                    : GAMEPLAY_CURSOR_CLEAR_TILE,
      NTADR_C(4, 5));

  one_vram_buffer(!yes_no_option ? (toggle ? GAMEPLAY_CURSOR_TILE
                                           : GAMEPLAY_CURSOR_ALT_TILE)
                                 : GAMEPLAY_CURSOR_CLEAR_TILE,
                  NTADR_C(22, 5));
}

void Gameplay::gameplay_handler() {
  ease_scroll(DEFAULT_Y_SCROLL);

  blocks_were_placed = false;
  failed_to_place = false;
  lines_cleared = 0;
  snack_was_eaten = false;

  if (any_pressed & PAD_START) {
    pause_game();
    return;
  } else if (any_pressed & PAD_SELECT) {
    swap_inputs();
  }

  bool line_clearing_in_progress = board.ongoing_line_clearing(
      polyomino.state == Polyomino::State::Settling);

  // we only spawn when there's no line clearing going on
  if (polyomino.state == Polyomino::State::Inactive &&
      !line_clearing_in_progress && --spawn_timer == 0) {
    banked_lambda(GET_BANK(polyominos), [this]() { polyomino.spawn(); });
    spawn_timer = SPAWN_DELAY_PER_LEVEL[current_level];
  }

  banked_lambda(PLAYER_BANK,
                [this]() { unicorn.update(unicorn_pressed, unicorn_held); });

  banked_lambda(GET_BANK(polyominos), [this]() {
    polyomino.handle_input(polyomino_pressed, polyomino_held);
    polyomino.update(DROP_FRAMES_PER_LEVEL[current_level], blocks_were_placed,
                     failed_to_place, lines_cleared);
  });

  fruits.update(unicorn, snack_was_eaten);

  if (current_controller_scheme == ControllerScheme::OnePlayer &&
      polyomino.state != Polyomino::State::Active &&
      input_mode == InputMode::Polyomino) {
    swap_inputs();
  }

  if (lines_cleared) {
    u16 points = 10 * (2 * lines_cleared - 1);
    unicorn.score += points;
    if (unicorn.score > 9999) {
      unicorn.score = 9999;
    }
    add_experience(points);
  } else if (blocks_were_placed) {
    unicorn.score += 1;
    add_experience(1);
  }

  if (gameplay_state == GameplayState::Playing ||
      gameplay_state == GameplayState::Swapping) {
    game_mode_upkeep(line_clearing_in_progress || blocks_were_placed ||
                     polyomino.state != Polyomino::State::Inactive);
  }
}

void Gameplay::game_mode_upkeep(bool stuff_in_progress) {
  u8 goal_counter_text[2];
  switch (current_game_mode) {
  case GameMode::Story:
    /*
     * Story mode goals:
     *  - Starlit Stables: clear 12 lines
     *  - Rainbow Retreat: eat 24 snacks
     *  - Fairy Forest: place 60 blocks
     *  - Glittery Grotto: score 200 points
     *  - Marshmallow Mountain: defeat Miroh Jr.
     */
    switch (current_stage) {
    case Stage::StarlitStables:
      if (lines_cleared > lines_left) {
        lines_left = 0;
      } else {
        lines_left -= lines_cleared;
      }
      break;
    case Stage::RainbowRetreat:
      if (snack_was_eaten && snacks_left > 0) {
        snacks_left--;
      }
      break;
    case Stage::FairyForest:
      if (blocks_were_placed && blocks_left > 0) {
        blocks_left--;
      }
      break;
    case Stage::GlitteryGrotto:
      if (unicorn.score > SCORE_GOAL) {
        points_left = 0;
      } else {
        points_left = SCORE_GOAL - unicorn.score;
      }
    case Stage::MarshmallowMountain:
      // TODO: track Miroh Jr's defeat
      break;
    }
    if (current_stage == Stage::GlitteryGrotto) {
      u8_to_text(goal_counter_text, current_level + 1);
    } else {
      u8_to_text(goal_counter_text, (u8)goal_counter);
    }
    multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));

    if (!stuff_in_progress && goal_counter == 0) {
      multi_vram_buffer_horz(
          story_mode_victory_text_per_stage[(u8)current_stage],
          sizeof(story_mode_victory_text_per_stage[0]), PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(continue_text, sizeof(continue_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      gameplay_state = GameplayState::ConfirmContinue;
      banked_play_song(Song::Victory);
      break;
    }
    if (failed_to_place) {
      multi_vram_buffer_horz(story_mode_failure_text,
                             sizeof(story_mode_failure_text),
                             PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(retry_exit_confirmation_text,
                             sizeof(retry_exit_confirmation_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      gameplay_state = GameplayState::RetryOrExit;
      yes_no_option = true;
      banked_play_song(Song::Failure);
    }
    break;
  case GameMode::Endless:
    u8_to_text(goal_counter_text, current_level + 1);
    multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));

    if (failed_to_place) {
      multi_vram_buffer_horz(non_story_mode_match_ending_text,
                             sizeof(non_story_mode_match_ending_text),
                             PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(continue_text, sizeof(continue_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      gameplay_state = GameplayState::ConfirmContinue;
    }
    break;
  case GameMode::TimeTrial:
    time_trial_frames++;
    if (time_trial_frames == 60) {
      time_trial_frames = 0;
      time_trial_seconds--;
      u8_to_text(goal_counter_text, time_trial_seconds);
      multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));
      if (time_trial_seconds == 10 || time_trial_seconds == 5 ||
          time_trial_seconds == 0) {
        banked_play_sfx(SFX::Timeralmostgone, GGSound::SFXPriority::One);
      }
      if (time_trial_seconds == 0) {
        end_game();
        break;
      }
    }
    if (failed_to_place) {
      end_game();
    }
    break;
  }
}

void Gameplay::end_game() {
  multi_vram_buffer_horz(non_story_mode_match_ending_text,
                         sizeof(non_story_mode_match_ending_text),
                         PAUSE_MENU_POSITION);
  multi_vram_buffer_horz(continue_text, sizeof(continue_text),
                         PAUSE_MENU_OPTIONS_POSITION);
  gameplay_state = GameplayState::ConfirmContinue;
}

void Gameplay::pause_game() {
  gameplay_state = GameplayState::Paused;
  GGSound::pause();
  multi_vram_buffer_horz(pause_menu_text, sizeof(pause_menu_text),
                         PAUSE_MENU_POSITION);
  multi_vram_buffer_horz(pause_menu_options_text,
                         sizeof(pause_menu_options_text),
                         PAUSE_MENU_OPTIONS_POSITION);
}

void Gameplay::swap_inputs() {
  if (input_mode == InputMode::Unicorn) {
    input_mode = InputMode::Polyomino;
  } else {
    input_mode = InputMode::Unicorn;
  }

  gameplay_state = GameplayState::Swapping;

  swap_frame_counter = 0;
  swap_index = 0;
}

void Gameplay::loop() {
  static bool no_lag_frame = true;
  extern volatile char FRAME_CNT1;

  while (current_game_state == GameState::Gameplay) {
    ppu_wait_nmi();

    START_MESEN_WATCH(1);

    pad_poll(0);
    pad_poll(1);
    if (input_mode == InputMode::Unicorn) {
      unicorn_pressed = get_pad_new(0);
      unicorn_held = pad_state(0);
      polyomino_pressed = get_pad_new(1);
      polyomino_held = pad_state(1);
    } else {
      unicorn_pressed = get_pad_new(1);
      unicorn_held = pad_state(1);
      polyomino_pressed = get_pad_new(0);
      polyomino_held = pad_state(0);
    }
    any_pressed = unicorn_pressed | polyomino_pressed;
    any_held = unicorn_held | polyomino_held;

    u8 frame = FRAME_CNT1;

    Attributes::enable_vram_buffer();

    pad_poll(0);
    pad_poll(1);

    switch (gameplay_state) {
    case GameplayState::Playing:
      Gameplay::gameplay_handler();
      break;
    case GameplayState::Paused:
      Gameplay::pause_handler();
      break;
    case GameplayState::ConfirmExit:
      Gameplay::confirm_exit_handler();
      break;
    case GameplayState::ConfirmRetry:
      Gameplay::confirm_retry_handler();
      break;
    case GameplayState::ConfirmContinue:
      Gameplay::confirm_continue_handler();
      break;
    case GameplayState::RetryOrExit:
      Gameplay::retry_exit_handler();
      break;
    case GameplayState::Retrying:
      // leaves the gameplay loop; since we're still on the gameplay game
      // state this is equivalent to retrying under the same conditions
      return;
    case GameplayState::Swapping:
      if (swap_frame_counter == 0) {
        if (swap_frames[swap_index].display_unicorn) {
          banked_play_sfx(SFX::Unicornon, GGSound::SFXPriority::One);
        } else {
          banked_play_sfx(SFX::Unicornoff, GGSound::SFXPriority::One);
        }
      }
      swap_frame_counter++;
      if (swap_frame_counter >= swap_frames[swap_index].duration) {
        swap_frame_counter = 0;
        swap_index++;
        if (swap_index >= sizeof(swap_frames)) {
          swap_index = 0;
          gameplay_state = GameplayState::Playing;
        }
      }
      break;
    }

    Attributes::flush_vram_update();

    if (VRAM_INDEX + 16 < 64) {
      unicorn.refresh_score_hud();
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
      banked_play_sfx(SFX::Levelup, GGSound::SFXPriority::Two);
    }
  }
}
