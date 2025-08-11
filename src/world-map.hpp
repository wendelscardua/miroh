#pragma once

#include "common.hpp"
#include <neslib.h>

class WorldMap {
public:
  static constexpr u8 BANK = 3;

  __attribute__((noinline)) WorldMap();
  __attribute__((noinline)) ~WorldMap();
  __attribute__((noinline)) void loop();

private:
  bool available_stages[NUM_STAGES];
  u8 ending_frame_counter;

  void render_sprites();
  void stage_change(Stage new_stage);
  void tick_ending();
};
