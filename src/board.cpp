#include "board.hpp"
#include "attributes.hpp"
#include "common.hpp"
#include <nesdoug.h>
#include <neslib.h>

Cell::Cell() :
  walls(0),
  occupied(false),
  parent(this) {}
Cell *Cell::representative() {
  Cell *temp = this;
  while(temp != temp->parent)
    temp = temp->parent;
  this->parent = temp;
  return temp;
}

void Cell::join(Cell *other) {
  Cell *x = this->representative();
  Cell *y = other->representative();
  if (x != y) y->parent = x;
}

Board::Board(u8 origin_x, u8 origin_y) : origin_x(origin_x), origin_y(origin_y) {
  // reset tally
  for(u8 i = 0; i < HEIGHT; i++) {
    tally[i] = 0;
    deleted[i] = 0;
  }
  cracking_row = -1;
  cracking_column = -1;
  erasing_row = -1;
  erasing_column = -1;
  dropping_column = -1;

  // some sparse random walls
  for(u8 i = 0; i < HEIGHT - 1; i++) {
    for(u8 j = 0; j < WIDTH - 1; j++) {
      switch(rand8() & 0b11) {
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
    }
  }

  // border walls
  for(u8 i = 0; i < HEIGHT; i++) {
    cell[i][0].left_wall = true;
    cell[i][WIDTH - 1].right_wall = true;
  }

  for(u8 j = 0; j < WIDTH; j++) {
    cell[0][j].up_wall = true;
    cell[HEIGHT - 1][j].down_wall = true;
  }

  // union-find-ish-ly ensure all cells are reachable
  for(u8 i = 0; i < HEIGHT; i++) {
    for(u8 j = 0; j < WIDTH; j++) {
      auto current_cell = &cell[i][j];
      if (j < WIDTH - 1 && !current_cell->right_wall) {
        current_cell->join(&cell[i][j+1]);
      }
      if (i < HEIGHT - 1 && !current_cell->down_wall) {
        current_cell->join(&cell[i+1][j]);
      }
    }
  }

  for(u8 i = 0; i < HEIGHT; i++) {
    for(u8 j = 0; j < WIDTH; j++) {
      auto current_cell = &cell[i][j];
      auto right_cell = j < WIDTH - 1 ? &cell[i][j + 1] : NULL;
      auto down_cell = i < HEIGHT -1 ? &cell[i + 1][j] : NULL;

      if (right_cell && current_cell->representative() != right_cell->representative()) {
        current_cell->right_wall = false;
        right_cell->left_wall = false;
        right_cell->join(current_cell);
      }

      if (down_cell && current_cell->representative() != down_cell->representative()) {
        current_cell->down_wall = false;
        down_cell->up_wall = false;
        down_cell->join(current_cell);
      }
    }
  }
}

Board::~Board() {}

void Board::render() {
  u8 x = origin_x >> 3;
  u8 y = origin_y >> 3;
  for(u8 i = 0; i < HEIGHT; i++) {
    for(u8 j = 0; j < WIDTH; j++) {
      auto current_cell = &cell[i][j];

      vram_adr(NTADR_A(x + 2 * j, y + 2 * i));

      if (current_cell->up_wall) {
        if (current_cell->left_wall) { // up left
          vram_put(TILE_BASE + 7);
        } else { // up !left
          vram_put(TILE_BASE + 1);
        }
      } else {
        if (current_cell->left_wall) { // !up left
          vram_put(TILE_BASE + 6);
        } else { // !up !left
          if (i == 0 || j == 0)
            vram_put(TILE_BASE + 0);
          else
            vram_put(TILE_BASE + 9);
        }
      }

      // top right tile (vram_adr auto advanced)
      if (current_cell->up_wall) {
        if (current_cell->right_wall) { // up right
          vram_put(TILE_BASE + 3);
        } else { // up !right
          vram_put(TILE_BASE + 1);
        }
      } else {
        if (current_cell->right_wall) { // !up right
          vram_put(TILE_BASE + 2);
        } else { // !up !right
          if (i == 0 || j == WIDTH - 1)
            vram_put(TILE_BASE + 0);
          else
            vram_put(TILE_BASE + 10);
        }
      }

      vram_adr(NTADR_A(x + 2 * j, y + 2 * i + 1));

      // bottom left tile
      if (current_cell->down_wall) {
        if (current_cell->left_wall) { // down left
          vram_put(TILE_BASE + 8);
        } else { // down !left
          vram_put(TILE_BASE + 4);
        }
      } else {
        if (current_cell->left_wall) { // !down left
          vram_put(TILE_BASE + 6);
        } else { // !down !left
          if (i == HEIGHT - 1 || j == 0)
            vram_put(TILE_BASE + 0);
          else
            vram_put(TILE_BASE + 12);
        }
      }

      // bottom right tile (vram_adr auto advanced)
      if (current_cell->down_wall) {
        if (current_cell->right_wall) { // down right
          vram_put(TILE_BASE + 5);
        } else { // down !right
          vram_put(TILE_BASE + 4);
        }
      } else {
        if (current_cell->right_wall) { // !down right
          vram_put(TILE_BASE + 2);
        } else { // !down !right
          if (i == HEIGHT - 1 || j == WIDTH - 1)
            vram_put(TILE_BASE + 0);
          else
            vram_put(TILE_BASE + 11);
        }
      }
    }
  }
  for(u8 meta_x = origin_x >> 4; meta_x < ((origin_x >> 4) + WIDTH); meta_x++) {
    for(u8 meta_y = origin_y >> 4; meta_y < ((origin_y >> 4) + HEIGHT); meta_y++) {
      Attributes::set(meta_x, meta_y, WALL_ATTRIBUTE);
    }
  }
  Attributes::update_vram();
}

bool Board::occupied(s8 row, s8 column) {
  if (column < 0 ||
      column > WIDTH - 1 ||
      row > HEIGHT - 1)
    return true;

  if (row < 0) return false;

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
  int position = NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));
  multi_vram_buffer_horz((const u8[2]){0x74, 0x75}, 2, position);
  multi_vram_buffer_horz((const u8[2]){0x84, 0x85}, 2, position+0x20);
  Attributes::enable_vram_buffer();
  Attributes::set((u8) ((origin_x >> 4) + column), (u8) ((origin_y >> 4) + row), BLOCK_ATTRIBUTE);
  Attributes::flush_vram_update();
  occupy(row, column);
}

