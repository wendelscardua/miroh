#pragma once

#include "common.hpp"
#include <soa.h>

constexpr int PPU_PATTERN_TABLE_0 = 0x0000;
constexpr int PPU_PATTERN_TABLE_1 = 0x1000;

// prg bank index where all assets are stored
constexpr u8 ASSETS_BANK = 1;

// various base tiles
static constexpr u8 MAZE_BASE_TILE = 0x20;
static constexpr u8 MARSHMALLOW_BASE_TILE = 0x40;
static constexpr u8 DIGITS_BASE_TILE = 0x64;
static constexpr u8 DARK_ZERO_TILE = 0x6e;

constexpr u8 NUM_LEVELS = 5;

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

extern "C" const char title_nametable[];
extern "C" const char title_bg_tiles[];
extern "C" const char title_bg_palette[];

extern "C" const char how_to_nam[];
extern "C" const char credits_nam[];
