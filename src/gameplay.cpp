#include "animation.hpp"
#include "assets.hpp"
#include "board.hpp"
#include "log.hpp"
#include "metasprites.hpp"
#include "polyomino.hpp"
#include "soundtrack.hpp"
#ifndef NDEBUG
#include <cstdio>
#endif
#include <nesdoug.h>
#include <neslib.h>

#include "bank-helper.hpp"

#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "fixed-point.hpp"
#include "fruits.hpp"
#include "gameplay.hpp"
#include "ggsound.hpp"
#include "unicorn.hpp"

#pragma clang section text = ".prg_rom_0.text.gameplay"
#pragma clang section rodata = ".prg_rom_0.rodata.gameplay"

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

const u8 CLOSED_MOUTH[] = {MOUNTAIN_MOUTH_BASE_TILE + 0,
                           MOUNTAIN_MOUTH_BASE_TILE + 1};
const u8 OPEN_MOUTH[] = {MOUNTAIN_MOUTH_BASE_TILE + 2,
                         MOUNTAIN_MOUTH_BASE_TILE + 3};
const u8 EMPTY_PREVIEW[] = {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE};

const u8 STREAM[][4] = {{PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                        {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                        {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                         PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2,
                         PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                        {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8,
                         PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8},
                        {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                         PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                        {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8,
                         PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                         PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8,
                         PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                         PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 10,
                         PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 10,
                         PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                        {PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10,
                         PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10}};

Drops::Drops() {
  for (auto drop : drops) {
    drop.row = 0xff;
  }
  active_drops = 0;
}

u8 Drops::active_drops(0);

void Drops::add_random_drop() {
  u8 index;
  for (index = 0; index < drops.size(); index++) {
    if (drops[index].row > HEIGHT) {
      break;
    }
  }
  if (index == drops.size()) {
    return;
  }
  drops[index].row =
      banked_lambda(Board::BANK, []() { return board.random_free_row(); });
  if (drops[index].row > HEIGHT) {
    return;
  }
  drops[index].column = banked_lambda(Board::BANK, [this, &index]() {
    return board.random_free_column(drops[index].row);
  });
  drops[index].x = (u8)(drops[index].column << 4) + board.origin_x;
  drops[index].target_y = (u8)(drops[index].row << 4) + board.origin_y;
  drops[index].current_y = 0;
  drops[index].shadow = drops[index].target_y >> 4;
  active_drops++;
}

void Drops::update() {
  for (auto drop : drops) {
    if (drop.row > HEIGHT) {
      continue;
    }
    if (drop.current_y == drop.target_y) {
      banked_play_sfx(SFX::Blockplacement, GGSound::SFXPriority::One);
      banked_lambda(Board::BANK, [&drop]() {
        board.set_maze_cell((s8)drop.row, drop.column, CellType::Marshmallow);
      });
      drop.row = 0xff;
      active_drops--;
    } else {
      drop.shadow--;
      drop.current_y += 0x10;
    }
  }
}

const Sprite *const shadows[] = {
    Metasprites::BlockShadow1, Metasprites::BlockShadow2,
    Metasprites::BlockShadow3, Metasprites::BlockShadow4,
    Metasprites::BlockShadow5};

void Drops::render(int y_scroll) {
  for (auto drop : drops) {
    if (drop.row > HEIGHT) {
      continue;
    }
    banked_oam_meta_spr(METASPRITES_BANK, drop.x, drop.current_y - y_scroll,
                        current_stage == Stage::StarlitStables
                            ? Metasprites::block
                            : Metasprites::BlockB);
    if (drop.shadow > 5) {
      banked_oam_meta_spr(METASPRITES_BANK, drop.x, drop.target_y - y_scroll,
                          Metasprites::BlockShadow5);
    } else if (drop.shadow > 0) {
      banked_oam_meta_spr(METASPRITES_BANK, drop.x, drop.target_y - y_scroll,
                          shadows[drop.shadow - 1]);
    }
  }
}

bool Drops::random_hard_drop() {
  return banked_lambda(Board::BANK, []() {
    u8 row = board.random_free_row();
    if (row > HEIGHT) {
      return false;
    }
    u8 column = board.random_free_column(row);
    board.set_maze_cell((s8)row, column, CellType::Marshmallow);
    if ((get_frame_count() & 0b1111) == 0) {
      banked_play_sfx(SFX::Blockplacement, GGSound::SFXPriority::One);
    }
    return true;
  });
}

Gameplay::Gameplay()
    : experience(0), current_level(0), spawn_timer(0),
      unicorn(banked_lambda(Unicorn::BANK,
                            []() {
                              return Unicorn(board, fixed_point(0x50, 0x00),
                                             fixed_point(0x50, 0x00));
                            })),
      polyomino(board), fruits(board), gameplay_state(GameplayState::Playing),
      input_mode(InputMode::Polyomino), yes_no_option(false),
      pause_option(PauseOption::Resume), drops(), y_scroll(INTRO_SCROLL_Y),
      goal_counter(0) {
  load_gameplay_assets();

  vram_adr(NAMETABLE_A);

  banked_lambda(Board::BANK, []() {
    board.reset();
    board.render();
  });

  pal_bright(0);

  oam_clear();

  scroll(0, (unsigned int)y_scroll);

  banked_lambda(Unicorn::BANK, [this]() { unicorn.refresh_score_hud(); });

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

  while (y_scroll != Gameplay::DEFAULT_Y_SCROLL) {
    ppu_wait_nmi();
    ease_scroll(Gameplay::DEFAULT_Y_SCROLL);
    if (y_scroll >= -0x20 && y_scroll < 0) {
      y_scroll = 0;
    }
    scroll(0, (unsigned int)y_scroll);
  }
}

Gameplay::~Gameplay() {
  pal_fade_to(4, 0);
  color_emphasis(COL_EMP_NORMAL);
  ppu_off();
}

void Gameplay::render() {
  Animation::paused = (gameplay_state != GameplayState::Playing &&
                       gameplay_state != GameplayState::Swapping &&
                       gameplay_state != GameplayState::MarshmallowOverflow);
  BoardAnimation::paused = Animation::paused;
  scroll(0, (unsigned int)y_scroll);
  bool left_wall = false, right_wall = false;
  START_MESEN_WATCH(10);
  if (unicorn.state == Unicorn::State::Moving) {
    u8 row = unicorn.row + 1;
    u8 col = unicorn.column;
    if (row < HEIGHT) {
      auto cell = board.cell_at(row, col);
      left_wall = cell.left_wall;
      right_wall = cell.right_wall;
    }
  }
  STOP_MESEN_WATCH(10);
  START_MESEN_WATCH(11);
  fruits.render_below_player(y_scroll, unicorn.y.whole + board.origin_y);
  STOP_MESEN_WATCH(11);
  START_MESEN_WATCH(12);
  if (gameplay_state != GameplayState::Swapping ||
      swap_frames[swap_index].display_unicorn) {
    banked_lambda(Unicorn::BANK, [this, &left_wall, &right_wall]() {
      unicorn.render(y_scroll, left_wall, right_wall);
    });
  }
  STOP_MESEN_WATCH(12);
  START_MESEN_WATCH(13);
  fruits.render_above_player(y_scroll, unicorn.y.whole + board.origin_y);
  STOP_MESEN_WATCH(13);

  if (gameplay_state == GameplayState::MarshmallowOverflow &&
      overflow_state == OverflowState::FlashOutsideBlocks &&
      (marshmallow_overflow_counter & 0b1000)) {
    START_MESEN_WATCH(14);
    polyomino.outside_render(y_scroll);
    STOP_MESEN_WATCH(14);
  } else if ((gameplay_state == GameplayState::Swapping &&
              swap_frames[swap_index].display_polyomino) ||
             (gameplay_state != GameplayState::Swapping &&
              gameplay_state != GameplayState::MarshmallowOverflow)) {
    START_MESEN_WATCH(15);
    polyomino.render(y_scroll);
    STOP_MESEN_WATCH(15);
  }
  START_MESEN_WATCH(16);
  if (Drops::active_drops) {
    drops.render(y_scroll);
  }
  STOP_MESEN_WATCH(16);

  START_MESEN_WATCH(17);
  banked_lambda(Unicorn::BANK,
                [this]() { unicorn.refresh_energy_hud(y_scroll); });
  STOP_MESEN_WATCH(17);

  if (SPRID) {
    // if we rendered 64 sprites already, SPRID will have wrapped around back to
    // zero. in that case oam_hide_rest() would've hidden everyone

    oam_hide_rest();
  }

  banked_lambda(Board::BANK, []() { board.animate(); });
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

const u8 ease_deltas[] = {1, 2, 3, 4, 255};
void Gameplay::ease_scroll(const int target) {
  static u8 ease_index = 0;
  if (target == y_scroll)
    return;
  if (target > y_scroll) {
    while (y_scroll + ease_deltas[ease_index] > target) {
      ease_index--;
    }
    y_scroll += ease_deltas[ease_index++];
  } else {
    while (y_scroll - ease_deltas[ease_index] < target) {
      ease_index--;
    }
    y_scroll -= ease_deltas[ease_index++];
  }
}

void Gameplay::pause_handler() {
  ease_scroll(PAUSE_SCROLL_Y);

  static constexpr PauseOption NEXT_OPTION[] = {
      Gameplay::PauseOption::Resume,
      Gameplay::PauseOption::Exit,
      Gameplay::PauseOption::Retry,
  };

  static constexpr PauseOption PREV_OPTION[] = {
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
      ppu_wait_nmi();
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
      ppu_wait_nmi();
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
      current_game_state = GameState::WorldMap;
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
    banked_play_sfx(SFX::Uiconfirm, GGSound::SFXPriority::One);
    current_game_state = GameState::WorldMap;
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
      current_game_state = GameState::WorldMap;
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

  if (gameplay_state == GameplayState::MarshmallowOverflow) {
    any_pressed &= ~(PAD_START | PAD_SELECT | PAD_A | PAD_B);
  }

  if (any_pressed & PAD_START) {
    pause_game();
    return;
  } else if (any_pressed & PAD_SELECT) {
    swap_inputs();
  }

  unicorn.statue = false;
  if (current_controller_scheme == ControllerScheme::OnePlayer &&
      input_mode == InputMode::Polyomino &&
      unicorn.state == Unicorn::State::Idle) {
    unicorn.statue = true;
  }

  // XXX: if we say line clearing is in progress during overflow, it will make
  // other stuff not happen (polyomino won't spawn, victory conditions won't
  // trigger, not even the line clearing itself will run)
  bool line_clearing_in_progress =
      gameplay_state == GameplayState::MarshmallowOverflow ||
      banked_lambda(Board::BANK, []() {
        return board.ongoing_line_clearing(board.active_animations);
      });

  // we only spawn when there's no line clearing going on
  if (polyomino.state == Polyomino::State::Inactive &&
      !line_clearing_in_progress && spawn_timer-- == 0) {
    polyomino.spawn();
    spawn_timer = SPAWN_DELAY_PER_LEVEL[current_level];
  }
  polyomino.handle_input(polyomino_pressed, polyomino_held);
  polyomino.update(DROP_FRAMES_PER_LEVEL[current_level], blocks_were_placed,
                   failed_to_place, lines_cleared);

  banked_lambda(Unicorn::BANK, [this, line_clearing_in_progress]() {
    unicorn.update(unicorn_pressed, unicorn_held, line_clearing_in_progress);
  });

  fruits.update(unicorn, snack_was_eaten,
                gameplay_state != GameplayState::MarshmallowOverflow);

  if (failed_to_place) {
    gameplay_state = GameplayState::MarshmallowOverflow;
    overflow_state = OverflowState::FlashOutsideBlocks;
    marshmallow_overflow_counter = 0xff;
    GGSound::stop();
    banked_play_sfx(SFX::Blockoverflow, GGSound::SFXPriority::Two);
  } else if (blocks_were_placed &&
             current_controller_scheme == ControllerScheme::TwoPlayers) {
    banked_play_sfx(SFX::Blockplacement, GGSound::SFXPriority::One);
  }

  if (current_controller_scheme == ControllerScheme::OnePlayer &&
      polyomino.state != Polyomino::State::Active &&
      input_mode == InputMode::Polyomino) {
    swap_inputs();
  }

  if (lines_cleared) {
    const u8 points_per_lines[] = {0, 10, 30, 50, 70};
    u8 points = points_per_lines[lines_cleared];
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
      gameplay_state == GameplayState::Swapping ||
      gameplay_state == GameplayState::MarshmallowOverflow) {
    game_mode_upkeep(line_clearing_in_progress || blocks_were_placed ||
                     board.active_animations);
  }
}

__attribute__((noinline)) void Gameplay::marshmallow_overflow_handler() {
  marshmallow_overflow_counter++;
  switch (overflow_state) {
  case OverflowState::FlashOutsideBlocks:
    if (marshmallow_overflow_counter >=
        39) { // enough for blocks to blink {off, on, off, on, off}
      overflow_state = OverflowState::SwallowNextPiece;
      marshmallow_overflow_counter = 0xff;
      multi_vram_buffer_horz(OPEN_MOUTH, 2, NTADR_A(5, 5));
      multi_vram_buffer_horz(EMPTY_PREVIEW, 2, NTADR_A(5, 3));
      multi_vram_buffer_horz(EMPTY_PREVIEW, 2, NTADR_A(5, 4));
    }
    break;
  case OverflowState::SwallowNextPiece:
    // wait without doing anything
    if (marshmallow_overflow_counter >= 20) {
      overflow_state = OverflowState::ShootBlockStream;
      marshmallow_overflow_counter = 0xff;
      multi_vram_buffer_horz(CLOSED_MOUTH, 2, NTADR_A(5, 5));
    }
    break;
  case OverflowState::ShootBlockStream:
    multi_vram_buffer_vert(STREAM[marshmallow_overflow_counter >> 2], 4,
                           NTADR_A(6, 1));
    if (marshmallow_overflow_counter >> 2 >= 20) {
      overflow_state = OverflowState::ShadowBeforeRaining;
      marshmallow_overflow_counter = 0xff;
      color_emphasis(COL_EMP_DARK);
    }
    break;
  case OverflowState::ShadowBeforeRaining:
    if (marshmallow_overflow_counter >= 180) {
      overflow_state = OverflowState::FewDrops;
      marshmallow_overflow_counter = 0xff;
    }
    break;
  case OverflowState::FewDrops:
    if (marshmallow_overflow_counter == 255) {
      overflow_state = OverflowState::FasterDrops;
      marshmallow_overflow_counter = 0xff;
    } else if ((marshmallow_overflow_counter & 0b111111) == 0) {
      drops.add_random_drop();
    }
    drops.update();
    break;
  case OverflowState::FasterDrops:
    if (marshmallow_overflow_counter == 255) {
      overflow_state = OverflowState::DropEverywhereElse;
      marshmallow_overflow_counter = 0xff;
    } else if ((marshmallow_overflow_counter & 0b11111) == 0) {
      drops.add_random_drop();
    }
    drops.update();
    break;
  case OverflowState::DropEverywhereElse:
    if (Drops::active_drops) {
      drops.update();
    } else if (!drops.random_hard_drop()) {
      overflow_state = OverflowState::GameOver;
    }
    break;
  case OverflowState::GameOver:
    break;
  }
}

bool Gameplay::game_is_over() {
  return unicorn.state == Unicorn::State::Trapped &&
         unicorn.generic_animation.finished &&
         (gameplay_state != GameplayState::MarshmallowOverflow ||
          (gameplay_state == GameplayState::MarshmallowOverflow &&
           overflow_state == OverflowState::GameOver));
}

__attribute__((noinline)) void
Gameplay::game_mode_upkeep(bool stuff_in_progress) {
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
      ppu_wait_nmi();
      multi_vram_buffer_horz(
          story_mode_victory_text_per_stage[(u8)current_stage],
          sizeof(story_mode_victory_text_per_stage[0]), PAUSE_MENU_POSITION);
      multi_vram_buffer_horz(continue_text, sizeof(continue_text),
                             PAUSE_MENU_OPTIONS_POSITION);
      gameplay_state = GameplayState::ConfirmContinue;
      story_completion[(u8)current_stage] = true;
      banked_play_song(Song::Victory);
      break;
    }
    break;
  case GameMode::Endless:
    u8_to_text(goal_counter_text, current_level + 1);
    multi_vram_buffer_horz(goal_counter_text, 2, NTADR_A(15, 27));
    break;
  case GameMode::TimeTrial:
    if (gameplay_state != GameplayState::MarshmallowOverflow) {
      time_trial_frames++;
      if (time_trial_frames == TIME_TRIAL_FPS) {
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
    }
    break;
  }
  if (game_is_over()) {
    ppu_wait_nmi();
    if (current_game_mode == GameMode::Story) {
      fail_game();
    } else {
      end_game();
    }
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

void Gameplay::fail_game() {
  multi_vram_buffer_horz(story_mode_failure_text,
                         sizeof(story_mode_failure_text), PAUSE_MENU_POSITION);
  multi_vram_buffer_horz(retry_exit_confirmation_text,
                         sizeof(retry_exit_confirmation_text),
                         PAUSE_MENU_OPTIONS_POSITION);
  gameplay_state = GameplayState::RetryOrExit;
  yes_no_option = true;
  banked_play_song(Song::Failure);
}

void Gameplay::pause_game() {
  ppu_wait_nmi();
  gameplay_state = GameplayState::Paused;
  GGSound::pause();
  multi_vram_buffer_horz(pause_menu_text, sizeof(pause_menu_text),
                         PAUSE_MENU_POSITION);
  multi_vram_buffer_horz(pause_menu_options_text,
                         sizeof(pause_menu_options_text),
                         PAUSE_MENU_OPTIONS_POSITION);
}

void Gameplay::swap_inputs() {
  if (gameplay_state != GameplayState::Playing) {
    return;
  }
  if (input_mode == InputMode::Unicorn) {
    if (polyomino.state != Polyomino::State::Active) {
      banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::Two);
      return;
    }
    input_mode = InputMode::Polyomino;
  } else {
    input_mode = InputMode::Unicorn;
  }

  if (current_controller_scheme == ControllerScheme::OnePlayer &&
      blocks_were_placed) {
    banked_play_sfx(SFX::Number1pblockdrop, GGSound::SFXPriority::Two);
  } else {
    banked_play_sfx(SFX::Unicornon, GGSound::SFXPriority::Two);
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
    START_MESEN_WATCH(20);
    pad_poll(0);
    pad_poll(1);
    if (input_mode == InputMode::Unicorn) {
      unicorn_pressed = get_pad_new(0);
      unicorn_held = pad_state(0);
      polyomino_pressed = get_pad_new(1);
      polyomino_held = pad_state(1);
      if (polyomino_pressed) {
        current_controller_scheme = ControllerScheme::TwoPlayers;
      }
    } else {
      unicorn_pressed = get_pad_new(1);
      unicorn_held = pad_state(1);
      polyomino_pressed = get_pad_new(0);
      polyomino_held = pad_state(0);
      if (unicorn_pressed) {
        current_controller_scheme = ControllerScheme::TwoPlayers;
      }
    }
    any_pressed = unicorn_pressed | polyomino_pressed;
    any_held = unicorn_held | polyomino_held;

    u8 frame = FRAME_CNT1;

    switch (gameplay_state) {
    case GameplayState::MarshmallowOverflow:
      Gameplay::marshmallow_overflow_handler();
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
    STOP_MESEN_WATCH(20);
    START_MESEN_WATCH(21);
    if (VRAM_INDEX + 16 < 64) {
      banked_lambda(Unicorn::BANK, [this]() { unicorn.refresh_score_hud(); });
    }
    STOP_MESEN_WATCH(21);

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

void Gameplay::add_experience(u8 exp) {
  if (current_level < MAX_LEVEL) {
    experience += exp;
    while (experience >= LEVEL_UP_POINTS && current_level < MAX_LEVEL) {
      experience -= LEVEL_UP_POINTS;
      current_level++;
      banked_play_sfx(SFX::Levelup, GGSound::SFXPriority::Two);
    }
  }
}
