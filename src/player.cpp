#include "player.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "metasprites.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Player::Player(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right),
      energy(STARTING_ENERGY), energy_timer(0), state(State::Idle),
      board(board), x(starting_x), y(starting_y), score(0), lines(0) {}

const fixed_point &Player::move_speed() { return DEFAULT_MOVE_SPEED; }

void Player::energy_upkeep(s16 delta) {
  energy_timer += delta;
  while (energy_timer >= ENERGY_TICKS) {
    energy_timer -= ENERGY_TICKS;
    if (energy == 0) {
      // TODO: don't die during co-op mode
      banked_play_song(Song::Glitter_grotto);
      state = State::Dying;
      ghost_height = 0;
      break;
    } else {
      energy--;
    }
  }
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

extern "C" char OAM_BUF[256];

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
    OAM_BUF[2] ^= OAM_BEHIND;
  }
  if (!right_wall) {
    OAM_BUF[6] ^= OAM_BEHIND;
  }
}

void Player::render(int y_scroll, bool left_wall, bool right_wall) {
  int reference_y = board.origin_y - y_scroll;
  static u8 animation_frame;
  static State current_state = State::Dead;
  CORO_RESET_WHEN(current_state != state);

  current_state = state;

  switch (state) {
  case State::Idle:
    for (animation_frame = 0; animation_frame < 162; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightIdle
                                                     : metasprite_UniLeftIdle);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 12; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightBlink
                                                     : metasprite_UniLeftBlink);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 8; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightIdle
                                                     : metasprite_UniLeftIdle);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 12; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightBlink
                                                     : metasprite_UniLeftBlink);
      if (animation_frame != 11) {
        CORO_YIELD();
      }
    }
    break;
  case State::Moving:
    for (animation_frame = 0; animation_frame < 5; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightWalk1
                                                     : metasprite_UniLeftWalk1);
      fix_uni_priority(left_wall, right_wall);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 9; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightWalk2
                                                     : metasprite_UniLeftWalk2);
      fix_uni_priority(left_wall, right_wall);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 5; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightWalk3
                                                     : metasprite_UniLeftWalk3);
      fix_uni_priority(left_wall, right_wall);
      CORO_YIELD();
    }
    for (animation_frame = 0; animation_frame < 9; animation_frame++) {
      banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                          facing == Direction::Right ? metasprite_UniRightWalk4
                                                     : metasprite_UniLeftWalk4);
      fix_uni_priority(left_wall, right_wall);
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
  banked_play_sfx(SFX::Number1, GGSound::SFXPriority::One);

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
  static u8 original_energy = STARTING_ENERGY;
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

  CORO_FINISH();
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
