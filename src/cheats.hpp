#pragma once

#include "common.hpp"
class Cheats {
  u8 cheat_code[4];
  u8 cheat_code_index;

public:
  bool higher_level;
  bool infinite_energy;

  Cheats();

  void push_code(u8 code);
  void reset();
};

extern Cheats cheats;