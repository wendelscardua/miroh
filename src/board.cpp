#include "board.hpp"
#include "assets.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "ggsound.hpp"
#include "maze-defs.hpp"
#include "soundtrack.hpp"
#include "union-find.hpp"
#include "utils.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Cell::Cell() : walls(0) {}

#pragma clang section rodata = ".prg_rom_fixed.rodata.board"

const soa::Array<const u16, WIDTH> Board::OCCUPIED_BITMASK = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020,
    0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800};

#pragma clang section text = ".prg_rom_4.text.board"
#pragma clang section rodata = ".prg_rom_4.rodata.board"

static const u8 CELL_ROW_START[] = {0,         WIDTH,     2 * WIDTH, 3 * WIDTH,
                                    4 * WIDTH, 5 * WIDTH, 6 * WIDTH, 7 * WIDTH,
                                    8 * WIDTH, 9 * WIDTH};

static_assert(sizeof(CELL_ROW_START) == HEIGHT,
              "CELL_ROW_START does not have HEIGHT entries");

__attribute__((section(".prg_rom_fixed.text.board"))) u8
board_index(u8 row, u8 column) {
  return CELL_ROW_START[row] + column;
}

bool BoardAnimation::paused = false;

BoardAnimation::BoardAnimation() : cells(NULL), finished(true), length(0) {}

BoardAnimation::BoardAnimation(const BoardAnimFrame (*cells)[], u8 length,
                               u8 row, u8 column)
    : cells(cells), current_cell(&(*cells)[0]), current_frame(0), row(row),
      column(column), finished(false), current_cell_index(0), length(length) {}

void BoardAnimation::update() {
  if (paused || finished)
    goto exit;
  current_frame++;
  if (current_frame >= current_cell->duration) {
    current_frame = 0;
    current_cell_index++;
    if (current_cell_index == length) {
      finished = true;
    } else {
      current_cell++;
    }
  }
exit:
}

Cell &Board::cell_at(u8 row, u8 column) {
  return this->cell[board_index(row, column)];
}

Board::Board() : animations({}), active_animations(false) {}

const Maze stage_mazes[] = {Maze::NewNormal, Maze::Onion, Maze::Shelves,
                            Maze::Normal, Maze::Normal};

