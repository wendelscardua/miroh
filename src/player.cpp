#include "player.hpp"
#include "animation.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "input-mode.hpp"
#include "maze-defs.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right),
      energy(STARTING_ENERGY), energy_timer(0),
      original_energy(STARTING_ENERGY), state(State::Idle), board(board),
      x(starting_x), y(starting_y), score(0), lines(0) {}

const fixed_point &Player::move_speed() { return DEFAULT_MOVE_SPEED; }

void Player::energy_upkeep(s16 delta) {
  energy_timer += delta;
  while (energy_timer >= ENERGY_TICKS) {
    energy_timer -= ENERGY_TICKS;
    if (energy == 0) {
      banked_play_sfx(SFX::Outofenergy, GGSound::SFXPriority::One);
      break;
    } else {
      energy--;
    }
  }
}

__attribute__((noinline, section(PLAYER_TEXT_SECTION))) void
Player::update(InputMode input_mode) {
  energy_upkeep(1);

  u8 pressed, held;

  switch (current_controller_scheme) {
  case ControllerScheme::OnePlayer:
    if (input_mode == InputMode::Player) {
      pressed = get_pad_new(0);
      held = pad_state(0);
      // TODO: think about allowing a second player to enter during a 1p game
    } else {
      pressed = 0;
      held = 0;
    }
    break;
  case ControllerScheme::TwoPlayers:
    pressed = get_pad_new(1);
    held = pad_state(1);
    break;
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
    auto current_cell = board.cell_at((u8)current_row, (u8)current_column);

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
      if (y <= target_y + move_speed()) {
        y = target_y;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      } else {
        y -= move_speed();
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
      if (x <= target_x + move_speed()) {
        x = target_x;
        state = State::Idle;
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      } else {
        x -= move_speed();
      }
      break;
    case Direction::None:
      break;
    }
    break;
  }
}

extern "C" char OAM_BUF[256];
extern "C" char SPRID;

void Player::fix_uni_priority(bool left_wall, bool right_wall) {
  if (state != State::Moving) {
    return;
  }
  u8 tile_y = y.whole & 0x0f;
  if (tile_y < 0x07 || tile_y > 0x0c) {
    return;
  }

  // XXX: assume player is first sprite, so we know the right indices to mess
  // with here
  if (!left_wall) {
    OAM_BUF[sprite_offset + 2] ^= OAM_BEHIND;
  }
  if (!right_wall) {
    OAM_BUF[sprite_offset + 6] ^= OAM_BEHIND;
  }
}

void Player::render(int y_scroll, bool left_wall, bool right_wall) {
  int reference_y = board.origin_y - y_scroll;

  switch (state) {
  case State::Idle:
    if (facing == Direction::Right) {
      idleRightAnimation.update(board.origin_x + x.whole,
                                reference_y + y.whole);
    } else {
      idleLeftAnimation.update(board.origin_x + x.whole, reference_y + y.whole);
    }
    break;
  case State::Moving:
    sprite_offset = SPRID;
    if (facing == Direction::Right) {
      movingRightAnimation.update(board.origin_x + x.whole,
                                  reference_y + y.whole);
    } else {
      movingLeftAnimation.update(board.origin_x + x.whole,
                                 reference_y + y.whole);
    }
    fix_uni_priority(left_wall, right_wall);
    break;
  }
}

void Player::feed(u8 nutrition) {
  banked_play_sfx(SFX::Eat, GGSound::SFXPriority::One);

  energy_timer = 0;
  if (energy < MAX_ENERGY - nutrition) {
    energy += nutrition;
  } else {
    energy = MAX_ENERGY;
  }
}

