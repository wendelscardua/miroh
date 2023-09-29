#include "board.hpp"
#include "common.hpp"
#include <neslib.h>

Cell::Cell() : parent(this) {}
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

Board::Board() {
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

void Board::render(u8 x, u8 y) {
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
}
