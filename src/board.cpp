#include "board.hpp"
#include "attributes.hpp"
#include "bag.hpp"
#include "bank-helper.hpp"
#include "common.hpp"
#include "coroutine.hpp"
#include "maze-defs.hpp"
#include <cstdio>
#include <nesdoug.h>
#include <neslib.h>

Cell::Cell() : walls(0), parent(NULL) {}

void Cell::reset() {
  this->walls = 0;
  this->parent = this;
}

Cell *Cell::representative() {
  Cell *temp = this;
  while (temp != temp->parent) {
    temp->parent = temp->parent->parent;
    temp = temp->parent;
  }
  return temp;
}

void Cell::join(Cell *other) {
  Cell *x = this->representative();
  Cell *y = other->representative();
  if (x != y)
    y->parent = x;
}

Board::Board(u8 origin_x, u8 origin_y)
    : origin_x(origin_x), origin_y(origin_y) {
  // reset tally
  for (u8 i = 0; i < HEIGHT; i++) {
    tally[i] = 0;
    deleted[i] = 0;
  }

  // reset walls
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      cell[i][j].reset();
    }
  }

#define NEED_WALL(direction)                                                   \
  (template_cell.maybe_##direction##_wall && ((rand8() & 0b11) == 0))
  banked_lambda(GET_BANK(mazes), [this]() {
    static_assert(sizeof(TemplateCell) == 1, "TemplateCell is too big");

    // read required walls from template
    for (u8 i = 0; i < HEIGHT; i++) {
      for (u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell = mazes[maze]->template_cells[i][j];
        if (template_cell.value != 0xff) {
          cell[i][j].walls = template_cell.walls;
        }
      }
    }

    // read "maybe" walls from template
    for (u8 i = 0; i < HEIGHT; i++) {
      for (u8 j = 0; j < WIDTH; j++) {
        TemplateCell template_cell = mazes[maze]->template_cells[i][j];

        if (template_cell.value == 0xff) {
          // use the old berzerk algorithm
          // assumes the cell is on the valid range
          switch (rand8() & 0b11) {
          case 0:
            cell[i][j].right_wall = true;
            cell[i][j + 1].left_wall = true;
            break;
          case 1:
            cell[i][j + 1].down_wall = true;
            cell[i + 1][j + 1].up_wall = true;
            break;
          case 2:
            cell[i + 1][j].right_wall = true;
            cell[i + 1][j + 1].left_wall = true;
            break;
          case 3:
            cell[i][j].down_wall = true;
            cell[i + 1][j].up_wall = true;
            break;
          }
          continue;
        }

        if (NEED_WALL(up)) {
          cell[i][j].up_wall = true;
          if (i > 0) {
            cell[i - 1][j].down_wall = true;
          }
        }
        if (NEED_WALL(down)) {
          cell[i][j].down_wall = true;
          if (i < HEIGHT - 1) {
            cell[i + 1][j].up_wall = true;
          }
        }
        if (NEED_WALL(left)) {
          cell[i][j].left_wall = true;
          if (j > 0) {
            cell[i][j - 1].right_wall = true;
          }
        }
        if (NEED_WALL(right)) {
          cell[i][j].right_wall = true;
          if (j < WIDTH - 1) {
            cell[i][j + 1].left_wall = true;
          }
        }
      }
    }
  });

  // border walls
  for (u8 i = 0; i < HEIGHT; i++) {
    cell[i][0].left_wall = true;
    cell[i][WIDTH - 1].right_wall = true;
  }

  for (u8 j = 0; j < WIDTH; j++) {
    cell[0][j].up_wall = true;
    cell[HEIGHT - 1][j].down_wall = true;
  }

  // union-find-ish-ly ensure all cells are reachable
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      auto current_cell = &cell[i][j];
      if (j < WIDTH - 1 && !current_cell->right_wall) {
        current_cell->join(&cell[i][j + 1]);
      }
      if (i < HEIGHT - 1 && !current_cell->down_wall) {
        current_cell->join(&cell[i + 1][j]);
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
      auto current_cell = &cell[i][j];
      auto right_cell = j < WIDTH - 1 ? &cell[i][j + 1] : NULL;
      auto down_cell = i < HEIGHT - 1 ? &cell[i + 1][j] : NULL;

      // randomize if we are looking first horizontally or vertically
      bool down_first = rand8() & 0b1;

      if (down_first && down_cell &&
          current_cell->representative() != down_cell->representative()) {
        current_cell->down_wall = false;
        down_cell->up_wall = false;
        down_cell->join(current_cell);
      }

      if (right_cell &&
          current_cell->representative() != right_cell->representative()) {
        current_cell->right_wall = false;
        right_cell->left_wall = false;
        right_cell->join(current_cell);
      }

      if (!down_first && down_cell &&
          current_cell->representative() != down_cell->representative()) {
        current_cell->down_wall = false;
        down_cell->up_wall = false;
        down_cell->join(current_cell);
      }
    }
  }

  // make all cells free - from this point on we don't use "parent" anymore
  for (u8 i = 0; i < HEIGHT; i++) {
    for (u8 j = 0; j < WIDTH; j++) {
      cell[i][j].occupied = false;
    }
  }
}

