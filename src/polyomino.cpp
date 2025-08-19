#include "polyomino.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "board.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "log.hpp"
#include "mountain-tiles.hpp"
#include "polyomino-defs.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_14.text.polyominos"
#pragma clang section rodata = ".prg_rom_14.rodata.polyominos"

// NOTE: source file defines indices [0, 4) as littleminos
auto Polyomino::littleminos = Bag<u8, 4>();

// NOTE: source file defines indices [11, 28) as pentominos
auto Polyomino::pentominos = Bag<u8, 17>();

// NOTE: source file defines indices [4, 11) as tetrominos
auto Polyomino::pieces = Bag<u8, 10>();

Polyomino::Polyomino(Board &board)
    : state(State::Inactive), board(board), definition(NULL) {}

void Polyomino::init() {
  spawn_state = SpawnState::WaitToSpawn;
  spawn_state_timer = 0;
  spawn_speed_tier = 0;

  // initialize littleminos bag
  littleminos.reset();
  for (u8 i = 0; i < 4; i++) {
    littleminos.insert(i);
  }

  // initialize pentominos bag
  pentominos.reset();
  for (u8 i = 11; i < 28; i++) {
    pentominos.insert(i);
  }

  // add all tetrominos to the pieces bag
  pieces.reset();
  for (u8 i = 4; i < 11; i++) {
    pieces.insert(i);
  }

  // also add two random "littleminos" (1,2, or 3 blocks)
  auto littlemino = littleminos.take();
  pieces.insert(littlemino);
  littleminos.insert(littlemino);

  littlemino = littleminos.take();
  pieces.insert(littlemino);
  littleminos.insert(littlemino);

  // ... and a random pentomino
  auto pentomino = pentominos.take();
  pieces.insert(pentomino);
  pentominos.insert(pentomino);

  next = polyominos[take_piece()];
}

u8 Polyomino::take_piece() {
  auto piece = pieces.take();
  if (piece < 4) {
    auto new_piece = littleminos.take();
    littleminos.insert(new_piece);
    pieces.insert(new_piece);
  } else if (piece >= 11) {
    auto new_piece = pentominos.take();
    pentominos.insert(new_piece);
    pieces.insert(new_piece);
  } else {
    pieces.insert(piece);
  }
  return piece;
}

void Polyomino::spawn() {
  state = State::Active;
  lock_down_timer = 0;
  lock_down_moves = 0;
  move_timer = 0;
  rotate_timer = 0;
  action = Action::Idle;
  column = SPAWN_COLUMN;
  row = 0;

  definition = next;
  next = polyominos[take_piece()];

  update_bitmask();

  row -= (bottom_limit + 1);

  x = board.origin_x + (u8)(column << 4);
  y = board.origin_y + (u8)(row << 4);

  // XXX: the next update on this frame will fix the shadow
  shadow_row = HEIGHT;
}

void Polyomino::update_bitmask() {
  START_MESEN_WATCH("bitmask");

  // NOTE: column + 3 because we've precomputed all possible shifts from
  // columns -3 to 11
  auto ptr = definition->bitmasks + (column + 3);

  // TODO: constantize bitmasks bank number
  banked_lambda(13, [this, ptr]() {
#pragma clang loop unroll(full)
    for (u8 i = 0; i < 4; i++) {
      bitmask[i] = (*ptr)[i];
    }
  });

  left_limit = definition->left_limit;
  right_limit = definition->right_limit;
  top_limit = definition->top_limit;
  bottom_limit = definition->bottom_limit;
  STOP_MESEN_WATCH("bitmask");
}

void Polyomino::move_bitmask_left() {
#pragma clang loop unroll(full)
  for (u8 i = 0; i < 4; i++) {
    bitmask[i] >>= 1;
  }
}

void Polyomino::move_bitmask_right() {
#pragma clang loop unroll(full)
  for (u8 i = 0; i < 4; i++) {
    bitmask[i] <<= 1;
  }
}

void Polyomino::update_shadow() {
  START_MESEN_WATCH("shadow");
  shadow_row = row;

  while (!collide(shadow_row + 1, (s8)column)) {
    shadow_row++;
  }

  shadow_y = board.origin_y + (u8)(shadow_row << 4);

  STOP_MESEN_WATCH("shadow");
}

