#include "player.hpp"
#include "banked-asset-helpers.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "gameplay.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "metasprites.hpp"
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right), hunger(0),
      hunger_timer(0), state(State::Idle), board(board), x(starting_x),
      y(starting_y), score(0) {}

const fixed_point &Player::move_speed() { return DEFAULT_MOVE_SPEED; }

void Player::hunger_upkeep(s16 delta) {
  hunger_timer += delta;
  while (hunger_timer >= HUNGER_TICKS) {
    hunger_timer -= HUNGER_TICKS;
    if (hunger == MAX_HUNGER) {
      banked_play_song(Song::Rip_in_peace);
      state = State::Dying;
      ghost_height = 0;
      break;
    } else {
      hunger++;
    }
  }
  refresh_hunger_hud();
}

__attribute__((noinline, section(PLAYER_TEXT_SECTION))) void
Player::update(InputMode input_mode, u8 pressed, u8 held) {
  if (state != State::Dying && state != State::Dead) {
    hunger_upkeep(1);
  }
  switch (state) {
  case State::Idle: {
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
      if (y < target_y) {
        y = target_y;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
      }
      break;
    case Direction::Right:
      x += move_speed();
      if (x > target_x) {
        x = target_x;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
      }
      break;
    case Direction::Down:
      y += move_speed();
      if (y > target_y) {
        y = target_y;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
      }
      break;
    case Direction::Left:
      x -= move_speed();
      if (x < target_x) {
        x = target_x;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
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
    if (ghost_height < 0x30 && (get_frame_count() & 0b100)) {
      ghost_height++;
      set_scroll_y(ghost_height);
    }
  } break;
  }
}

void Player::render() {
  u8 reference_y = board.origin_y - Gameplay::DEFAULT_SCROLL_Y;
  const u8 *metasprite;
  switch (state) {
  case State::Idle:
    switch (facing) {
    case Direction::Up:
    case Direction::Right:
    case Direction::Down:
    case Direction::None:
      metasprite = metasprite_UniRightIdle;
      break;
    case Direction::Left:
      metasprite = metasprite_UniLeftIdle;
      break;
    }
    break;
  case State::Moving: {
    bool toggle = ((x.whole ^ y.whole) & 0b1000) != 0;
    switch (facing) {
    case Direction::Up:
    case Direction::Right:
    case Direction::Down:
    case Direction::None:
      if (toggle) {
        metasprite = metasprite_UniRightWalk1;
      } else {
        metasprite = metasprite_UniRightWalk2;
      }
      break;
    case Direction::Left:
      if (toggle) {
        metasprite = metasprite_UniLeftWalk1;
      } else {
        metasprite = metasprite_UniLeftWalk2;
      }
      break;
    }
  } break;
  case State::Dying:
    metasprite = metasprite_UniRightIdle;
    break;
  case State::Dead:
    banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                        reference_y + (u8)y.round() - ghost_height,
                        metasprite_UniRightIdle);
    return;
  }
  banked_oam_meta_spr(board.origin_x + (u8)x.round(),
                      reference_y + (u8)y.round(), metasprite);
}

void Player::feed(u8 nutrition) {
  banked_play_sfx(SFX::Nom, GGSound::SFXPriority::One);

  hunger_timer = 0;
  if (hunger > nutrition) {
    hunger -= nutrition;
  } else {
    hunger = 0;
  }

  refresh_hunger_hud();
}

void Player::refresh_hunger_hud() {
  // refresh hunger hud
  u8 hunger_bar[4];
  u8 temp = hunger;
  for (auto &hunger_cell : hunger_bar) {
    if (temp >= 8) {
      hunger_cell = HUNGER_BAR_BASE_TILE + 8;
      temp -= 8;
    } else {
      hunger_cell = HUNGER_BAR_BASE_TILE + temp;
      temp = 0;
    }
  }

  multi_vram_buffer_horz(hunger_bar, 4, NTADR_A(12, 27));
}

__attribute__((section(".prg_rom_0.text"))) void int_to_text(u8 score_text[4],
                                                             u16 value) {
  score_text[0] = 0x10;
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

  score_text[1] = 0x10;
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

  score_text[2] = 0x10;
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

  score_text[3] = 0x10 + (u8)value;
}

extern u16 high_score[];

void Player::refresh_score_hud() {
  // refresh hunger hud
  u8 old_bank = get_prg_bank();
  set_prg_bank(0);
  u8 score_text[4];

  int_to_text(score_text, score);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 27));

  if (score > high_score[maze]) {
    high_score[maze] = score;
  }
  int_to_text(score_text, high_score[maze]);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 28));
  set_prg_bank(old_bank);
}
