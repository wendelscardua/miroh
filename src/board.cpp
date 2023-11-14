#include "board.hpp"
#include "assets.hpp"
#include "attributes.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "banked-asset-helpers.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "maze-defs.hpp"
#include "soundtrack.hpp"
#include "union-find.hpp"
#include "utils.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Cell::Cell() : walls(0) {}

static constexpr u8 CELL_ROW_START[] = {
    0,         WIDTH,     2 * WIDTH, 3 * WIDTH, 4 * WIDTH,
    5 * WIDTH, 6 * WIDTH, 7 * WIDTH, 8 * WIDTH, 9 * WIDTH};

static_assert(sizeof(CELL_ROW_START) == HEIGHT,
              "CELL_ROW_START does not have HEIGHT entries");

u8 board_index(u8 row, u8 column) { return CELL_ROW_START[row] + column; }

Cell &Board::cell_at(u8 row, u8 column) {
  return this->cell[board_index(row, column)];
}

Board::Board() {}

Board::~Board() {}

__attribute__((noinline, section(".prg_rom_0"))) void Board::generate_maze() {
  // reset walls
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      cell_at(i, j).walls = 0;
    }
  }

#define NEED_WALL(direction)                                                   \
  (template_cell.maybe_##direction##_wall && (RAND_UP_TO_POW2(2) == 0))

  {
    ScopedBank scopedBank(GET_BANK(mazes));
    static_assert(sizeof(TemplateCell) == 1, "TemplateCell is too big");

    // read required walls from template
    for (u8 i = 0; i < HEIGHT; i++) {
      for (u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell =
            mazes[maze]->template_cells[board_index(i, j)];
        if (template_cell.value != 0xff) {
          cell_at(i, j).walls = template_cell.walls;
        }
      }
    }

    // read "maybe" walls from template
    for (u8 i = 0; i < HEIGHT; i++) {
      for (u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell =
            mazes[maze]->template_cells[board_index(i, j)];

        if (template_cell.value == 0xff) {
          // use the old berzerk algorithm
          // assumes the cell is on the valid range
          switch (RAND_UP_TO_POW2(2)) {
          case 0:
            cell_at(i, j).right_wall = true;
            cell_at(i, j + 1).left_wall = true;
            break;
          case 1:
            cell_at(i, j + 1).down_wall = true;
            cell_at(i + 1, j + 1).up_wall = true;
            break;
          case 2:
            cell_at(i + 1, j).right_wall = true;
            cell_at(i + 1, j + 1).left_wall = true;
            break;
          case 3:
            cell_at(i, j).down_wall = true;
            cell_at(i + 1, j).up_wall = true;
            break;
          }
          continue;
        }

        if (NEED_WALL(up)) {
          cell_at(i, j).up_wall = true;
          if (i > 0) {
            cell_at(i - 1, j).down_wall = true;
          }
        }
        if (NEED_WALL(down)) {
          cell_at(i, j).down_wall = true;
          if (i < HEIGHT - 1) {
            cell_at(i + 1, j).up_wall = true;
          }
        }
        if (NEED_WALL(left)) {
          cell_at(i, j).left_wall = true;
          if (j > 0) {
            cell_at(i, j - 1).right_wall = true;
          }
        }
        if (NEED_WALL(right)) {
          cell_at(i, j).right_wall = true;
          if (j < WIDTH - 1) {
            cell_at(i, j + 1).left_wall = true;
          }
        }
      }
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
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      auto current_element = &disjoint_set[board_index(i, j)];
      if (j < WIDTH - 1 && !cell_at(i, j).right_wall) {
        current_element->join(&disjoint_set[board_index(i, j + 1)]);
      }
      if (i < HEIGHT - 1 && !cell_at(i, j).down_wall) {
        current_element->join(&disjoint_set[board_index(i + 1, j)]);
      }
    }
  }

  Bag<u8, HEIGHT> row_bag([](auto *bag) {
    for (u8 i = 0; i < HEIGHT; i++) {
      bag->insert(i);
    }
  });
  Bag<u8, WIDTH> column_bag([](auto *bag) {
    for (u8 j = 0; j < WIDTH; j++) {
      bag->insert(j);
    }
  });

  // randomize order of rows, then within a row, the order of columns
  for (u8 rows = 0; rows < HEIGHT; rows++) {
    u8 i = row_bag.take();
    for (u8 columns = 0; columns < WIDTH; columns++) {
      u8 j = column_bag.take();
      auto current_element = &disjoint_set[board_index(i, j)];
      auto right_element =
          j < WIDTH - 1 ? &disjoint_set[board_index(i, j + 1)] : NULL;
      auto down_element =
          i < HEIGHT - 1 ? &disjoint_set[board_index(i + 1, j)] : NULL;

      // randomize if we are looking first horizontally or vertically
      bool down_first = rand8() & 0b1;

      if (down_first && down_element &&
          current_element->representative() != down_element->representative()) {
        cell_at(i, j).down_wall = false;
        cell_at(i + 1, j).up_wall = false;
        down_element->join(current_element);
      }

      if (right_element && current_element->representative() !=
                               right_element->representative()) {
        cell_at(i, j).right_wall = false;
        cell_at(i, j + 1).left_wall = false;
        right_element->join(current_element);
      }

      if (!down_first && down_element &&
          current_element->representative() != down_element->representative()) {
        cell_at(i, j).down_wall = false;
        cell_at(i + 1, j).up_wall = false;
        down_element->join(current_element);
      }
    }
  }

  // make all cells free
  for (s8 i = 0; i < HEIGHT; i++) {
    for (s8 j = 0; j < WIDTH; j++) {
      free(i, j);
    }
  }
}