void Board::restore_maze_cell(s8 row, s8 column) {
  auto current_cell = &cell[row][column];
  int position = NTADR_A((origin_x >> 3) + (column << 1), (origin_y >> 3) + (row << 1));

  if (current_cell->up_wall) {
    if (current_cell->left_wall) { // up left
      one_vram_buffer(TILE_BASE + 7, position);
    } else { // up !left
      one_vram_buffer(TILE_BASE + 1, position);
    }
  } else {
    if (current_cell->left_wall) { // !up left
      one_vram_buffer(TILE_BASE + 6, position);
    } else { // !up !left
      if (row == 0 || column == 0)
        one_vram_buffer(TILE_BASE + 0, position);
      else
        one_vram_buffer(TILE_BASE + 9, position);
    }
  }

  // top right tile
  if (current_cell->up_wall) {
    if (current_cell->right_wall) { // up right
      one_vram_buffer(TILE_BASE + 3, position + 1);
    } else { // up !right
      one_vram_buffer(TILE_BASE + 1, position + 1);
    }
  } else {
    if (current_cell->right_wall) { // !up right
      one_vram_buffer(TILE_BASE + 2, position + 1);
    } else { // !up !right
      if (row == 0 || column == WIDTH - 1)
        one_vram_buffer(TILE_BASE + 0, position + 1);
      else
        one_vram_buffer(TILE_BASE + 10, position + 1);
    }
  }

  // bottom left tile
  if (current_cell->down_wall) {
    if (current_cell->left_wall) { // down left
      one_vram_buffer(TILE_BASE + 8, position + 0x20);
    } else { // down !left
      one_vram_buffer(TILE_BASE + 4, position + 0x20);
    }
  } else {
    if (current_cell->left_wall) { // !down left
      one_vram_buffer(TILE_BASE + 6, position + 0x20);
    } else { // !down !left
      if (row == HEIGHT - 1 || column == 0)
        one_vram_buffer(TILE_BASE + 0, position + 0x20);
      else
        one_vram_buffer(TILE_BASE + 12, position + 0x20);
    }
  }

  // bottom right tile (vram_adr auto advanced)
  if (current_cell->down_wall) {
    if (current_cell->right_wall) { // down right
      one_vram_buffer(TILE_BASE + 5, position + 0x21);
    } else { // down !right
      one_vram_buffer(TILE_BASE + 4, position + 0x21);
    }
  } else {
    if (current_cell->right_wall) { // !down right
      one_vram_buffer(TILE_BASE + 2, position + 0x21);
    } else { // !down !right
      if (row == HEIGHT - 1 || column == WIDTH - 1)
        one_vram_buffer(TILE_BASE + 0, position + 0x21);
      else
        one_vram_buffer(TILE_BASE + 11, position + 0x21);
    }
  }

  Attributes::enable_vram_buffer();
  Attributes::set((u8) ((origin_x >> 4) + column), (u8) ((origin_y >> 4) + row), WALL_ATTRIBUTE);
  Attributes::flush_vram_update();

  free(row, column);
}

