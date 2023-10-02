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
  const PolyominoDef *definition;
  s8 row;
  s8 column;
  u8 drop_timer;
public:
  u8 grounded_timer;
  bool active;
  Polyomino(Board& board, bool active);

  void spawn();

  void update(InputMode &input_mode, u8 pressed, u8 held, bool &blocks_placed, bool &failed_to_place, u8 &lines_filled);

  void render();

  // checks if polyomino is already inside the board
  bool can_be_frozen();

  // returns number of filled lines aftter blocks were frozen
  u8 freeze_blocks();
};