void Board::generate_maze() {
  // reset walls
  for (auto &one_cell : cell) {
    one_cell.walls = 0;
  }

#define NEED_WALL(direction)                                                   \
  (template_cell.maybe_##direction##_wall && (RAND_UP_TO_POW2(2) == 0))

  static_assert(sizeof(TemplateCell) == 1, "TemplateCell is too big");

  Maze maze = stage_mazes[(u8)current_stage];
  // read required walls from template
  for (u8 i = 0, index = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      TemplateCell template_cell = mazes[(u8)maze]->template_cells[index];
      if (template_cell.value != 0xff) {
        cell[index].walls = template_cell.walls;
      }
      index++;
    }
  }

  // read "maybe" walls from template
  for (u8 i = 0, index = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      TemplateCell template_cell = mazes[(u8)maze]->template_cells[index];

      if (template_cell.value == 0xff) {
        // use the old berzerk algorithm
        // assumes the cell is on the valid range
        switch (RAND_UP_TO_POW2(2)) {
        case 0:
          cell[index].right_wall = true;
          cell[index + 1].left_wall = true;
          break;
        case 1:
          cell[index + 1].down_wall = true;
          cell[index + WIDTH + 1].up_wall = true;
          break;
        case 2:
          cell[index + WIDTH].right_wall = true;
          cell[index + WIDTH + 1].left_wall = true;
          break;
        case 3:
          cell[index].down_wall = true;
          cell[index + WIDTH].up_wall = true;
          break;
        }
      } else {
        if (NEED_WALL(up)) {
          cell[index].up_wall = true;
          if (i > 0) {
            cell[index - WIDTH].down_wall = true;
          }
        }
        if (NEED_WALL(down)) {
          cell[index].down_wall = true;
          if (i < HEIGHT - 1) {
            cell[index + WIDTH].up_wall = true;
          }
        }
        if (NEED_WALL(left)) {
          cell[index].left_wall = true;
          if (j > 0) {
            cell[index - 1].right_wall = true;
          }
        }
        if (NEED_WALL(right)) {
          cell[index].right_wall = true;
          if (j < WIDTH - 1) {
            cell[index + 1].left_wall = true;
          }
        }
      }
      index++;
    }
  }

  // border walls
  for (u8 i = 0; i < HEIGHT; i++) {
    cell_at(i, 0).left_wall = true;
    cell_at(i, WIDTH - 1).right_wall = true;
  }

  for (u8 j = 0; j < WIDTH; j++) {
    cell_at(0, j).up_wall = true;
    cell_at(HEIGHT - 1, j).down_wall = true;
  }

  Set disjoint_set[HEIGHT * WIDTH];

  // union-find-ish-ly ensure all cells are reachable
  for (u8 i = 0, index = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      auto current_element = &disjoint_set[index];
      if (j < WIDTH - 1 && !cell[index].right_wall) {
        current_element->join(&disjoint_set[index + 1]);
      }
      if (i < HEIGHT - 1 && !cell[index].down_wall) {
        current_element->join(&disjoint_set[index + WIDTH]);
      }
      index++;
    }
  }

  // temporarily using bitset to mark visited cells
  for (u8 i = 0; i < HEIGHT; i++) {
    occupied_bitset[i] = 0;
  }

  // randomize visit order of cells

  u8 random_row;
  while ((random_row = random_free_row()) != 0xff) {
    u8 random_column = random_free_column(random_row);
    u8 index = board_index(random_row, random_column);

    occupy(random_row, random_column);

    auto current_element = &disjoint_set[index];
    auto right_element =
        random_column < WIDTH - 1 ? &disjoint_set[index + 1] : NULL;
    auto down_element =
        random_row < HEIGHT - 1 ? &disjoint_set[index + WIDTH] : NULL;

    // randomize if we are looking first horizontally or vertically
    bool down_first = rand8() & 0b1;

    if (down_first && down_element &&
        current_element->representative() != down_element->representative()) {
      cell[index].down_wall = false;
      cell[index + WIDTH].up_wall = false;
      down_element->join(current_element);
    }

    if (right_element &&
        current_element->representative() != right_element->representative()) {
      cell[index].right_wall = false;
      cell[index + 1].left_wall = false;
      right_element->join(current_element);
    }

    if (down_element &&
        current_element->representative() != down_element->representative()) {
      cell[index].down_wall = false;
      cell[index + WIDTH].up_wall = false;
      down_element->join(current_element);
    }
  }
}

void Board::reset() {
  // make all cells free
  for (u8 i = 0; i < HEIGHT; i++) {
    occupied_bitset[i] = 0;
  }

  // reset animations
  for (auto animation : animations) {
    animation.finished = true;
  }
  active_animations = false;
}

void Board::render() {
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      set_maze_cell(
          i, j, occupied((s8)i, j) ? CellType::Marshmallow : CellType::Maze);
      flush_vram_update2();
    }
  }
}

bool Board::occupied(s8 row, u8 column) {
  if (column > WIDTH - 1 || row > HEIGHT - 1)
    return true;

  if (row < 0)
    return false;

  return occupied_bitset[(u8)row] & OCCUPIED_BITMASK[(u8)column];
}

void Board::occupy(u8 row, u8 column) {
  occupied_bitset[row] |= OCCUPIED_BITMASK[column];
}

void Board::free(u8 row, u8 column) {
  occupied_bitset[row] &= ~OCCUPIED_BITMASK[column];
}

static const Cell null_cell;