bool Board::ongoing_line_clearing() {
  bool ongoing = false;

  if (cracking_row < 0) {
    for(s8 i = erasing_row + 1; i < HEIGHT; i++) {
      if (tally[i] == WIDTH) {
        deleted[i] = true;
        cracking_row = i;
        cracking_column = 0;
        ongoing = true;
        break;
      }
    }
  } else {
    ongoing = true;
    Attributes::enable_vram_buffer();
    Attributes::set((u8) ((origin_x >> 4) + cracking_column), (u8) ((origin_y >> 4) + cracking_row), FLASH_ATTRIBUTE);
    Attributes::flush_vram_update();
    int position = NTADR_A((origin_x >> 3) + (cracking_column << 1), (origin_y >> 3) + (cracking_row << 1));
    multi_vram_buffer_horz((const u8[2]){0x76, 0x77}, 2, position);
    multi_vram_buffer_horz((const u8[2]){0x86, 0x87}, 2, position+0x20);

    cracking_column++;
    if (cracking_column == WIDTH) {
      if (erasing_row < 0) {
        erasing_row = cracking_row;
        erasing_column = 0;
      }
      cracking_row = -1;
    }
  }

  if (erasing_row >= 0) {
    ongoing = true;

    restore_maze_cell(erasing_row, erasing_column);

    erasing_column++;
    if (erasing_column == WIDTH) {
      erasing_column = 0;

      for(erasing_row++; erasing_row < HEIGHT; erasing_row++) {
        if (tally[erasing_row] == WIDTH) break;
      }

      if (erasing_row == HEIGHT) {
        erasing_row = -1;
      }
    }
  }

  if (dropping_column < 0) {
    if (!ongoing && erasing_row < 0 && cracking_row < 0 && line_gravity_enabled) {
      // dropping column will only start doing stuff if it ever becomes zero
      for(auto deleted_row : deleted) {
        if (deleted_row) {
          dropping_column = 0;
          ongoing = true;
          break;
        }
      }
    }
  } else {
    ongoing = true;

    bool new_states[HEIGHT];
    for(s8 i = 0; i < HEIGHT; i++) {
      new_states[i] = occupied(i, dropping_column);
    }

    s8 current_row = HEIGHT - 1;
    s8 source_row = current_row;

    while(current_row >= 0) {
      while (source_row >= 0 && deleted[source_row]) {
        source_row--;
      }
      if (source_row < current_row) {
        bool source_occupied = source_row < 0 ? false : new_states[source_row];

        if (source_occupied) {
          new_states[current_row] = true;
          if (source_row >= 0) new_states[source_row] = false;
        } else {
          new_states[current_row] = false;
        }
      }

      current_row--;
      source_row--;
    }

    for(s8 i = 0; i < HEIGHT; i++) {
      if (new_states[i] != occupied(i, dropping_column)) {
        if (new_states[i]) {
          block_maze_cell(i, dropping_column);
        } else {
          restore_maze_cell(i, dropping_column);
        }
      }
    }

    dropping_column++;
    if (dropping_column == WIDTH) {
      for(s8 i = 0; i < HEIGHT; i++) {
        deleted[i] = false;
      }
      dropping_column = -1;
    }
  }

  return ongoing;
}
