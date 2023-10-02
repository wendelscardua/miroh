#include "player.hpp"
#include "bank-helper.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
  : facing(Direction::Right),
    moving(Direction::Right),
    hunger(0),
    hunger_timer(0),
    score(0),
    state(State::Idle),
    board(board),
    x(starting_x),
    y(starting_y) {}

void Player::hunger_upkeep() {
  hunger_timer++;
  if (hunger_timer >= HUNGER_TICKS) {
    hunger_timer = 0;
    if (hunger == MAX_HUNGER) {
      u8 old_bank = get_prg_bank();
      set_prg_bank(GET_BANK(song_list));
      GGSound::play_song(Song::Rip_in_peace);
      set_prg_bank(old_bank);
      state = State::Dying;
      ghost_height = 0;
    } else {
      hunger++;
      refresh_hunger_hud();
    }
  }
}

void Player::update(InputMode input_mode, u8 pressed, u8 held) {
  switch (state) {
  case State::Idle: {
    hunger_upkeep();
    auto current_row = y.whole >> 4;
    auto current_column = x.whole >> 4;
    auto current_cell = board.cell[current_row][current_column];
    if (input_mode != InputMode::Player) break;
    if (held & PAD_UP) {
      moving = Direction::Up;
      if (!current_cell.up_wall &&
          current_row > 0 &&
          !board.occupied((s8)(current_row - 1), (s8)current_column)) {
        target_x = x;
        target_y = y - GRID_SIZE;
        state = State::Moving;
      }
    } else if (held & PAD_DOWN) {
      moving = Direction::Down;
      if (!current_cell.down_wall &&
          !board.occupied((s8)(current_row + 1), (s8)current_column)) {
        target_x = x;
        target_y = y + GRID_SIZE;
        state = State::Moving;
      }
    } else if (held & PAD_LEFT) {
      facing = Direction::Left;
      moving = Direction::Left;
      if (!current_cell.left_wall &&
          !board.occupied((s8)current_row, (s8)(current_column - 1))) {
        target_x = x - GRID_SIZE;
        target_y = y;
        state = State::Moving;
      }
    } else if (held & PAD_RIGHT) {
      facing = Direction::Right;
      moving = Direction::Right;
      if (!current_cell.right_wall &&
          !board.occupied((s8)current_row, (s8)(current_column + 1))) {
        target_x = x + GRID_SIZE;
        target_y = y;
        state = State::Moving;
      }
    } else {
      moving = Direction::None;
    }
  } break;
  case State::Moving:
    hunger_upkeep();
    switch (moving) {
    case Direction::Up:
      y -= move_speed;
      if (y < target_y) {
        y = target_y;
        state = State::Idle;
      }
      break;
    case Direction::Right:
      x += move_speed;
      if (x > target_x) {
        x = target_x;
        state = State::Idle;
      }
      break;
    case Direction::Down:
      y += move_speed;
      if (y > target_y) {
        y = target_y;
        state = State::Idle;
      }
      break;
    case Direction::Left:
      x -= move_speed;
      if (x < target_x) {
        x = target_x;
        state = State::Idle;
      }
      break;
    case Direction::None:
      break;
    }
    break;
  case State::Dying:
    {
      if (get_frame_count() & 0b100) {
        ghost_height++;
      }
      if (ghost_height > 0x40) {
        ghost_height = 0;
        state = State::Dead;
      }
    }
    break;
  case State::Dead:
    {
      if (ghost_height < 0x30 && (get_frame_count() & 0b100)) {
        ghost_height++;
        set_scroll_y(ghost_height);
      }
    }
    break;
  }
}

void Player::render() {
  const u8 *metasprite;
  switch (state) {
  case State::Idle:
    switch (facing) {
    case Direction::Up:
    case Direction::Right:
    case Direction::Down:
    case Direction::None:
      metasprite = metasprite_MinoRight1;
      break;
    case Direction::Left:
      metasprite = metasprite_MinoLeft1;
      break;
    }
    break;
  case State::Moving:
    {
      bool toggle = ((x.whole ^ y.whole) & 0b1000) != 0;
      switch (facing) {
      case Direction::Up:
      case Direction::Right:
      case Direction::Down:
      case Direction::None:
        if (toggle) {
          metasprite = metasprite_MinoRight1;
        } else {
          metasprite = metasprite_MinoRight2;
        }
        break;
      case Direction::Left:
        if (toggle) {
          metasprite = metasprite_MinoLeft1;
        } else {
          metasprite = metasprite_MinoLeft2;
        }
        break;
      }
    }
    break;
  case State::Dying:
    if (ghost_height > 4 && board.origin_y + (u8)y.whole > ghost_height) {
      oam_meta_spr(board.origin_x + (u8)x.whole, board.origin_y + (u8)y.whole - ghost_height,
                   metasprite_Ghost);
    }
    // break; <--- skipped on purpose
  case State::Dead:
    metasprite = metasprite_RIP;
    break;
  }
  oam_meta_spr(board.origin_x + (u8)x.round(), board.origin_y + (u8)y.round(),
               metasprite);
}

void Player::feed(u8 nutrition) {
  hunger_timer = 0;
  if (hunger > nutrition) {
    hunger -= nutrition;
  } else {
    hunger = 0;
  }

  score += 1;

  refresh_hunger_hud();
  refresh_score_hud();
}

void Player::refresh_hunger_hud() {
  // refresh hunger hud
  u8 hunger_bar[4];
  u8 temp = hunger;
  for(auto &hunger_cell : hunger_bar) {
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


void Player::refresh_score_hud() {
  // refresh hunger hud
  u8 score_text[4];
  u16 temp = score;

  score_text[0] = 0x10;
  if (temp >= 8000) {
    score_text[0] += 8;
    temp -= 8000;
  }
  if (temp >= 4000) {
    score_text[0] += 4;
    temp -= 4000;
  }
  if (temp >= 2000) {
    score_text[0] += 2;
    temp -= 2000;
  }
  if (temp >= 1000) {
    score_text[0] += 1;
    temp -= 1000;
  }

  score_text[1] = 0x10;
  if (temp >= 800) {
    score_text[1] += 8;
    temp -= 800;
  }
  if (temp >= 400) {
    score_text[1] += 4;
    temp -= 400;
  }
  if (temp >= 200) {
    score_text[1] += 2;
    temp -= 200;
  }
  if (temp >= 100) {
    score_text[1] += 1;
    temp -= 100;
  }

  score_text[2] = 0x10;
  if (temp >= 80) {
    score_text[2] += 8;
    temp -= 80;
  }
  if (temp >= 40) {
    score_text[2] += 4;
    temp -= 40;
  }
  if (temp >= 20) {
    score_text[2] += 2;
    temp -= 20;
  }
  if (temp >= 10) {
    score_text[2] += 1;
    temp -= 10;
  }

  score_text[3] =  0x10 + (u8)temp;

  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 27));
}
