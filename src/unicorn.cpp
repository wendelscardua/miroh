#include "unicorn.hpp"
#include "animation.hpp"
#include "banked-asset-helpers.hpp"
#include "board.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "direction.hpp"
#include "fixed-point.hpp"
#include "ggsound.hpp"
#include "metasprites.hpp"
#include "utils.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

Unicorn::Unicorn(Board &board, fixed_point starting_x, fixed_point starting_y)
    : facing(Direction::Right), moving(Direction::Right),
      energy(STARTING_ENERGY), energy_timer(0),
      original_energy(STARTING_ENERGY), state(State::Idle), board(board),
      x(starting_x), y(starting_y), row(starting_y.whole >> 4),
      column(starting_x.whole >> 4), score(0), statue(false) {
  set_state(State::Idle);
}

const fixed_point &Unicorn::move_speed() {
  if (energy > 0) {
    return DEFAULT_MOVE_SPEED;
  } else {
    return TIRED_MOVE_SPEED;
  }
}

__attribute__((noinline, section(PLAYER_TEXT_SECTION))) void
Unicorn::set_state(State new_state) {
  if (state == new_state) {
    // avoid resetting animation when reinforcing a current state
    return;
  }
  state = new_state;
  switch (state) {
  case State::Idle:
    left_animation =
        Animation{&idle_left_cells, sizeof(idle_left_cells) / sizeof(AnimCell)};
    right_animation = Animation{&idle_right_cells,
                                sizeof(idle_right_cells) / sizeof(AnimCell)};
    left_tired_animation = Animation{
        &tired_left_cells, sizeof(tired_left_cells) / sizeof(AnimCell)};
    right_tired_animation = Animation{
        &tired_right_cells, sizeof(tired_right_cells) / sizeof(AnimCell)};
    break;
  case State::Moving:
    left_animation = Animation{&moving_left_cells,
                               sizeof(moving_left_cells) / sizeof(AnimCell)};
    right_animation = Animation{&moving_right_cells,
                                sizeof(moving_right_cells) / sizeof(AnimCell)};
    left_tired_animation = Animation{
        &trudging_left_cells, sizeof(trudging_left_cells) / sizeof(AnimCell)};
    right_tired_animation = Animation{
        &trudging_right_cells, sizeof(trudging_right_cells) / sizeof(AnimCell)};
    break;
  case State::Yawning:
    left_animation = left_tired_animation =
        Animation{&yawn_left_cells, sizeof(yawn_left_cells) / sizeof(AnimCell)};
    right_animation = right_tired_animation = Animation{
        &yawn_right_cells, sizeof(yawn_right_cells) / sizeof(AnimCell)};
    break;
  case State::Sleeping:
    left_animation = left_tired_animation = Animation{
        &sleep_left_cells, sizeof(sleep_left_cells) / sizeof(AnimCell)};
    right_animation = right_tired_animation = Animation{
        &sleep_right_cells, sizeof(sleep_right_cells) / sizeof(AnimCell)};
    break;
  case State::Trapped:
    generic_animation =
        Animation{&trapped_cells, sizeof(trapped_cells) / sizeof(AnimCell)};
    break;
  case State::Roll:
    generic_animation =
        (facing == Direction::Right)
            ? Animation{&roll_right_cells,
                        sizeof(roll_right_cells) / sizeof(AnimCell)}
            : Animation{&roll_left_cells,
                        sizeof(roll_left_cells) / sizeof(AnimCell)};

    break;
  case State::Impact:
    generic_animation =
        (facing == Direction::Right)
            ? Animation{&impact_right_cells,
                        sizeof(impact_right_cells) / sizeof(AnimCell)}
            : Animation{&impact_left_cells,
                        sizeof(impact_left_cells) / sizeof(AnimCell)};

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
Unicorn::update(u8 pressed, u8 held, bool roll_disabled) {
  energy_upkeep();

  switch (state) {
  case State::Idle:
  case State::Yawning:
  case State::Sleeping: {
  check_idle:
    if ((pressed & (PAD_UP | PAD_DOWN | PAD_LEFT | PAD_RIGHT)) &&
        state != State::Idle) {
      set_state(State::Idle);
    }
    if (!pressed && buffered_input) {
      pressed = buffered_input;
      buffered_input = 0;
    }
    // check if unicorn is trapped
    if (board.occupied((s8)row, (s8)column)) {
      set_state(State::Trapped);
      break;
    }

    // begins roll action
    if (pressed & (PAD_A | PAD_B)) {
      if (energy < CHARGE_COST && !roll_disabled) {
        banked_play_sfx(SFX::Uiabort, GGSound::SFXPriority::Two);
        break;
      }
      banked_play_sfx(SFX::Headbutt, GGSound::SFXPriority::Two);
      energy -= CHARGE_COST;
      set_state(State::Roll);
      roll_distance = 0;
      bool occ, wall;
      if (facing == Direction::Right) {
        while (
            roll_distance < 3 &&
            !(wall = board.cell_at(row, column + roll_distance).right_wall) &&
            !(occ = board.occupied((s8)row, column + roll_distance + 1))) {
          roll_distance++;
        }
      } else {
        while (roll_distance < 3 &&
               !(wall = board.cell_at(row, column - roll_distance).left_wall) &&
               !(occ = board.occupied((s8)row, column - roll_distance - 1))) {
          roll_distance++;
        }
      }
      roll_into_block = (roll_distance < 3 && occ);
      break;
    }

    auto current_cell = board.cell_at(row, column);

#define PRESS_HELD(button)                                                     \
  ((pressed & (button)) ||                                                     \
   (!pressed && moving != Direction::None && (held & (button))))
    if (PRESS_HELD(PAD_UP)) {
      if (!current_cell.up_wall && row > 0 &&
          !board.occupied((s8)(row - 1), (s8)column)) {
        moving = Direction::Up;
        target_x = x;
        target_y = y - GRID_SIZE;
        set_state(State::Moving);
        break;
      }
    }
    if (PRESS_HELD(PAD_DOWN)) {
      if (!current_cell.down_wall &&
          !board.occupied((s8)(row + 1), (s8)column)) {
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
          !board.occupied((s8)row, (s8)(column - 1))) {
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
          !board.occupied((s8)row, (s8)(column + 1))) {
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
        row--;
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
        column++;
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
        row++;
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
        column--;
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
    if (generic_animation.current_cell_index == 1 &&
        generic_animation.current_frame == 0) {
      banked_play_sfx(SFX::Marshmallow, GGSound::SFXPriority::Two);
    }
    break;
  case State::Roll:
    if (generic_animation.finished) {
      set_state(State::Idle);
      break;
    }
    if (generic_animation.current_frame == 0 &&
        (generic_animation.current_cell_index == 3 ||
         generic_animation.current_cell_index == 4 ||
         generic_animation.current_cell_index == 5)) {
      // would have started Roll B, C or D
      if (roll_distance == 0) {
        set_state(State::Impact);
        break;
      }
      roll_distance--;
      if (facing == Direction::Right) {
        column++;
        x += GRID_SIZE;
      } else {
        column--;
        x -= GRID_SIZE;
      }
    }
    break;
  case State::Impact:
    if (generic_animation.finished) {
      set_state(State::Idle);
    }
    if (roll_into_block && generic_animation.current_cell_index == 0 &&
        generic_animation.current_frame == 3) {
      banked_play_sfx(SFX::Blockhit, GGSound::SFXPriority::Two);
      if (facing == Direction::Right) {
        if (board.occupied((s8)row, column + 2)) {
          board.add_animation(
              BoardAnimation(&Board::block_break_right,
                             sizeof(Board::block_break_right) /
                                 sizeof(Board::block_break_right[0]),
                             row, column + 1));
        } else {
          board.add_animation(
              BoardAnimation(&Board::block_move_right,
                             sizeof(Board::block_move_right) /
                                 sizeof(Board::block_move_right[0]),
                             row, column + 1));
          board.add_animation(
              BoardAnimation(&Board::block_arrive_right,
                             sizeof(Board::block_arrive_right) /
                                 sizeof(Board::block_arrive_right[0]),
                             row, column + 2));
        }
      } else {
        if (board.occupied((s8)row, column - 2)) {
          board.add_animation(
              BoardAnimation(&Board::block_break_left,
                             sizeof(Board::block_break_left) /
                                 sizeof(Board::block_break_left[0]),
                             row, column - 1));
        } else {
          board.add_animation(
              BoardAnimation(&Board::block_move_left,
                             sizeof(Board::block_move_left) /
                                 sizeof(Board::block_move_left[0]),
                             row, column - 1));
          board.add_animation(
              BoardAnimation(&Board::block_arrive_left,
                             sizeof(Board::block_arrive_left) /
                                 sizeof(Board::block_arrive_left[0]),
                             row, column - 2));
        }
      }
    } else if (generic_animation.current_cell_index == 5 &&
               generic_animation.current_frame == 0) {
      banked_play_sfx(SFX::Butt, GGSound::SFXPriority::One);
    }
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

  if (statue) {
    banked_oam_meta_spr(board.origin_x + x.whole, reference_y + y.whole,
                        facing == Direction::Right ? metasprite_UniRightStatue
                                                   : metasprite_UniLeftStatue);
    return;
  }

  if (state == State::Trapped || state == State::Roll ||
      state == State::Impact) {
    generic_animation.update(board.origin_x + x.whole, reference_y + y.whole);
    return;
  }

  sprite_offset = SPRID;

  Animation &animation =
      (facing == Direction::Right
           ? (energy > 0 ? right_animation : right_tired_animation)
           : (energy > 0 ? left_animation : left_tired_animation));
  animation.update(board.origin_x + x.whole, reference_y + y.whole);

  switch (state) {
  case State::Idle:
    if (animation.finished) {
      set_state(State::Yawning);
    }
    break;
  case State::Moving:
    fix_uni_priority(left_wall, right_wall);
    break;
  case State::Yawning:
    if (animation.finished) {
      set_state(State::Sleeping);
    }
    break;
  default:
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

  if (score > high_score[(u8)current_stage]) {
    high_score[(u8)current_stage] = score;
  }
  int_to_text(score_text, high_score[(u8)current_stage]);
  multi_vram_buffer_horz(score_text, 4, NTADR_A(23, 4));
}
