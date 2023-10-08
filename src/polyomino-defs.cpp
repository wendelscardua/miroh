#include "polyomino-defs.hpp"

bool PolyominoDef::collide(Board& board, s8 row, s8 column) const {
  for (u8 i = 0; i < size; i++) {
    auto delta = deltas[i];
    if (board.occupied(row + delta.delta_row,
                       column + delta.delta_column)) {
      return true;
    }
  }
  return false;
}
