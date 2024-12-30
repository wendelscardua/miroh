#pragma once

#include "common.hpp"
#include <neslib.h>

class WorldMap {
public:
  static constexpr u8 BANK = 3;

  bool available_stages[NUM_STAGES];

  u8 ending_frame_counter;

  WorldMap();
  ~WorldMap();
  void render_sprites();
  void stage_change(Stage new_stage);
  void loop();
  void tick_ending();
};
