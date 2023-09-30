#include "board.hpp"
#include "fixed-point.hpp"
#include "input-mode.hpp"
#include "polyominos.hpp"
#include <nesdoug.h>
#include <neslib.h>

#define GRID_SIZE fixed_point(0x10, 0)

class Polyomino {
  Board& board;
  PolyominoDef *definition;
  fixed_point x;
  fixed_point y;
public:
  bool active;
  Polyomino(Board& board, bool active);

  void spawn();

  void update(InputMode input_mode, u8 pressed, u8 held);

  void render();
};