Board::~Board() {}

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

  return cell[row][column].occupied;
}

void Board::occupy(s8 row, s8 column) {
  if (!cell[row][column].occupied) { // just to be safe
    cell[row][column].occupied = true;
    tally[row]++;
  }
}

void Board::free(s8 row, s8 column) {
  if (cell[row][column].occupied) { // just to be safe
    cell[row][column].occupied = false;
    tally[row]--;
  }
}

void Board::block_maze_cell(s8 row, s8 column) {
  block_maze_cell(row, column, false);
}

void Board::block_maze_cell(s8 row, s8 column, bool jiggling) {
  char metatile_top[2];
  char metatile_bottom[2];

  int position =
      NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));

  // TODO: maybe read from Edges session?
  if (jiggling) {
    if (row == 0) {
      if (column == 0) {
        metatile_top[0] = 0x6a;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x72;
        metatile_bottom[1] = 0x79;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x6b;
        metatile_bottom[0] = 0x78;
        metatile_bottom[1] = 0x73;
      } else {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x78;
        metatile_bottom[1] = 0x79;
      }
    } else if (row == HEIGHT - 1) {
      if (column == 0) {
        metatile_top[0] = 0x68;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x74;
        metatile_bottom[1] = 0x7a;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x69;
        metatile_bottom[0] = 0x7a;
        metatile_bottom[1] = 0x75;
      } else {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x7a;
        metatile_bottom[1] = 0x7a;
      }
    } else {
      if (column == 0) {
        metatile_top[0] = 0x68;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x72;
        metatile_bottom[1] = 0x79;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x69;
        metatile_bottom[0] = 0x78;
        metatile_bottom[1] = 0x73;
      } else {
        metatile_top[0] = 0x66;
        metatile_top[1] = 0x67;
        metatile_bottom[0] = 0x78;
        metatile_bottom[1] = 0x79;
      }
    }
  } else {
    if (row == 0) {
      if (column == 0) {
        metatile_top[0] = 0x64;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x72;
        metatile_bottom[1] = 0x71;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x65;
        metatile_bottom[0] = 0x70;
        metatile_bottom[1] = 0x73;
      } else {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x70;
        metatile_bottom[1] = 0x71;
      }
    } else if (row == HEIGHT - 1) {
      if (column == 0) {
        metatile_top[0] = 0x62;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x74;
        metatile_bottom[1] = 0x77;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x63;
        metatile_bottom[0] = 0x76;
        metatile_bottom[1] = 0x75;
      } else {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x76;
        metatile_bottom[1] = 0x77;
      }
    } else {
      if (column == 0) {
        metatile_top[0] = 0x62;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x72;
        metatile_bottom[1] = 0x71;
      } else if (column == WIDTH - 1) {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x63;
        metatile_bottom[0] = 0x70;
        metatile_bottom[1] = 0x73;
      } else {
        metatile_top[0] = 0x60;
        metatile_top[1] = 0x61;
        metatile_bottom[0] = 0x70;
        metatile_bottom[1] = 0x71;
      }
    }
  }

  multi_vram_buffer_horz(metatile_top, 2, position);
  multi_vram_buffer_horz(metatile_bottom, 2, position + 0x20);
  Attributes::set((u8)((origin_x >> 4) + column), (u8)((origin_y >> 4) + row),
                  BLOCK_ATTRIBUTE);
  occupy(row, column);
}

static const Cell null_cell;

static constexpr u8 upper_left_block_tile[] = {
    Board::TILE_BASE + 0x0f, Board::TILE_BASE + 0x18, Board::TILE_BASE + 0x16,
    Board::TILE_BASE + 0x16, Board::TILE_BASE + 0x14, Board::TILE_BASE + 0x0e,
    Board::TILE_BASE + 0x11, Board::TILE_BASE + 0x11, Board::TILE_BASE + 0x18,
    Board::TILE_BASE + 0x18, Board::TILE_BASE + 0x12, Board::TILE_BASE + 0x16,
    Board::TILE_BASE + 0x14, Board::TILE_BASE + 0x14, Board::TILE_BASE + 0x11,
    Board::TILE_BASE + 0x11};

static constexpr u8 upper_right_block_tile[] = {
    Board::TILE_BASE + 0x0f, Board::TILE_BASE + 0x15, Board::TILE_BASE + 0x15,
    Board::TILE_BASE + 0x15, Board::TILE_BASE + 0x10, Board::TILE_BASE + 0x0d,
    Board::TILE_BASE + 0x10, Board::TILE_BASE + 0x10, Board::TILE_BASE + 0x17,
    Board::TILE_BASE + 0x17, Board::TILE_BASE + 0x12, Board::TILE_BASE + 0x17,
    Board::TILE_BASE + 0x13, Board::TILE_BASE + 0x13, Board::TILE_BASE + 0x13,
    Board::TILE_BASE + 0x13};

