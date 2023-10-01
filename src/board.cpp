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
  for(u8 i = 0; i < SIZE; i++) tally[i] = 0;
  cracking_row = -1;
  cracking_column = -1;
  erasing_row = -1;
  erasing_column = -1;

  // some sparse random walls
  for(u8 i = 0; i < SIZE - 1; i++) {
    for(u8 j = 0; j < SIZE - 1; j++) {
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

  // no border walls (implicit walls)
  for(u8 i = 0; i < SIZE; i++) {
    cell[0][i].up_wall = false;
    cell[SIZE - 1][i].down_wall = false;
    cell[i][0].left_wall = false;
    cell[i][SIZE - 1].right_wall = false;
  }

  // union-find-ish-ly ensure all cells are reachable
  for(u8 i = 0; i < SIZE; i++) {
    for(u8 j = 0; j < SIZE; j++) {
      auto current_cell = &cell[i][j];
      if (j < SIZE - 1 && !current_cell->right_wall) {
        current_cell->join(&cell[i][j+1]);
      }
      if (i < SIZE - 1 && !current_cell->down_wall) {
        current_cell->join(&cell[i+1][j]);
      }
    }
  }

  for(u8 i = 0; i < SIZE; i++) {
    for(u8 j = 0; j < SIZE; j++) {
      auto current_cell = &cell[i][j];
      auto right_cell = j < SIZE - 1 ? &cell[i][j + 1] : NULL;
      auto down_cell = i < SIZE -1 ? &cell[i + 1][j] : NULL;

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

void Board::render() {
  u8 x = origin_x >> 3;
  u8 y = origin_y >> 3;
  for(u8 i = 0; i < SIZE; i++) {
    for(u8 j = 0; j < SIZE; j++) {
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
          if (i == 0 || j == SIZE - 1)
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
          if (i == SIZE - 1 || j == 0)
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
          if (i == SIZE - 1 || j == SIZE - 1)
            vram_put(TILE_BASE + 0);
          else
            vram_put(TILE_BASE + 11);
        }
      }
    }
  }
  for(u8 meta_x = origin_x >> 4; meta_x < ((origin_x >> 4) + SIZE); meta_x++) {
    for(u8 meta_y = origin_y >> 4; meta_y < ((origin_y >> 4) + SIZE); meta_y++) {
      Attributes::set(meta_x, meta_y, WALL_ATTRIBUTE);
    }
  }
  Attributes::update_vram();
}

bool Board::occupied(s8 row, s8 column) {
  if (column < 0 ||
      column > SIZE - 1 ||
      row > SIZE - 1)
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

bool Board::ongoing_line_clearing() {
  bool ongoing = false;

  if (cracking_row < 0) {
    for(s8 i = erasing_row + 1; i < SIZE; i++) {
      if (tally[i] == SIZE) {
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
    multi_vram_buffer_horz((const u8[2]){0x06, 0x07}, 2, position);
    multi_vram_buffer_horz((const u8[2]){0x16, 0x17}, 2, position+0x20);

    cracking_column++;
    if (cracking_column == SIZE) {
      if (erasing_row < 0) {
        erasing_row = cracking_row;
        erasing_column = 0;
      }
      cracking_row = -1;
    }
  }

  if (erasing_row >= 0) {
    ongoing = true;
    Attributes::enable_vram_buffer();
    Attributes::set((u8) ((origin_x >> 4) + erasing_column), (u8) ((origin_y >> 4) + erasing_row), WALL_ATTRIBUTE);
    Attributes::flush_vram_update();

    // redrawing a maze cell

    auto current_cell = &cell[erasing_row][erasing_column];
    int position = NTADR_A((origin_x >> 3) + (erasing_column << 1), (origin_y >> 3) + (erasing_row << 1));

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
        if (erasing_row == 0 || erasing_column == 0)
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
        if (erasing_row == 0 || erasing_column == SIZE - 1)
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
        if (erasing_row == SIZE - 1 || erasing_column == 0)
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
        if (erasing_row == SIZE - 1 || erasing_column == SIZE - 1)
          one_vram_buffer(TILE_BASE + 0, position + 0x21);
        else
          one_vram_buffer(TILE_BASE + 11, position + 0x21);
      }
    }

    // end of "redrawing a maze cell"

    free(erasing_row, erasing_column);


    erasing_column++;
    if (erasing_column == SIZE) {
      erasing_column = 0;

      for(erasing_row++; erasing_row < SIZE; erasing_row++) {
        if (tally[erasing_row] == SIZE) break;
      }

      if (erasing_row == SIZE) {
        erasing_row = -1;
      }
    }
  }

  return ongoing;
}
