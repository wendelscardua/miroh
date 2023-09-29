#pragma once

#include "common.hpp"
#define SIZE 12
#define TILE_BASE 0x70

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
  Cell cell[SIZE][SIZE];
public:
  Board();
  ~Board();

  void render(u8 x, u8 y);
};