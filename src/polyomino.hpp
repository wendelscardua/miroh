#include "bag.hpp"
#include "board.hpp"
#include "direction.hpp"
#include "input-mode.hpp"
#include "polyomino-defs.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define MAX_GROUNDED_TIMER 2
#define FROZEN_BLOCK_ATTRIBUTE 2

class Polyomino {
  static const s8 MOVEMENT_INITIAL_DELAY = 16;
  static const s8 MOVEMENT_DELAY = 6;

  static Bag<u8, NUM_POLYOMINOS> pieces;

  Board &board;
  const PolyominoDef *definition;
  const PolyominoDef *next;
  const PolyominoDef *second_next;
  s8 row;
  s8 column;
  u16 drop_timer;
  s8 move_timer;
  Direction movement_direction;

  bool able_to_kick(auto kick_deltas);

public:
  u8 grounded_timer;
  bool active;
  Polyomino(Board &board);

  void spawn();

  void handle_input(InputMode &input_mode, u8 pressed, u8 held);

  void update(u8 drop_frames, bool &blocks_placed, bool &failed_to_place,
              u8 &lines_filled);

  void banked_render();

  void render();

  void render_next();

  // checks if polyomino is already inside the board
  bool can_be_frozen();

  // returns number of filled lines aftter blocks were frozen
  u8 freeze_blocks();
};
