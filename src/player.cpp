#include "player.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "metasprites.hpp"
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right), energy(MAX_ENERGY),
      energy_timer(0), state(State::Idle), board(board), x(starting_x),
      y(starting_y), score(0), lines(0) {}

const fixed_point &Player::move_speed() { return DEFAULT_MOVE_SPEED; }

void Player::energy_upkeep(s16 delta) {
  energy_timer += delta;
  while (energy_timer >= ENERGY_TICKS) {
    energy_timer -= ENERGY_TICKS;
    if (energy == 0) {
      // TODO: don't die during co-op mode
      banked_play_song(Song::Rip_in_peace);
      state = State::Dying;
      ghost_height = 0;
      break;
    } else {
      energy--;
    }
  }
  refresh_energy_hud();
}

__attribute__((noinline, section(PLAYER_TEXT_SECTION))) void
Player::update(InputMode input_mode, u8 pressed, u8 held) {
  if (state != State::Dying && state != State::Dead) {
    energy_upkeep(1);
  }
  switch (state) {
  case State::Idle: {
  check_idle:
    if (!pressed && buffered_input) {
      pressed = buffered_input;
      buffered_input = 0;
    }
    auto current_row = y.whole >> 4;
    auto current_column = x.whole >> 4;
    auto current_cell = board.cell[current_row][current_column];

    if (input_mode != InputMode::Player)
      break;
#define PRESS_HELD(button)                                                     \
  ((pressed & (button)) ||                                                     \
   (!pressed && moving != Direction::None && (held & (button))))
    if (PRESS_HELD(PAD_UP)) {
      if (!current_cell.up_wall && current_row > 0 &&
          !board.occupied((s8)(current_row - 1), (s8)current_column)) {
        moving = Direction::Up;
        target_x = x;
        target_y = y - GRID_SIZE;
        state = State::Moving;
        break;
      }
    }
    if (PRESS_HELD(PAD_DOWN)) {
      if (!current_cell.down_wall &&
          !board.occupied((s8)(current_row + 1), (s8)current_column)) {
        moving = Direction::Down;
        target_x = x;
        target_y = y + GRID_SIZE;
        state = State::Moving;
        break;
      }
    }
    if (PRESS_HELD(PAD_LEFT)) {
      facing = Direction::Left;
      if (!current_cell.left_wall &&
          !board.occupied((s8)current_row, (s8)(current_column - 1))) {
        moving = Direction::Left;
        target_x = x - GRID_SIZE;
        target_y = y;
        state = State::Moving;
        break;
      }
    }
    if (PRESS_HELD(PAD_RIGHT)) {
      facing = Direction::Right;
      if (!current_cell.right_wall &&
          !board.occupied((s8)current_row, (s8)(current_column + 1))) {
        moving = Direction::Right;
        target_x = x + GRID_SIZE;
        target_y = y;
        state = State::Moving;
        break;
      }
    }
    moving = Direction::None;
  } break;
  case State::Moving:
    if (pressed) {
      buffered_input = pressed;
    }
    if (!held && buffered_input) {
      held |= buffered_input;
    }
    switch (moving) {
    case Direction::Up:
      y -= move_speed();
      if (y <= target_y) {
        y = target_y;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      }
      break;
    case Direction::Right:
      x += move_speed();
      if (x >= target_x) {
        x = target_x;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      }
      break;
    case Direction::Down:
      y += move_speed();
      if (y >= target_y) {
        y = target_y;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      }
      break;
    case Direction::Left:
      x -= move_speed();
      if (x <= target_x) {
        x = target_x;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      }
      break;
    case Direction::None:
      break;
    }
    break;
  case State::Dying: {
    if (get_frame_count() & 0b100) {
      ghost_height++;
    }
    if (ghost_height > 0x40) {
      ghost_height = 0;
      state = State::Dead;
    }
  } break;
  case State::Dead: {
  } break;
  }
}