bool Polyomino::collide(s8 new_row, s8 new_column) {
  if ((s8)(left_limit + new_column) < 0 ||
      (s8)(right_limit + new_column) >= WIDTH ||
      (s8)(bottom_limit + new_row) >= HEIGHT) {
    return true;
  }
  s8 mod_row = new_row;

  if (new_column == column) {
#pragma clang loop unroll(full)
    for (u8 i = 0; i < 4; i++, mod_row++) {
      if (i >= top_limit && i <= bottom_limit && mod_row >= 0) {
        if (bitmask[i] & board.occupied_bitset[(u8)mod_row]) {
          return true;
        }
      }
    }
  } else if (new_column > column) {
#pragma clang loop unroll(full)
    for (u8 i = 0; i < 4; i++, mod_row++) {
      if (i >= top_limit && i <= bottom_limit && mod_row >= 0) {
        if ((bitmask[i] << 1) & board.occupied_bitset[(u8)mod_row]) {
          return true;
        }
      }
    }
  } else {
#pragma clang loop unroll(full)
    for (u8 i = 0; i < 4; i++, mod_row++) {
      if (i >= top_limit && i <= bottom_limit && mod_row >= 0) {
        if ((bitmask[i] >> 1) & board.occupied_bitset[(u8)mod_row]) {
          return true;
        }
      }
    }
  }

  return false;
}

bool Polyomino::able_to_kick(const auto &kick_deltas) {
  START_MESEN_WATCH("kicks");
  for (auto kick : kick_deltas) {
    START_MESEN_WATCH("kick");
    s8 new_row = row + kick.delta_row;
    s8 new_column = column + kick.delta_column;

    if (!definition->collide(board, new_row, new_column)) {
      row = new_row;
      column = new_column;
      x += kick.delta_x;
      y += kick.delta_y;
      STOP_MESEN_WATCH("kick");
      STOP_MESEN_WATCH("kicks");
      return true;
    }
    STOP_MESEN_WATCH("kick");
  }
  STOP_MESEN_WATCH("kicks");
  return false;
}

void Polyomino::handle_input(u8 pressed, u8 held) {
  if (state != State::Active) {
    return;
  }
  if (pressed & PAD_UP) {
    action = Action::HardDrop;
  } else if (pressed & PAD_DOWN) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    action = Action::SoftDrop;
  } else if (held & PAD_DOWN) {
    if (move_timer == 0) {
      move_timer = MOVEMENT_DELAY;
      action = Action::SoftDrop;
    } else {
      move_timer--;
    }
  } else if (pressed & PAD_LEFT) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    action = Action::MoveLeft;
  } else if (held & PAD_LEFT) {
    if (move_timer == 0) {
      move_timer = MOVEMENT_DELAY;
      action = Action::MoveLeft;
    } else {
      move_timer--;
    }
  } else if (pressed & PAD_RIGHT) {
    move_timer = MOVEMENT_INITIAL_DELAY;
    action = Action::MoveRight;
  } else if (held & PAD_RIGHT) {
    if (move_timer == 0) {
      move_timer = MOVEMENT_DELAY;
      action = Action::MoveRight;
    } else {
      move_timer--;
    }
  }
  if (pressed & PAD_A) {
    rotate_timer = ROTATION_INITIAL_DELAY;
    if (action == Action::Idle) {
      action = Action::RotateRight;
    } else if (action == Action::MoveLeft) {
      action = Action::MoveLeftAndRotateRight;
    } else if (action == Action::MoveRight) {
      action = Action::MoveRightAndRotateRight;
    }
  } else if (held & PAD_A) {
    if (rotate_timer == 0) {
      rotate_timer = ROTATION_DELAY;
      if (action == Action::Idle) {
        action = Action::RotateRight;
      } else if (action == Action::MoveLeft) {
        action = Action::MoveLeftAndRotateRight;
      } else if (action == Action::MoveRight) {
        action = Action::MoveRightAndRotateRight;
      }
    } else {
      rotate_timer--;
    }
  } else if (pressed & PAD_B) {
    rotate_timer = ROTATION_INITIAL_DELAY;
    if (action == Action::Idle) {
      action = Action::RotateLeft;
    } else if (action == Action::MoveLeft) {
      action = Action::MoveLeftAndRotateLeft;
    } else if (action == Action::MoveRight) {
      action = Action::MoveRightAndRotateLeft;
    }
  } else if (held & PAD_B) {
    if (rotate_timer == 0) {
      rotate_timer = ROTATION_DELAY;
      if (action == Action::Idle) {
        action = Action::RotateLeft;
      } else if (action == Action::MoveLeft) {
        action = Action::MoveLeftAndRotateLeft;
      } else if (action == Action::MoveRight) {
        action = Action::MoveRightAndRotateLeft;
      }
    } else {
      rotate_timer--;
    }
  }
}

