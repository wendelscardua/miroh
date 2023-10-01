#pragma once

#include "common.hpp"
#define SIZE 12
#define TILE_BASE 0x60
#define WALL_ATTRIBUTE 1
#define FLASH_ATTRIBUTE 3

class Cell {
 public:
  union {
    u8 walls : 4;
    struct {
      bool up_wall : 1;
      bool right_wall : 1;
      bool down_wall : 1;
      bool left_wall : 1;
    };
  };
  bool occupied : 1;
  Cell *parent;

  Cell();
  Cell *representative();
  void join(Cell *other);
};

class Board {
  s8 cracking_row;
  s8 cracking_column;
  s8 erasing_row;
  s8 erasing_column;
  u8 tally[SIZE]; // counts how many occupied cells are in each row
public:
  Cell cell[SIZE][SIZE]; // each of the board's cells
  u8 origin_x; // where to start rendering the board and its contents (x)
  u8 origin_y; // where to start rendering the board and its contents (y)

  Board(u8 origin_x, u8 origin_y);
  ~Board();

  // draws board tiles (with rendering disabled)
  void render();

  // tells if a cell is occupied by a solid block
  bool occupied(s8 row, s8 column);

  // marks a position as occupied by a solid block
  void occupy(s8 row, s8 column);

  // marks a position as not occupied by a solid block
  void free(s8 row, s8 column);

  // advances the process of clearing a filled line
  // returns true if such process is still ongoing
  bool ongoing_line_clearing();
};