const u8 upper_left_maze_tile[] = {0x00,
                                   MAZE_BASE_TILE + 0x18,
                                   MAZE_BASE_TILE + 0x16,
                                   MAZE_BASE_TILE + 0x16,
                                   MAZE_BASE_TILE + 0x14,
                                   MAZE_BASE_TILE + 0x0e,
                                   MAZE_BASE_TILE + 0x11,
                                   MAZE_BASE_TILE + 0x11,
                                   MAZE_BASE_TILE + 0x18,
                                   MAZE_BASE_TILE + 0x18,
                                   MAZE_BASE_TILE + 0x12,
                                   MAZE_BASE_TILE + 0x16,
                                   MAZE_BASE_TILE + 0x14,
                                   MAZE_BASE_TILE + 0x14,
                                   MAZE_BASE_TILE + 0x11,
                                   MAZE_BASE_TILE + 0x11};

const u8 upper_right_maze_tile[] = {0x00,
                                    MAZE_BASE_TILE + 0x15,
                                    MAZE_BASE_TILE + 0x15,
                                    MAZE_BASE_TILE + 0x15,
                                    MAZE_BASE_TILE + 0x10,
                                    MAZE_BASE_TILE + 0x0d,
                                    MAZE_BASE_TILE + 0x10,
                                    MAZE_BASE_TILE + 0x10,
                                    MAZE_BASE_TILE + 0x17,
                                    MAZE_BASE_TILE + 0x17,
                                    MAZE_BASE_TILE + 0x12,
                                    MAZE_BASE_TILE + 0x17,
                                    MAZE_BASE_TILE + 0x13,
                                    MAZE_BASE_TILE + 0x13,
                                    MAZE_BASE_TILE + 0x13,
                                    MAZE_BASE_TILE + 0x13};

const u8 lower_left_maze_tile[] = {0x00,
                                   MAZE_BASE_TILE + 0x08,
                                   MAZE_BASE_TILE + 0x01,
                                   MAZE_BASE_TILE + 0x06,
                                   MAZE_BASE_TILE + 0x04,
                                   MAZE_BASE_TILE + 0x0e,
                                   MAZE_BASE_TILE + 0x01,
                                   MAZE_BASE_TILE + 0x06,
                                   MAZE_BASE_TILE + 0x04,
                                   MAZE_BASE_TILE + 0x08,
                                   MAZE_BASE_TILE + 0x02,
                                   MAZE_BASE_TILE + 0x06,
                                   MAZE_BASE_TILE + 0x04,
                                   MAZE_BASE_TILE + 0x08,
                                   MAZE_BASE_TILE + 0x01,
                                   MAZE_BASE_TILE + 0x06};

const u8 lower_right_maze_tile[] = {0x00,
                                    MAZE_BASE_TILE + 0x05,
                                    MAZE_BASE_TILE + 0x00,
                                    MAZE_BASE_TILE + 0x05,
                                    MAZE_BASE_TILE + 0x00,
                                    MAZE_BASE_TILE + 0x0d,
                                    MAZE_BASE_TILE + 0x00,
                                    MAZE_BASE_TILE + 0x05,
                                    MAZE_BASE_TILE + 0x03,
                                    MAZE_BASE_TILE + 0x07,
                                    MAZE_BASE_TILE + 0x02,
                                    MAZE_BASE_TILE + 0x07,
                                    MAZE_BASE_TILE + 0x03,
                                    MAZE_BASE_TILE + 0x07,
                                    MAZE_BASE_TILE + 0x03,
                                    MAZE_BASE_TILE + 0x07};

const u8 lower_left_block_tile[] = {
    MARSHMALLOW_BASE_TILE + 0x02, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x04,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x0a, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08};