void Board::render() {
  for (s8 i = 0; i < HEIGHT; i++) {
    for (s8 j = 0; j < WIDTH; j++) {
      restore_maze_cell(i, j);
      flush_vram_update2();
    }
  }
}

bool Board::occupied(s8 row, s8 column) {
  if (column < 0 || column > WIDTH - 1 || row > HEIGHT - 1)
    return true;

  if (row < 0)
    return false;

  return occupied_bitset[(u8)row] & OCCUPIED_BITMASK[(u8)column];
}

void Board::occupy(s8 row, s8 column) {
  occupied_bitset[(u8)row] |= OCCUPIED_BITMASK[(u8)column];
}

void Board::free(s8 row, s8 column) {
  occupied_bitset[(u8)row] &= ~OCCUPIED_BITMASK[(u8)column];
}

static const Cell null_cell;

static constexpr u8 upper_left_maze_tile[] = {0x00,
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

static constexpr u8 upper_right_maze_tile[] = {0x00,
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

static constexpr u8 lower_left_maze_tile[] = {0x00,
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

static constexpr u8 lower_right_maze_tile[] = {0x00,
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

static constexpr u8 lower_left_block_tile[] = {
    MARSHMALLOW_BASE_TILE + 0x02, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x04,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x0a, MARSHMALLOW_BASE_TILE + 0x08,
    MARSHMALLOW_BASE_TILE + 0x06, MARSHMALLOW_BASE_TILE + 0x06,
    MARSHMALLOW_BASE_TILE + 0x08, MARSHMALLOW_BASE_TILE + 0x08};

static constexpr u8 lower_right_block_tile[] = {
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

void Board::block_maze_cell(s8 row, s8 column) {
  block_maze_cell(row, column, false);
}

extern u8 VRAM_INDEX;
extern char VRAM_BUF[256];

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

#define TOP_0 VRAM_BUF[VRAM_INDEX + 3]
#define TOP_1 VRAM_BUF[VRAM_INDEX + 4]
#define BOTTOM_0 VRAM_BUF[VRAM_INDEX + 8]
#define BOTTOM_1 VRAM_BUF[VRAM_INDEX + 9]

void Board::block_maze_cell(s8 row, s8 column, bool jiggling) {
  auto current_cell = &cell_at((u8)row, (u8)column);
  auto lower_cell =
      row < HEIGHT - 1 ? &cell_at((u8)row + 1, (u8)column) : &null_cell;
  auto left_cell = column > 0 ? &cell_at((u8)row, (u8)column - 1) : &null_cell;
  auto right_cell =
      column < WIDTH - 1 ? &cell_at((u8)row, (u8)column + 1) : &null_cell;
  int position =
      NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));

  TOP_0 = MARSHMALLOW_BASE_TILE;
  TOP_1 = MARSHMALLOW_BASE_TILE + 1;

  BOTTOM_0 = lower_left_block_tile[walls_to_index(
      current_cell->left_wall, current_cell->down_wall, lower_cell->left_wall,
      left_cell->down_wall)];
  BOTTOM_1 = lower_right_block_tile[walls_to_index(
      current_cell->right_wall, right_cell->down_wall, lower_cell->right_wall,
      current_cell->down_wall)];

  if (row == HEIGHT - 1) {
    if (column > 0 && current_cell->left_wall) {
      BOTTOM_0 = MARSHMALLOW_BASE_TILE + 0x0a;
    }
    if (column < WIDTH - 1 && current_cell->right_wall) {
      BOTTOM_1 = MARSHMALLOW_BASE_TILE + 0x0b;
    }
  }

  if (column == 0) {
    if (row < HEIGHT - 1 && current_cell->down_wall) {
      BOTTOM_0 = MARSHMALLOW_BASE_TILE + 0x0a;
    }
  } else if (column == WIDTH - 1) {
    if (row < HEIGHT - 1 && current_cell->down_wall) {
      BOTTOM_1 = MARSHMALLOW_BASE_TILE + 0x0b;
    }
  }

  if (jiggling) {
    TOP_0 += 0x10;
    TOP_1 += 0x10;
    BOTTOM_0 += 0x10;
    BOTTOM_1 += 0x10;
  }

  // unrolled equivalent of...
  // multi_vram_buffer_horz(metatile_top, 2, position);
  // multi_vram_buffer_horz(metatile_bottom, 2, position + 0x20);

  VRAM_BUF[VRAM_INDEX] = (u8)(position >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 1] = (u8)position;
  VRAM_BUF[VRAM_INDEX + 2] = 2;
  // 3, 4 = tiles already set
  VRAM_BUF[VRAM_INDEX + 5] = (u8)((position + 0x20) >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 6] = (u8)(position + 0x20);
  VRAM_BUF[VRAM_INDEX + 7] = 2;
  // 8, 9 = tiles already set
  VRAM_BUF[VRAM_INDEX + 10] = 0xff;
  VRAM_INDEX += 10;

  // end of unrolled

  Attributes::set((u8)(origin_column + column), (u8)(origin_row + row),
                  BLOCK_ATTRIBUTE);

  occupy(row, column);
}

void Board::restore_maze_cell(s8 row, s8 column) {
  auto current_cell = &cell_at((u8)row, (u8)column);
  int position =
      NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));

  auto upper_cell = row > 0 ? &cell_at((u8)row - 1, (u8)column) : &null_cell;
  auto lower_cell =
      row < HEIGHT - 1 ? &cell_at((u8)row + 1, (u8)column) : &null_cell;
  auto left_cell = column > 0 ? &cell_at((u8)row, (u8)column - 1) : &null_cell;
  auto right_cell =
      column < WIDTH - 1 ? &cell_at((u8)row, (u8)column + 1) : &null_cell;

  TOP_0 = upper_left_maze_tile[walls_to_index(
      upper_cell->left_wall, current_cell->up_wall, current_cell->left_wall,
      left_cell->up_wall)];
  TOP_1 = upper_right_maze_tile[walls_to_index(
      upper_cell->right_wall, right_cell->up_wall, current_cell->right_wall,
      current_cell->up_wall)];
  BOTTOM_0 = lower_left_maze_tile[walls_to_index(
      current_cell->left_wall, current_cell->down_wall, lower_cell->left_wall,
      left_cell->down_wall)];
  BOTTOM_1 = lower_right_maze_tile[walls_to_index(
      current_cell->right_wall, right_cell->down_wall, lower_cell->right_wall,
      current_cell->down_wall)];

  if (row == 0) {
    if (column > 0 && current_cell->left_wall) {
      TOP_0 = MAZE_BASE_TILE + 0x0a;
    }
    if (column < WIDTH - 1 && current_cell->right_wall) {
      TOP_1 = MAZE_BASE_TILE + 0x09;
    }
  } else if (row == HEIGHT - 1) {
    if (column > 0 && current_cell->left_wall) {
      BOTTOM_0 = MAZE_BASE_TILE + 0x1a;
    }
    if (column < WIDTH - 1 && current_cell->right_wall) {
      BOTTOM_1 = MAZE_BASE_TILE + 0x19;
    }
  }

  if (column == 0) {
    if (row > 0 && current_cell->up_wall) {
      TOP_0 = MAZE_BASE_TILE + 0x1b;
    } else if (row < HEIGHT - 1 && current_cell->down_wall) {
      BOTTOM_0 = MAZE_BASE_TILE + 0x0b;
    }
  } else if (column == WIDTH - 1) {
    if (row > 0 && current_cell->up_wall) {
      TOP_1 = MAZE_BASE_TILE + 0x1c;
    } else if (row < HEIGHT - 1 && current_cell->down_wall) {
      BOTTOM_1 = MAZE_BASE_TILE + 0x0c;
    }
  }

  // unrolled equivalent of...
  // multi_vram_buffer_horz(metatile_top, 2, position);
  // multi_vram_buffer_horz(metatile_bottom, 2, position + 0x20);

  VRAM_BUF[VRAM_INDEX] = (u8)(position >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 1] = (u8)position;
  VRAM_BUF[VRAM_INDEX + 2] = 2;
  // 3, 4 = tiles already set
  VRAM_BUF[VRAM_INDEX + 5] = (u8)((position + 0x20) >> 8) | 0x40;
  VRAM_BUF[VRAM_INDEX + 6] = (u8)(position + 0x20);
  VRAM_BUF[VRAM_INDEX + 7] = 2;
  // 8, 9 = tiles already set
  VRAM_BUF[VRAM_INDEX + 10] = 0xff;
  VRAM_INDEX += 10;

  // end of unrolled

  Attributes::set((u8)(origin_column + column), (u8)(origin_row + row),
                  WALL_ATTRIBUTE);

  free(row, column);
}

bool Board::row_filled(s8 row) {
  return occupied_bitset[(u8)row] == FULL_ROW_BITMASK;
}

const SFX sfx_per_lines_cleared[] = {SFX::Lineclear1, SFX::Lineclear2,
                                     SFX::Lineclear3, SFX::Lineclear4};

bool Board::ongoing_line_clearing(bool jiggling) {
  bool any_deleted = false;
  u8 lines_cleared_for_sfx;

  CORO_INIT;

  for (s8 i = 0; i < HEIGHT; i++) {
    if (row_filled(i)) {
      deleted[i] = true;
      any_deleted = true;
    }
  }

  if (!any_deleted) {
    CORO_FINISH(false);
  }

  while (jiggling) {
    CORO_YIELD(true);
  }

  lines_cleared_for_sfx = 0xff;
  for (u8 i = 0; i < HEIGHT; i++) {
    if (deleted[i]) {
      lines_cleared_for_sfx++;
    }
  }

  banked_play_sfx(sfx_per_lines_cleared[lines_cleared_for_sfx],
                  GGSound::SFXPriority::One);

  for (erasing_row = HEIGHT - 1; erasing_row >= 0; erasing_row--) {
    if (deleted[erasing_row]) {
      for (erasing_column = 0; erasing_column < WIDTH; erasing_column++) {
        restore_maze_cell(erasing_row, erasing_column);
        CORO_YIELD(true);
      }
    }
  }

  for (erasing_column = 0; erasing_column < WIDTH; erasing_column++) {
    erasing_row = HEIGHT - 1;
    erasing_row_source = HEIGHT - 1;

    while (erasing_row >= 0) {
      while (erasing_row_source >= 0 && deleted[erasing_row_source]) {
        erasing_row_source--;
      }

      if (erasing_row_source < erasing_row) {
        bool source_occupied =
            erasing_row_source < 0
                ? false
                : occupied(erasing_row_source, erasing_column);

        if (source_occupied) {
          if (!occupied(erasing_row, erasing_column)) {
            block_maze_cell(erasing_row, erasing_column);
            occupy(erasing_row, erasing_column);
          }
          if (erasing_row_source >= 0 &&
              occupied(erasing_row_source, erasing_column)) {
            restore_maze_cell(erasing_row_source, erasing_column);
          }
        } else if (occupied(erasing_row, erasing_column)) {
          restore_maze_cell(erasing_row, erasing_column);
        }
      }
      erasing_row--;
      erasing_row_source--;

      CORO_YIELD(true);
    }
  }

  for (s8 i = 0; i < HEIGHT; i++) {
    deleted[i] = false;
  }

  CORO_FINISH(false);
}