void Player::render(int y_scroll) {
  u8 reference_y = (u8)(board.origin_y - y_scroll);
  static u8 animation_frame;
  static State current_state = State::Dead;
  CORO_RESET_WHEN(current_state != state);

  current_state = state;

  switch (state) {
  case State::Idle:
    for (animation_frame = 0; animation_frame < 162; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightIdle
                                                     : metasprite_UniLeftIdle);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 12; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightBlink
                                                     : metasprite_UniLeftBlink);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 8; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightIdle
                                                     : metasprite_UniLeftIdle);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 12; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightBlink
                                                     : metasprite_UniLeftBlink);
      if (animation_frame != 11) {
        CORO_YIELD();
      }
    }
    break;
  case State::Moving:
    for (animation_frame = 0; animation_frame < 5; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightWalk1
                                                     : metasprite_UniLeftWalk1);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 9; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightWalk2
                                                     : metasprite_UniLeftWalk2);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 5; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightWalk3
                                                     : metasprite_UniLeftWalk3);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 9; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                          reference_y + (u8)y.round(),
                          facing == Direction::Right ? metasprite_UniRightWalk4
                                                     : metasprite_UniLeftWalk4);
      if (animation_frame != 8) {
        CORO_YIELD();
      }
    }
    break;
  case State::Dead:
  case State::Dying:
    // TODO
    break;
  }

  CORO_FINISH();
}

void Player::feed(u8 nutrition) {
  banked_play_sfx(SFX::Nom, GGSound::SFXPriority::One);

  energy_timer = 0;
  if (energy < MAX_ENERGY - nutrition) {
    energy += nutrition;
  } else {
    energy = MAX_ENERGY;
  }

  refresh_energy_hud();
}

void Player::refresh_energy_hud() {
  // refresh hunger hud
  u8 hunger_bar[4];
  u8 temp = energy;
  for (auto &hunger_cell : hunger_bar) {
    // TODO: use proper energy tiles
    if (temp >= 8) {
      hunger_cell = 0x03;
      temp -= 8;
    } else {
      hunger_cell = 0xb4;
      temp = 0;
    }
  }

  multi_vram_buffer_horz(hunger_bar, 4, NTADR_A(6, 27));
}

__attribute__((section(".prg_rom_0.text"))) void int_to_text(u8 score_text[4],
                                                             u16 value) {
  score_text[0] = 0x03;
  if (value >= 8000) {
    score_text[0] += 8;
    value -= 8000;
  }
  if (value >= 4000) {
    score_text[0] += 4;
    value -= 4000;
  }
  if (value >= 2000) {
    score_text[0] += 2;
    value -= 2000;
  }
  if (value >= 1000) {
    score_text[0] += 1;
    value -= 1000;
  }

  score_text[1] = 0x03;
  if (value >= 800) {
    score_text[1] += 8;
    value -= 800;
  }
  if (value >= 400) {
    score_text[1] += 4;
    value -= 400;
  }
  if (value >= 200) {
    score_text[1] += 2;
    value -= 200;
  }
  if (value >= 100) {
    score_text[1] += 1;
    value -= 100;
  }

  score_text[2] = 0x03;
  if (value >= 80) {
    score_text[2] += 8;
    value -= 80;
  }
  if (value >= 40) {
    score_text[2] += 4;
    value -= 40;
  }
  if (value >= 20) {
    score_text[2] += 2;
    value -= 20;
  }
  if (value >= 10) {
    score_text[2] += 1;
    value -= 10;
  }

  score_text[3] = 0x03 + (u8)value;

  // leading zeroes are darker
  for (u8 i = 0; i < 3; i++) {
    if (score_text[i] > 0x03) {
      break;
    }
    score_text[i] = 0x0d;
  }
}

__attribute__((section(".prg_rom_0.text"))) void u8_to_text(u8 score_text[4],
                                                            u8 value) {
  score_text[0] = 0x03;
  if (value >= 80) {
    score_text[0] += 8;
    value -= 80;
  }
  if (value >= 40) {
    score_text[0] += 4;
    value -= 40;
  }
  if (value >= 20) {
    score_text[0] += 2;
    value -= 20;
  }
  if (value >= 10) {
    score_text[0] += 1;
    value -= 10;
  }

  score_text[1] = 0x03 + (u8)value;

  if (score_text[0] == 0x03) {
    score_text[0] = 0x0d;
  }
}

extern u16 high_score[];

void Player::refresh_score_hud() {
  // refresh hunger hud
  ScopedBank scopedBank(0); // int_to_text's bank
  u8 score_text[4];

  int_to_text(score_text, score);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(22, 27));

  if (score > high_score[maze]) {
    high_score[maze] = score;
  }
  int_to_text(score_text, high_score[maze]);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 4));

  u8_to_text(score_text, lines);
  multi_vram_buffer_horz(score_text, 2, NTADR_A(15, 27));
}