const u8 lower_right_block_tile[] = {
    MARSHMALLOW_BASE_TILE + 0x03, MARSHMALLOW_BASE_TILE + 0x07,
    MARSHMALLOW_BASE_TILE + 0x07, MARSHMALLOW_BASE_TILE + 0x07,
    MARSHMALLOW_BASE_TILE + 0x07, MARSHMALLOW_BASE_TILE + 0x05,
    MARSHMALLOW_BASE_TILE + 0x07, MARSHMALLOW_BASE_TILE + 0x07,
    MARSHMALLOW_BASE_TILE + 0x09, MARSHMALLOW_BASE_TILE + 0x09,
    MARSHMALLOW_BASE_TILE + 0x0b, MARSHMALLOW_BASE_TILE + 0x09,
    MARSHMALLOW_BASE_TILE + 0x09, MARSHMALLOW_BASE_TILE + 0x09,
    MARSHMALLOW_BASE_TILE + 0x09, MARSHMALLOW_BASE_TILE + 0x09};

// converts 4 boolean walls into a integer value between 0 and 15
u8 walls_to_index(bool wall_going_up, bool wall_going_right,
                  bool wall_going_down, bool wall_going_left) {
  u8 value = 0;
  if (wall_going_up) {
    value |= 0b0001;
  }
  if (wall_going_right) {
    value |= 0b0010;
  }
  if (wall_going_down) {
    value |= 0b0100;
  }
  if (wall_going_left) {
    value |= 0b1000;
  }
  return value;
}

/*
 VRAM BUFFER layout:
 0: address.h + horizontal (top)
 1: address.l
 2: length (2)
 3: tile (top 0)
 4: tile (top 1)
 5: address.h + horizontal (bottom)
 6: address.l
 7: length (2)
 8: tile (bottom 0)
 9: tile (bottom 1)
 10: eof (0xff)
 */

