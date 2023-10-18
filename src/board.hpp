#pragma once

#include "common.hpp"

constexpr u8 HEIGHT = 10;
constexpr u8 WIDTH = 12;
constexpr u8 WALL_ATTRIBUTE = 0;
constexpr u8 BLOCK_ATTRIBUTE = 0;

#define BOARD_X_ORIGIN 0x20
#define BOARD_Y_ORIGIN 0x30

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

  // true if a block is here
  union {
    bool occupied;
    // used for union-find
    Cell *parent;
  };

  Cell();
  void reset();
  Cell *representative();
  void join(Cell *other);
};

class Board {
  // these are used for the coroutinish line clearing function
  static const u8 LINE_CLEARING_BUDGET = 4;
  s8 erasing_row;
  s8 erasing_column;
  s8 erasing_row_source;

public:
  static constexpr u8 TILE_BASE = 0x40;
  Cell cell[HEIGHT][WIDTH]; // each of the board's cells
  u8 tally[HEIGHT];         // counts how many occupied cells are in each row
  bool deleted[HEIGHT]; // mark which rows were removed in case we apply gravity
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

  // draw a block and occupy these coordinates
  void block_maze_cell(s8 row, s8 column);
  void block_maze_cell(s8 row, s8 column, bool jiggling);

  // restore a maze andfree these coordinates
  void restore_maze_cell(s8 row, s8 column);

  // advances the process of clearing a filled line
  // returns true if such process is still ongoing
  bool ongoing_line_clearing();
};
