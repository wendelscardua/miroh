#pragma once

#include "common.hpp"

enum class CellType : u8 {
  Maze,
  Marshmallow,
  Jiggling,
  LeanLeft,
  LeanRight,
};

class Cell {
public:
  union {
    struct {
      u8 walls : 4;
    };
    struct {
      bool up_wall : 1;
      bool right_wall : 1;
      bool down_wall : 1;
      bool left_wall : 1;
    };
  };

  Cell();
};