void Board::set_maze_cell(u8 row, u8 column, CellType cell_type) {
  u8 top_0, top_1, bottom_0, bottom_1;
  const auto current_cell_index = board_index(row, column);
  const auto current_cell = &cell[current_cell_index];
  int position =
      NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));

  if (cell_type == CellType::Maze) {
    free(row, column);
    const auto upper_cell =
        row > 0 ? &cell[current_cell_index - WIDTH] : &null_cell;
    const auto lower_cell =
        row < HEIGHT - 1 ? &cell[current_cell_index + WIDTH] : &null_cell;
    const auto left_cell =
        column > 0 ? &cell[current_cell_index - 1] : &null_cell;
    const auto right_cell =
        column < WIDTH - 1 ? &cell[current_cell_index + 1] : &null_cell;

    top_0 = upper_left_maze_tile[walls_to_index(
        upper_cell->left_wall, current_cell->up_wall, current_cell->left_wall,
        left_cell->up_wall)];
    top_1 = upper_right_maze_tile[walls_to_index(
        upper_cell->right_wall, right_cell->up_wall, current_cell->right_wall,
        current_cell->up_wall)];
    bottom_0 = lower_left_maze_tile[walls_to_index(
        current_cell->left_wall, current_cell->down_wall, lower_cell->left_wall,
        left_cell->down_wall)];
    bottom_1 = lower_right_maze_tile[walls_to_index(
        current_cell->right_wall, right_cell->down_wall, lower_cell->right_wall,
        current_cell->down_wall)];

    if (row == 0) {
      if (column > 0 && current_cell->left_wall) {
        top_0 = MAZE_BASE_TILE + 0x0a;
      }
      if (column < WIDTH - 1 && current_cell->right_wall) {
        top_1 = MAZE_BASE_TILE + 0x09;
      }
    } else if (row == HEIGHT - 1) {
      if (column > 0 && current_cell->left_wall) {
        bottom_0 = MAZE_BASE_TILE + 0x1a;
      }
      if (column < WIDTH - 1 && current_cell->right_wall) {
        bottom_1 = MAZE_BASE_TILE + 0x19;
      }
    }

    if (column == 0) {
      if (row > 0 && current_cell->up_wall) {
        top_0 = MAZE_BASE_TILE + 0x1b;
      }
      if (row < HEIGHT - 1 && current_cell->down_wall) {
        bottom_0 = MAZE_BASE_TILE + 0x0b;
      }
    } else if (column == WIDTH - 1) {
      if (row > 0 && current_cell->up_wall) {
        top_1 = MAZE_BASE_TILE + 0x1c;
      }
      if (row < HEIGHT - 1 && current_cell->down_wall) {
        bottom_1 = MAZE_BASE_TILE + 0x0c;
      }
    }

  } else {
    occupy(row, column);
    const auto lower_cell =
        row < HEIGHT - 1 ? &cell[current_cell_index + WIDTH] : &null_cell;
    const auto left_cell =
        column > 0 ? &cell[current_cell_index - 1] : &null_cell;
    const auto right_cell =
        column < WIDTH - 1 ? &cell[current_cell_index + 1] : &null_cell;

    top_0 = MARSHMALLOW_BASE_TILE;
    top_1 = MARSHMALLOW_BASE_TILE + 1;

    bottom_0 = lower_left_block_tile[walls_to_index(
        current_cell->left_wall, current_cell->down_wall, lower_cell->left_wall,
        left_cell->down_wall)];
    bottom_1 = lower_right_block_tile[walls_to_index(
        current_cell->right_wall, right_cell->down_wall, lower_cell->right_wall,
        current_cell->down_wall)];

    if (row == HEIGHT - 1) {
      if (column > 0 && current_cell->left_wall) {
        bottom_0 = MARSHMALLOW_BASE_TILE + 0x0a;
      }
      if (column < WIDTH - 1 && current_cell->right_wall) {
        bottom_1 = MARSHMALLOW_BASE_TILE + 0x0b;
      }
    }

    if (column == 0) {
      if (row < HEIGHT - 1 && current_cell->down_wall) {
        bottom_0 = MARSHMALLOW_BASE_TILE + 0x0a;
      }
    } else if (column == WIDTH - 1) {
      if (row < HEIGHT - 1 && current_cell->down_wall) {
        bottom_1 = MARSHMALLOW_BASE_TILE + 0x0b;
      }
    }

    switch (cell_type) {
    case CellType::Jiggling:
      top_0 += 0x10;
      top_1 += 0x10;
      bottom_0 += 0x10;
      bottom_1 += 0x10;
      break;
    case CellType::LeanLeft:
      top_0 = 0x4c;
      top_1 = 0x4d;
      break;
    case CellType::LeanRight:
      top_0 = 0x4e;
      top_1 = 0x4f;
      break;
    case CellType::Maze:
    case CellType::Marshmallow:
      break;
    }
  }

  // unrolled equivalent of...
  // multi_vram_buffer_horz(metatile_top, 2, position);
  // multi_vram_buffer_horz(metatile_bottom, 2, position + 0x20);

  VRAM_BUF[VRAM_INDEX] = (u8)(position >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 1] = (u8)position;
  VRAM_BUF[VRAM_INDEX + 2] = 2;
  VRAM_BUF[VRAM_INDEX + 3] = top_0;
  VRAM_BUF[VRAM_INDEX + 4] = top_1;
  VRAM_BUF[VRAM_INDEX + 5] = (u8)((position + 0x20) >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 6] = (u8)(position + 0x20);
  VRAM_BUF[VRAM_INDEX + 7] = 2;
  VRAM_BUF[VRAM_INDEX + 8] = bottom_0;
  VRAM_BUF[VRAM_INDEX + 9] = bottom_1;
  VRAM_BUF[VRAM_INDEX + 10] = 0xff;
  VRAM_INDEX += 10;

  // end of unrolled
}

bool Board::row_filled(u8 row) {
  return occupied_bitset[row] == FULL_ROW_BITMASK;
}

const SFX sfx_per_lines_cleared[] = {SFX::Lineclear1, SFX::Lineclear2,
                                     SFX::Lineclear3, SFX::Lineclear4};