static constexpr u8 lower_left_block_tile[] = {
    Board::TILE_BASE + 0x0f, Board::TILE_BASE + 0x08, Board::TILE_BASE + 0x01,
    Board::TILE_BASE + 0x06, Board::TILE_BASE + 0x04, Board::TILE_BASE + 0x0e,
    Board::TILE_BASE + 0x01, Board::TILE_BASE + 0x06, Board::TILE_BASE + 0x04,
    Board::TILE_BASE + 0x08, Board::TILE_BASE + 0x02, Board::TILE_BASE + 0x06,
    Board::TILE_BASE + 0x04, Board::TILE_BASE + 0x08, Board::TILE_BASE + 0x01,
    Board::TILE_BASE + 0x06};

static constexpr u8 lower_right_block_tile[] = {
    Board::TILE_BASE + 0x0f, Board::TILE_BASE + 0x05, Board::TILE_BASE + 0x00,
    Board::TILE_BASE + 0x05, Board::TILE_BASE + 0x00, Board::TILE_BASE + 0x0d,
    Board::TILE_BASE + 0x00, Board::TILE_BASE + 0x05, Board::TILE_BASE + 0x03,
    Board::TILE_BASE + 0x07, Board::TILE_BASE + 0x02, Board::TILE_BASE + 0x07,
    Board::TILE_BASE + 0x03, Board::TILE_BASE + 0x07, Board::TILE_BASE + 0x03,
    Board::TILE_BASE + 0x07};

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

void Board::restore_maze_cell(s8 row, s8 column) {
  auto current_cell = &cell[row][column];
  int position =
      NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));
  char metatile_top[2];
  char metatile_bottom[2];

  auto upper_cell = row > 0 ? &cell[row - 1][column] : &null_cell;
  auto lower_cell = row < HEIGHT - 1 ? &cell[row + 1][column] : &null_cell;
  auto left_cell = column > 0 ? &cell[row][column - 1] : &null_cell;
  auto right_cell = column < WIDTH - 1 ? &cell[row][column + 1] : &null_cell;

  metatile_top[0] = upper_left_block_tile[walls_to_index(
      upper_cell->left_wall, current_cell->up_wall, current_cell->left_wall,
      left_cell->up_wall)];
  metatile_top[1] = upper_right_block_tile[walls_to_index(
      upper_cell->right_wall, right_cell->up_wall, current_cell->right_wall,
      current_cell->up_wall)];
  metatile_bottom[0] = lower_left_block_tile[walls_to_index(
      current_cell->left_wall, current_cell->down_wall, lower_cell->left_wall,
      left_cell->down_wall)];
  metatile_bottom[1] = lower_right_block_tile[walls_to_index(
      current_cell->right_wall, right_cell->down_wall, lower_cell->right_wall,
      current_cell->down_wall)];

  if (row == 0) {
    if (column > 0 && current_cell->left_wall) {
      metatile_top[0] = TILE_BASE + 0x0a;
    }
    if (column < WIDTH - 1 && current_cell->right_wall) {
      metatile_top[1] = TILE_BASE + 0x09;
    }
  } else if (row == HEIGHT - 1) {
    if (column > 0 && current_cell->left_wall) {
      metatile_bottom[0] = TILE_BASE + 0x1a;
    }
    if (column < WIDTH - 1 && current_cell->right_wall) {
      metatile_bottom[1] = TILE_BASE + 0x19;
    }
  }

  if (column == 0) {
    if (row > 0 && current_cell->up_wall) {
      metatile_top[0] = TILE_BASE + 0x1b;
    } else if (row < HEIGHT - 1 && current_cell->down_wall) {
      metatile_bottom[0] = TILE_BASE + 0x0b;
    }
  } else if (column == WIDTH - 1) {
    if (row > 0 && current_cell->up_wall) {
      metatile_top[1] = TILE_BASE + 0x1c;
    } else if (row < HEIGHT - 1 && current_cell->down_wall) {
      metatile_bottom[1] = TILE_BASE + 0x0c;
    }
  }

  multi_vram_buffer_horz(metatile_top, 2, position);
  multi_vram_buffer_horz(metatile_bottom, 2, position + 0x20);

  Attributes::set((u8)((origin_x >> 4) + column), (u8)((origin_y >> 4) + row),
                  WALL_ATTRIBUTE);

  free(row, column);
}

bool Board::ongoing_line_clearing() {
  bool any_deleted = false;

  CORO_INIT;

  for (s8 i = 0; i < HEIGHT; i++) {
    if (tally[i] == WIDTH) {
      deleted[i] = true;
      any_deleted = true;
    }
  }

  if (!any_deleted) {
    CORO_FINISH(false);
  }

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

      // as a rule of thumb, we'll avoid updating too much on a single frame
      extern u8 VRAM_INDEX;
      if (VRAM_INDEX >= 32) {
        CORO_YIELD(true);
      }
    }
  }

  for (s8 i = 0; i < HEIGHT; i++) {
    deleted[i] = false;
  }

  CORO_FINISH(false);
}