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
public:
  Cell cell[SIZE][SIZE];
  u8 origin_x;
  u8 origin_y;

  Board(u8 origin_x, u8 origin_y);
  ~Board();

  void render();
  bool occupied(s8 row, s8 column);
};
