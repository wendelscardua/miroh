#pragma once

#include <array>
#include <soa.h>

#include "board.hpp"

#define NUM_MAZES 5
#define MAZE_NAME_WIDTH 10

struct TemplateCell {
  bool up_wall : 1;
  bool right_wall : 1;
  bool down_wall : 1;
  bool left_wall : 1;
  bool maybe_up_wall : 1;
  bool maybe_right_wall : 1;
  bool maybe_down_wall : 1;
  bool maybe_left_wall : 1;
};

struct MazeDef {
  const TemplateCell template_cells[HEIGHT][WIDTH];
};

typedef u8 Maze;

extern "C" const soa::Array<MazeDef*, NUM_MAZES> mazes;

extern "C" const char maze_names[NUM_MAZES][MAZE_NAME_WIDTH];

extern Maze maze;