void Polyomino::freezing_handler(bool &blocks_placed, bool &failed_to_place,
                                 u8 &lines_cleared) {
  s8 lines = freeze_blocks();
  if (lines >= 0) {
    lines_cleared = (u8)lines;
    blocks_placed = true;
  } else {
    failed_to_place = true;
  }
}

// For each of 4 speed tiers, tells how many frames each spawn state lasts;
// usually spawn states do their main "thing" on their first frame
static const soa::Array<Polyomino::SpawnStateTransition, 6>
    spawn_state_frames[4] = {
        {{Polyomino::SpawnState::OpenToPushPreview, 12}, // WaitToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 20},    // OpenToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 4},     // PreviewFliesUp
         {Polyomino::SpawnState::SpawnAndPrepareToSpit, 32}, // WaitToSpawn
         {Polyomino::SpawnState::SpitNewPreview, 20}, // SpawnAndPrepareToSpit
         {Polyomino::SpawnState::WaitToPushPreview, 12}}, // SpitNewPreview
        {{Polyomino::SpawnState::OpenToPushPreview, 9},   // WaitToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 15},     // OpenToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 3},      // PreviewFliesUp
         {Polyomino::SpawnState::SpawnAndPrepareToSpit, 24}, // WaitToSpawn
         {Polyomino::SpawnState::SpitNewPreview, 15}, // SpawnAndPrepareToSpit
         {Polyomino::SpawnState::WaitToPushPreview, 9}}, // SpitNewPreview
        {{Polyomino::SpawnState::OpenToPushPreview, 6},  // WaitToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 10},    // OpenToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 2},     // PreviewFliesUp
         {Polyomino::SpawnState::SpawnAndPrepareToSpit, 16}, // WaitToSpawn
         {Polyomino::SpawnState::SpitNewPreview, 10}, // SpawnAndPrepareToSpit
         {Polyomino::SpawnState::WaitToPushPreview, 6}},    // SpitNewPreview
        {{Polyomino::SpawnState::OpenToPushPreview, 3},     // WaitToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 5},        // OpenToPushPreview
         {Polyomino::SpawnState::PreviewFliesUp, 1},        // PreviewFliesUp
         {Polyomino::SpawnState::SpawnAndPrepareToSpit, 8}, // WaitToSpawn
         {Polyomino::SpawnState::SpitNewPreview, 5}, // SpawnAndPrepareToSpit
         {Polyomino::SpawnState::WaitToPushPreview, 3}}, // SpitNewPreview
};

void Polyomino::spawn_update() {
  if (spawn_state_timer == 0) {
    switch (spawn_state) {
    case SpawnState::WaitToPushPreview:
      if (state == State::Active) {
        // we wait until the polyomino is inactive to start the spawn sequence
        return;
      }
      break;
    case SpawnState::OpenToPushPreview:
      multi_vram_buffer_horz(MountainTiles::OPEN_MOUTH, 2, NTADR_A(5, 5));
      preview_row = 3;
      break;
    case SpawnState::PreviewFliesUp:
      multi_vram_buffer_horz(MountainTiles::CLOSED_MOUTH, 2, NTADR_A(5, 5));
      if (preview_row == 0) {
        spawn_state = SpawnState::WaitToSpawn;
        multi_vram_buffer_horz(MountainTiles::EMPTY_PREVIEW, 2, NTADR_A(5, 0));
        multi_vram_buffer_horz(MountainTiles::EMPTY_PREVIEW, 2, NTADR_A(5, 1));
      } else {
        preview_row--;
        next->chibi_render(preview_row, 5);
        multi_vram_buffer_horz(MountainTiles::EMPTY_PREVIEW, 2,
                               NTADR_A(5, preview_row + 2));
      }
      break;
    case SpawnState::WaitToSpawn:
      break;
    case SpawnState::SpawnAndPrepareToSpit:
      spawn();
      multi_vram_buffer_horz(MountainTiles::OPEN_MOUTH, 2, NTADR_A(5, 5));
      break;
    case SpawnState::SpitNewPreview:
      next->chibi_render(3, 5);
      multi_vram_buffer_horz(MountainTiles::CLOSED_MOUTH, 2, NTADR_A(5, 5));
      break;
    }
  }
  spawn_state_timer++;

  if (spawn_state_timer >=
      spawn_state_frames[spawn_speed_tier][(u8)spawn_state]->duration) {
    spawn_state =
        spawn_state_frames[spawn_speed_tier][(u8)spawn_state]->next_state;
    spawn_state_timer = 0;
  }
}

