#include "board.hpp"
#include "input-mode.hpp"
#include "polyominos.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define DROP_FRAMES 60
#define MAX_GROUNDED_TIMER 2
#define FROZEN_BLOCK_ATTRIBUTE 2

class Polyomino {
  Board& board;
  PolyominoDef *definition;
  s8 row;
  s8 column;
  u8 grounded_timer;
  u8 drop_timer;
public:
  bool active;
  Polyomino(Board& board, bool active);

  void spawn();

  void update(InputMode &input_mode, u8 pressed, u8 held, bool &blocks_placed, u8 &lines_filled);

  void render();

  // returns number of filled lines aftter blocks were frozen
  u8 freeze_blocks();
};
