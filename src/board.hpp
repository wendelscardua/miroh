#pragma once

#include "board-animation.hpp"
#include "cell.hpp"
#include "common.hpp"
#include <soa.h>

static constexpr u8 HEIGHT = 10;
static constexpr u8 WIDTH = 12;

class Board {

  static constexpr u16 FULL_ROW_BITMASK = 0x0fff;

public:
  static constexpr u8 BANK = 4;

  // convert column into its bitmask
  static const soa::Array<const u16, WIDTH> OCCUPIED_BITMASK;

  static constexpr u8 origin_x = 0x20;
  static constexpr u8 origin_y = 0x30;

  soa::Array<u16, HEIGHT> occupied_bitset;
  Cell cell[HEIGHT * WIDTH]; // each of the board's cells
  bool deleted[HEIGHT]; // mark which rows were removed in case we apply gravity
  std::array<BoardAnimation, 15> animations;
  bool active_animations;

  static constexpr u8 origin_row =
      origin_y >> 4; // origin in metatile space (y)
  static constexpr u8 origin_column =
      origin_x >> 4; // origin in metatile space (x)

  __attribute__((section(".prg_rom_fixed.text.board"))) Board();

  // (re)generates the maze
  __attribute__((noinline)) void generate_maze();

  // reset for a new run
  __attribute__((noinline)) void reset();

  // draws board tiles (with rendering disabled)
  __attribute__((noinline)) void render();

  // tells if a cell is occupied by a solid block
  __attribute__((section(".prg_rom_fixed.text.board"))) bool
  occupied(s8 row, u8 column);

  // tells if a row if filled
  __attribute__((section(".prg_rom_fixed.text.board"))) bool row_filled(u8 row);

  // marks a position as occupied by a solid block
  __attribute__((section(".prg_rom_fixed.text.board"))) void occupy(u8 row,
                                                                    u8 column);

  // change a cell at these coordinates and with a given style
  __attribute__((noinline)) void set_maze_cell(u8 row, u8 column,
                                               CellType type);

  // advances the process of clearing a filled line
  // returns true if such process is still ongoing
  __attribute__((noinline)) bool ongoing_line_clearing();

  // returns index of a row with free space (or 0xff in case of failure)
  __attribute__((noinline)) u8 random_free_row();

  // returns index of a column with free space
  // (you passed a valid row so it should always succeed)
  __attribute__((noinline)) u8 random_free_column(u8 row);

  __attribute__((section(".prg_rom_fixed.text.board"))) Cell &
  cell_at(u8 row, u8 column);

  // enqueues a new animation
  __attribute__((noinline)) void add_animation(BoardAnimation animation);

  // animate cells
  __attribute__((noinline)) void animate();

private:
  s8 erasing_row;
  u8 erasing_column;
  s8 erasing_row_source;

  // marks a position as not occupied by a solid block
  __attribute__((section(".prg_rom_fixed.text.board"))) void free(u8 row,
                                                                  u8 column);
};
