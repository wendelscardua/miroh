#pragma once

#include "common.hpp"
#include <soa.h>

constexpr int PPU_PATTERN_TABLE_0 = 0x0000;
constexpr int PPU_PATTERN_TABLE_1 = 0x1000;
constexpr int PPU_PATTERN_TABLE_SIZE = 4096;
constexpr int PPU_PATTERN_ROW_SIZE = (4096 / 16);

constexpr u8 NUM_LEVELS = 5;

constexpr u8 ASSETS_BANK = 1;

enum class Location : u8 {
  StarlitStables,
  RainbowRetreat,
  FairyForest,
  GlitteryGrotto,
  MarshmallowMountain
};

extern "C" const soa::Array<char *, NUM_LEVELS> level_bg_palettes;
extern "C" const soa::Array<char *, NUM_LEVELS> level_spr_palettes;
extern "C" const soa::Array<char *, NUM_LEVELS> level_bg_tiles;
extern "C" const soa::Array<char *, NUM_LEVELS> level_spr_tiles;
extern "C" const soa::Array<char *, NUM_LEVELS> level_nametables;
extern "C" const soa::Array<char *, NUM_LEVELS> level_alt_nametables;

extern "C" const char title_nam[];
extern "C" const char how_to_nam[];
extern "C" const char credits_nam[];