void Polyomino::update(u8 drop_frames, bool &blocks_placed,
                       bool &failed_to_place, u8 &lines_cleared) {
  if (state == State::Inactive) {
    return;
  }
  // NOTE: since we've computed shadow row using collision, when we arrive at
  // it it means we are grounded
  if (row == shadow_row) {
    if (lock_down_timer >= MAX_LOCK_DOWN_TIMER ||
        lock_down_moves >= MAX_LOCK_DOWN_MOVES) {
      drop_timer = 0;
      action = Action::Idle;
      freezing_handler(blocks_placed, failed_to_place, lines_cleared);
    } else {
      lock_down_timer++;
    }
  } else {
    lock_down_timer = 0;
    lock_down_moves = 0;
    if (drop_timer++ >= drop_frames || action == Action::SoftDrop) {
      drop_timer = 0;
      row++;
      y += 16;
      if (current_controller_scheme == ControllerScheme::OnePlayer &&
          select_reminder == SelectReminder::WaitingRowToRemind) {
        select_reminder = SelectReminder::Reminding;
      }
    }
  }

  switch (action) {
  case Action::MoveLeft:
  case Action::MoveLeftAndRotateLeft:
  case Action::MoveLeftAndRotateRight:
    action = (Action)((u8)action & ~(u8)Action::MoveLeft);
    if (!collide(row, column - 1)) {
      column--;
      x -= 16;
      move_bitmask_left();
      if (lock_down_timer > 0) {
        lock_down_timer = 0;
        lock_down_moves++;
      }
    }
    break;
  case Action::MoveRight:
  case Action::MoveRightAndRotateLeft:
  case Action::MoveRightAndRotateRight:
    action = (Action)((u8)action & ~(u8)Action::MoveRight);
    if (!collide(row, column + 1)) {
      column++;
      x += 16;
      move_bitmask_right();
      if (lock_down_timer > 0) {
        lock_down_timer = 0;
        lock_down_moves++;
      }
    }
    break;
  case Action::RotateRight:
    definition = definition->right_rotation;

    if (able_to_kick(definition->right_kick->deltas)) {
      GGSound::play_sfx(SFX::Rotate, GGSound::SFXPriority::One);
      update_bitmask();
      if (lock_down_timer > 0) {
        lock_down_timer = 0;
        lock_down_moves++;
      }
    } else {
      definition = definition->left_rotation; // undo rotation
    }
    action = Action::Idle;
    break;
  case Action::RotateLeft:
    definition = definition->left_rotation;

    if (able_to_kick(definition->left_kick->deltas)) {
      update_bitmask();
      GGSound::play_sfx(SFX::Rotate, GGSound::SFXPriority::One);
      if (lock_down_timer > 0) {
        lock_down_timer = 0;
        lock_down_moves++;
      }
    } else {
      definition = definition->right_rotation; // undo rotation
    }
    action = Action::Idle;
    break;
  case Action::HardDrop:
    row = shadow_row;
    y = shadow_y;
    freezing_handler(blocks_placed, failed_to_place, lines_cleared);
    action = Action::Idle;
    break;
  case Action::SoftDrop:
  case Action::Idle:
    action = Action::Idle;
    break;
  }

  update_shadow();
}

void Polyomino::render(int y_scroll) {
  if (state != State::Active)
    return;
  // XXX: extend signal if y feels negativish (y values after 0xe8 are likely an
  // underflow when a large polyomino is spawned)
  definition->render(x, (y >= 0xe8 ? (s16)(0xff00 | y) : y) - y_scroll);
  definition->shadow(x, shadow_y - y_scroll, (u8)(shadow_row - row));
}

void Polyomino::outside_render(int y_scroll) {
  // XXX: extend signal if y feels negativish (y values after 0xe8 are likely an
  // underflow when a large polyomino is spawned)
  definition->render(x, (y >= 0xe8 ? (s16)(0xff00 | y) : y) - y_scroll);
}

void Polyomino::render_next() { next->chibi_render(3, 5); }

s8 Polyomino::freeze_blocks() {
  state = State::Inactive;
  lock_down_timer = 0;
  s8 filled_lines = 0;
  if (!definition->board_render(board, row, column)) {
    return -1;
  }

// XXX: checking the range of possible rows that could have been filled by a
// polyomino
#pragma clang loop unroll(full)
  for (u8 delta_row = 0; delta_row <= 3; delta_row++) {
    u8 block_row = (u8)(row + delta_row);
    if (board.row_filled(block_row)) {
      filled_lines++;
    }
  }

  return filled_lines;
}
