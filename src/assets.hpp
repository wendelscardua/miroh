#pragma once

#include "common.hpp"
#include <soa.h>

const int PPU_PATTERN_TABLE_0 = 0x0000;
const int PPU_PATTERN_TABLE_1 = 0x1000;

// prg bank index where most assets are stored
const u8 ASSETS_BANK = 1;
// prg bank for palettes
const u8 PALETTES_BANK = 2;

// various base tiles
static constexpr u8 MAZE_BASE_TILE = 0x20;
static constexpr u8 MARSHMALLOW_BASE_TILE = 0x40;
static constexpr u8 DIGITS_BASE_TILE = 0x64;
static constexpr u8 DARK_ZERO_TILE = 0x6e;
static constexpr u8 GAMEPLAY_CURSOR_TILE = 0x3d;
static constexpr u8 GAMEPLAY_CURSOR_ALT_TILE = 0x3e;
static constexpr u8 GAMEPLAY_CURSOR_CLEAR_TILE = 0x02;
static constexpr u8 SELECT_BUTTON_BASE_TILE = 0xb3;
static constexpr u8 HOW_TO_PLAYER_LABELS_BASE_TILE = 0xa9;
static constexpr u8 MOUNTAIN_MOUTH_BASE_TILE = 0x80;
static constexpr u8 PREVIEW_BASE_TILE = 0x70;

extern "C" const soa::Array<char *, NUM_STAGES> level_bg_palettes;
extern "C" const soa::Array<char *, NUM_STAGES> level_spr_palettes;
extern "C" const soa::Array<char *, NUM_STAGES> level_bg_tiles;
extern "C" const soa::Array<char *, NUM_STAGES> level_nametables;
extern "C" const u8 level_bg_tile_blocks[NUM_STAGES];

extern "C" const char title_bg_palette[];
extern "C" const char title_spr_palette[];
extern "C" const char title_bg_tiles[];
extern "C" const char title_nametable[];

// map
extern "C" const char map_nametable[];

// base tiles, other tiles are replacement suffixes of this one
// except for Fairy Forest, which is actually the same one
extern "C" const char base_bg_tiles[];

extern "C" const char spr_tiles[];

// replace story mode's goal with "level"
extern "C" const char level_label_tiles[];
// replace story mode's goal with "time"
extern "C" const char time_label_tiles[];

// replace story mode's goal with "level" on Starlit Stables stage
extern "C" const char starlit_level_label_tiles[];
// replace story mode's goal with "time" on Starlit Stables stage
extern "C" const char starlit_time_label_tiles[];

// uppercase and stuff
extern "C" const char spare_characters[];

// replaces story mode prompt with time trial one
extern "C" const char time_trial_prompt[3][20];
// replaces story mode prompt with endless mode one
extern "C" const char endless_prompt[3][20];