#pragma once

#define PPU_PATTERN_TABLE_0 0x0000
#define PPU_PATTERN_TABLE_1 0x1000
#define PPU_PATTERN_TABLE_SIZE 4096
#define PPU_PATTERN_ROW_SIZE (4096/16)

extern const char bg_chr[];
extern const char sprites_chr[];
