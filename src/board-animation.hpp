#pragma once

#include "cell.hpp"
#include "common.hpp"

enum class BoardAnimTrigger {
  None,
  FallDown,
  DropFromAbove,
};

struct BoardAnimFrame {
  union {
    CellType cell_type;
    BoardAnimTrigger trigger;
  };
  u8 duration;
};

class BoardAnimation {
public:
  static constexpr u8 BANK = 4;

  static const BoardAnimFrame block_jiggle[];
  static const BoardAnimFrame block_move_right[];
  static const BoardAnimFrame block_move_left[];
  static const BoardAnimFrame block_arrive_right[];
  static const BoardAnimFrame block_arrive_left[];
  static const BoardAnimFrame block_break_right[];
  static const BoardAnimFrame block_break_left[];
  static const BoardAnimFrame block_start_falling[];
  static const BoardAnimFrame block_finish_falling[];

  const BoardAnimFrame (*cells)[];
  const BoardAnimFrame *current_cell;
  static bool paused;
  u8 current_frame;
  u8 row;
  u8 column;
  bool finished;

  BoardAnimation();

  BoardAnimation(const BoardAnimFrame (*cells)[], u8 row, u8 column);

  void reset();

  void update();
};
