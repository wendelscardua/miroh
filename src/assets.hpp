#pragma once

#include "common.hpp"
#include <soa.h>

const int PPU_PATTERN_TABLE_0 = 0x0000;
const int PPU_PATTERN_TABLE_1 = 0x1000;

// prg bank index where all assets are stored
const u8 ASSETS_BANK = 1;

// various base tiles
static const u8 MAZE_BASE_TILE = 0x20;
static const u8 MARSHMALLOW_BASE_TILE = 0x40;
static const u8 DIGITS_BASE_TILE = 0x64;
static const u8 DARK_ZERO_TILE = 0x6e;
static const u8 GAMEPLAY_CURSOR_TILE = 0x3d;
static const u8 GAMEPLAY_CURSOR_ALT_TILE = 0x3e;
static const u8 GAMEPLAY_CURSOR_CLEAR_TILE = 0x02;
static const u8 SELECT_BUTTON_BASE_TILE = 0xb3;
static const u8 HOW_TO_PLAYER_LABELS_BASE_TILE = 0xa9;

extern "C" const soa::Array<char *, NUM_STAGES> level_bg_palettes;
extern "C" const soa::Array<char *, NUM_STAGES> level_spr_palettes;
extern "C" const soa::Array<char *, NUM_STAGES> level_bg_tiles;
extern "C" const soa::Array<char *, NUM_STAGES> level_nametables;
extern "C" const soa::Array<char *, NUM_STAGES> level_alt_nametables;
extern "C" const u8 level_bg_tile_blocks[NUM_STAGES];

extern "C" const char title_bg_palette[];
extern "C" const char title_spr_palette[];
extern "C" const char title_bg_tiles[];
extern "C" const char title_nametable[];
extern "C" const char title_alt_nametable[];

// base tiles, other tiles are replacement suffixes of this one
// except for Fairy Forest, which is actually the same one
extern "C" const char base_bg_tiles[];

extern "C" const char spr_tiles[];
