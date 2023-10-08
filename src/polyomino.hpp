#include "bag.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "input-mode.hpp"
#include "polyomino-defs.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define DROP_FRAMES 60
#define MAX_GROUNDED_TIMER 2
#define FROZEN_BLOCK_ATTRIBUTE 2

class Polyomino {
  static const s8 SIDEWAYS_INITIAL_DELAY = 16;
  static const s8 SIDEWAYS_DELAY = 6;

  static Bag<u8, 32> pieces;

  Board &board;
  const PolyominoDef *definition;
  const PolyominoDef *next;
  s8 row;
  s8 column;
  u8 drop_timer;
  s8 move_timer;
  Direction sideways_direction;

  bool able_to_kick(auto kick_deltas);

public:
  u8 grounded_timer;
  bool active;
  Polyomino(Board &board);

  void spawn();

  void handle_input(InputMode &input_mode, u8 pressed, u8 held);

  void update(bool &blocks_placed, bool &failed_to_place, u8 &lines_filled);

  void banked_render();

  void render();

  void render_next();

  // checks if polyomino is already inside the board
  bool can_be_frozen();

  // returns number of filled lines aftter blocks were frozen
  u8 freeze_blocks();
};