void render_energy_hud(int y_scroll, u8 value) {
  static constexpr u8 ENERGY_HUD_X = 0x30;
  static constexpr u8 ENERGY_HUD_Y = 0xd7;

  if ((u16)(ENERGY_HUD_Y - y_scroll) >> 8 != 0) {
    return;
  }

  if (value == 0) {
  } else if (value == 1) {
    oam_spr(ENERGY_HUD_X, (u8)(ENERGY_HUD_Y - y_scroll), 0x31, 0);
  } else if (value == 2) {
    oam_spr(ENERGY_HUD_X, (u8)(ENERGY_HUD_Y - y_scroll), 0x32, 0);
  } else {
    oam_spr(ENERGY_HUD_X, (u8)(ENERGY_HUD_Y - y_scroll), 0x33, 0);

    if (value == 3) {
    } else if (value == 4) {
      oam_spr(ENERGY_HUD_X + 8, (u8)(ENERGY_HUD_Y - y_scroll), 0x31, 0);
    } else if (value == 5) {
      oam_spr(ENERGY_HUD_X + 8, (u8)(ENERGY_HUD_Y - y_scroll), 0x32, 0);
    } else {
      oam_spr(ENERGY_HUD_X + 8, (u8)(ENERGY_HUD_Y - y_scroll), 0x33, 0);

      if (value == 6) {
      } else if (value == 7) {
        oam_spr(ENERGY_HUD_X + 16, (u8)(ENERGY_HUD_Y - y_scroll), 0x31, 0);
      } else if (value == 8) {
        oam_spr(ENERGY_HUD_X + 16, (u8)(ENERGY_HUD_Y - y_scroll), 0x32, 0);
      } else {
        oam_spr(ENERGY_HUD_X + 16, (u8)(ENERGY_HUD_Y - y_scroll), 0x33, 0);

        if (value == 9) {
        } else if (value == 10) {
          oam_spr(ENERGY_HUD_X + 24, (u8)(ENERGY_HUD_Y - y_scroll), 0x31, 0);
        } else if (value == 11) {
          oam_spr(ENERGY_HUD_X + 24, (u8)(ENERGY_HUD_Y - y_scroll), 0x32, 0);
        } else {
          oam_spr(ENERGY_HUD_X + 24, (u8)(ENERGY_HUD_Y - y_scroll), 0x33, 0);
        }
      }
    }
  }
}

void Player::refresh_energy_hud(int y_scroll) {
  static u8 animation_frames;

  CORO_INIT;

  if (original_energy == energy) {
    render_energy_hud(y_scroll, energy);
    CORO_FINISH();
    return;
  }

  for (animation_frames = 0; animation_frames < 4; animation_frames++) {
    render_energy_hud(y_scroll, energy);
    CORO_YIELD();
  }

  for (animation_frames = 0; animation_frames < 4; animation_frames++) {
    render_energy_hud(y_scroll, original_energy);
    CORO_YIELD();
  }

  for (animation_frames = 0; animation_frames < 4; animation_frames++) {
    render_energy_hud(y_scroll, energy);
    CORO_YIELD();
  }

  for (animation_frames = 0; animation_frames < 4; animation_frames++) {
    render_energy_hud(y_scroll, original_energy);
    CORO_YIELD();
  }

  original_energy = energy;
  render_energy_hud(y_scroll, energy);

  CORO_FINISH();
}

__attribute__((section(".prg_rom_0.text"))) void int_to_text(u8 score_text[4],
                                                             u16 value) {
  score_text[0] = DIGITS_BASE_TILE;
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

  score_text[1] = DIGITS_BASE_TILE;
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

  score_text[2] = DIGITS_BASE_TILE;
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

  score_text[3] = DIGITS_BASE_TILE + (u8)value;

  // leading zeroes are darker
  for (u8 i = 0; i < 3; i++) {
    if (score_text[i] > DIGITS_BASE_TILE) {
      break;
    }
    score_text[i] = DARK_ZERO_TILE;
  }
}

__attribute__((section(".prg_rom_0.text"))) void u8_to_text(u8 score_text[4],
                                                            u8 value) {
  score_text[0] = DIGITS_BASE_TILE;
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

  score_text[1] = DIGITS_BASE_TILE + (u8)value;

  if (score_text[0] == DIGITS_BASE_TILE) {
    score_text[0] = DARK_ZERO_TILE;
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
