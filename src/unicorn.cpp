#include "unicorn.hpp"
#include "animation.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "utils.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Unicorn::Unicorn(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right),
      energy(STARTING_ENERGY), energy_timer(0),
      original_energy(STARTING_ENERGY), state(State::Idle), board(board),
      x(starting_x), y(starting_y), score(0) {}

const fixed_point &Unicorn::move_speed() {
  if (energy > 0) {
    return DEFAULT_MOVE_SPEED;
  } else {
    return TIRED_MOVE_SPEED;
  }
}

void Unicorn::set_state(State new_state) {
  state = new_state;
  switch (state) {
  case State::Idle:
    idle_left_animation.reset();
    idle_right_animation.reset();
    tired_left_animation.reset();
    tired_right_animation.reset();
    break;
  case State::Moving:
    moving_left_animation.reset();
    moving_right_animation.reset();
    trudging_left_animation.reset();
    trudging_right_animation.reset();
    break;
  case State::Yawning:
    yawn_left_animation.reset();
    yawn_right_animation.reset();
    break;
  case State::Sleeping:
    sleep_left_animation.reset();
    sleep_right_animation.reset();
    break;
  case State::Trapped:
    trapped_animation.reset();
    break;
  }
}

void Unicorn::energy_upkeep() {
  energy_timer++;
  // XXX: this could be an if, but eats 40 bytes of rom for some reason?
  while (energy_timer >= ENERGY_TICKS) {
    energy_timer = 0;
    if (energy > 0) {
      energy--;
      if (energy == 0) {
        banked_play_sfx(SFX::Outofenergy, GGSound::SFXPriority::One);
      }
    }
  }
}

__attribute__((noinline, section(PLAYER_TEXT_SECTION))) void
Unicorn::update(u8 pressed, u8 held) {
  energy_upkeep();

  switch (state) {
  case State::Idle:
  case State::Yawning:
  case State::Sleeping: {
  check_idle:
    if (!pressed && buffered_input) {
      pressed = buffered_input;
      buffered_input = 0;
    }
    u8 current_row = y.whole >> 4;
    u8 current_column = x.whole >> 4;

    // check if unicorn is trapped
    if (board.occupied((s8)current_row, (s8)current_column)) {
      set_state(State::Trapped);
      break;
    }

    auto current_cell = board.cell_at(current_row, current_column);

#define PRESS_HELD(button)                                                     \
  ((pressed & (button)) ||                                                     \
   (!pressed && moving != Direction::None && (held & (button))))
    if (PRESS_HELD(PAD_UP)) {
      if (!current_cell.up_wall && current_row > 0 &&
          !board.occupied((s8)(current_row - 1), (s8)current_column)) {
        moving = Direction::Up;
        target_x = x;
        target_y = y - GRID_SIZE;
        set_state(State::Moving);
        break;
      }
    }
    if (PRESS_HELD(PAD_DOWN)) {
      if (!current_cell.down_wall &&
          !board.occupied((s8)(current_row + 1), (s8)current_column)) {
        moving = Direction::Down;
        target_x = x;
        target_y = y + GRID_SIZE;
        set_state(State::Moving);
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
        set_state(State::Moving);
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
        set_state(State::Moving);
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
        set_state(State::Idle);
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
        set_state(State::Idle);
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
        set_state(State::Idle);
        if (!(held & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT))) {
          moving = Direction::None;
        }
        goto check_idle;
      }
      break;
    case Direction::Left:
      if (x <= target_x + move_speed()) {
        x = target_x;
        set_state(State::Idle);
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
  case State::Trapped:
    break;
  }
}

void Unicorn::fix_uni_priority(bool left_wall, bool right_wall) {
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

void Unicorn::render(int y_scroll, bool left_wall, bool right_wall) {
  int reference_y = board.origin_y - y_scroll;

  switch (state) {
  case State::Idle: {
    Animation &animation =
        (facing == Direction::Right
             ? (energy > 0 ? idle_right_animation : tired_right_animation)
             : (energy > 0 ? idle_left_animation : tired_left_animation));
    animation.update(board.origin_x + x.whole, reference_y + y.whole);
    if (animation.finished) {
      set_state(State::Yawning);
    }
  } break;
  case State::Moving: {
    sprite_offset = SPRID;
    Animation &animation =
        (facing == Direction::Right
             ? (energy > 0 ? moving_right_animation : trudging_right_animation)
             : (energy > 0 ? moving_left_animation : trudging_left_animation));
    animation.update(board.origin_x + x.whole, reference_y + y.whole);
    fix_uni_priority(left_wall, right_wall);
  } break;
  case State::Yawning: {
    Animation &animation = (facing == Direction::Right ? yawn_right_animation
                                                       : yawn_left_animation);
    animation.update(board.origin_x + x.whole, reference_y + y.whole);
    if (animation.finished) {
      set_state(State::Sleeping);
    }
  } break;
  case State::Sleeping:
    (facing == Direction::Right ? sleep_right_animation : sleep_left_animation)
        .update(board.origin_x + x.whole, reference_y + y.whole);
    break;
  case State::Trapped:
    trapped_animation.update(board.origin_x + x.whole, reference_y + y.whole);
    break;
  }
}

void Unicorn::feed(u8 nutrition) {
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

void Unicorn::refresh_energy_hud(int y_scroll) {
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

void Unicorn::refresh_score_hud() {
  // refresh hunger hud
  u8 score_text[4];

  int_to_text(score_text, score);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(22, 27));

  if (score > high_score[maze]) {
    high_score[maze] = score;
  }
  int_to_text(score_text, high_score[maze]);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 4));
}
