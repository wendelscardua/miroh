#pragma once

#include "common.hpp"
#include <soa.h>

static constexpr u8 HEIGHT = 10;
static constexpr u8 WIDTH = 12;
static constexpr u8 WALL_ATTRIBUTE = 0b00000000;

static constexpr u8 BLOCK_ATTRIBUTE = 0b00000000;

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

  Cell();
};

class Board {
  // these are used for the coroutinish line clearing function
  static const u8 LINE_CLEARING_BUDGET = 4;

  // convert column into its bitmask
  static constexpr soa::Array<const u16, WIDTH> OCCUPIED_BITMASK = {
      0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020,
      0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800};

  static constexpr u16 FULL_ROW_BITMASK = 0x0fff;

  s8 erasing_row;
  s8 erasing_column;
  s8 erasing_row_source;

public:
  static constexpr u8 origin_x = 0x20;
  static constexpr u8 origin_y = 0x30;

  soa::Array<u16, HEIGHT> occupied_bitset;
  Cell cell[HEIGHT * WIDTH]; // each of the board's cells
  bool deleted[HEIGHT]; // mark which rows were removed in case we apply gravity

  static constexpr u8 origin_row =
      origin_y >> 4; // origin in metatile space (y)
  static constexpr u8 origin_column =
      origin_x >> 4; // origin in metatile space (x)

  static constexpr u8 MAZE_BANK = 0;

  Board();
  ~Board();

  // (re)generates the maze
  void generate_maze();

  // draws board tiles (with rendering disabled)
  void render();

  // tells if a cell is occupied by a solid block
  bool occupied(s8 row, s8 column);

  // tells if a row if filled
  bool row_filled(s8 row);

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
  bool ongoing_line_clearing(bool jiggling);

  // returns index of a row with free space (or 0xff in case of failure)
  u8 random_free_row();

  // returns index of a column with free space
  // (you passed a valid row so it should always succeed)
  u8 random_free_column(u8 row);

  Cell &cell_at(u8 row, u8 column);
};