bool Board::ongoing_line_clearing() {
  bool any_deleted = false;
  bool changed = false;
  u8 lines_cleared_for_sfx;
  static u16 column_mask;

  CORO_INIT;

  for (u8 i = 0; i < HEIGHT; i++) {
    if (row_filled(i)) {
      deleted[i] = true;
      any_deleted = true;
    }
  }

  if (!any_deleted) {
    CORO_FINISH(false);
  }

  lines_cleared_for_sfx = 0xff;
  for (u8 i = 0; i < HEIGHT; i++) {
    if (deleted[i]) {
      lines_cleared_for_sfx++;
    }
  }

  GGSound::play_sfx(sfx_per_lines_cleared[lines_cleared_for_sfx],
                    GGSound::SFXPriority::Two);

  for (erasing_row = HEIGHT - 1; erasing_row >= 0; erasing_row--) {
    if (deleted[erasing_row]) {
      for (erasing_column = 0; erasing_column < WIDTH; erasing_column++) {
        set_maze_cell((u8)erasing_row, erasing_column, CellType::Maze);
        CORO_YIELD(true);
      }
    }
  }

  for (erasing_column = 0, column_mask = 1; erasing_column < WIDTH;
       erasing_column++, column_mask <<= 1) {
    erasing_row = HEIGHT - 1;
    erasing_row_source = HEIGHT - 1;

    while (erasing_row >= 0) {
      while (erasing_row_source >= 0 && deleted[erasing_row_source]) {
        erasing_row_source--;
      }

      changed = false;
      if (erasing_row_source < erasing_row) {
        bool source_occupied =
            erasing_row_source < 0
                ? false
                : occupied_bitset[(u8)erasing_row_source] & column_mask;

        if (source_occupied) {
          if (!(occupied_bitset[(u8)erasing_row] & column_mask)) {
            set_maze_cell((u8)erasing_row, erasing_column,
                          CellType::Marshmallow);
            changed = true;
          }
          set_maze_cell((u8)erasing_row_source, erasing_column, CellType::Maze);
          changed = true;
        } else if (occupied_bitset[(u8)erasing_row] & column_mask) {
          set_maze_cell((u8)erasing_row, erasing_column, CellType::Maze);
          changed = true;
        }
      }
      erasing_row--;
      erasing_row_source--;

      if (!changed) {
        continue;
      }

      CORO_YIELD(true);
    }
  }

  for (u8 i = 0; i < HEIGHT; i++) {
    deleted[i] = false;
  }

  CORO_FINISH(false);
}

u8 Board::random_free_row() {
  u8 possible_rows[HEIGHT];
  u8 max_possible_rows = 0;
  for (u8 i = 0; i < HEIGHT; i++) {
    if (!row_filled(i)) {
      possible_rows[max_possible_rows++] = i;
    }
  }
  if (max_possible_rows == 0) {
    return 0xff;
  }
  return possible_rows[RAND_UP_TO(max_possible_rows)];
}

u8 Board::random_free_column(u8 row) {
  u8 possible_columns[WIDTH];
  u8 max_possible_columns = 0;
  u16 bits = occupied_bitset[row];
  for (u8 j = 0; j < WIDTH; j++) {
    if (!(bits & 0b1)) {
      possible_columns[max_possible_columns++] = j;
    }
    bits >>= 1;
  }
  return possible_columns[RAND_UP_TO(max_possible_columns)];
}

void Board::add_animation(BoardAnimation new_animation) {
  for (auto &animation : animations) {
    if (animation.finished) {
      animation = new_animation;
      break;
    }
  }
}

void Board::animate() {
  active_animations = false;
  for (auto &animation : animations) {
    if (animation.finished) {
      continue;
    }
    active_animations = true;
    if (animation.current_frame == 0) {
      set_maze_cell(animation.row, animation.column,
                    animation.current_cell->cell_type);
    }
    animation.update();
  }
}