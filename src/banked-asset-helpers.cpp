#include "banked-asset-helpers.hpp"
#include "assets.hpp"
#include "bank-helper.hpp"
#include "common.hpp"
#include "donut.hpp"
#include "zx02.hpp"
#include <mapper.h>
#include <neslib.h>

#pragma clang section text = ".prg_rom_fixed.text.bah"
#pragma clang section rodata = ".prg_rom_fixed.rodata.bah"

void load_title_palette() {
  ScopedBank scopedBank(PALETTES_BANK);
  pal_bg(title_bg_palette);
  pal_spr(title_spr_palette);
}

void load_title_assets() {
  ScopedBank scopedBank(ASSETS_BANK);
  vram_adr(PPU_PATTERN_TABLE_0);
  Donut::decompress_to_ppu((void *)base_bg_tiles, 4096 / 64 - 56);
  Donut::decompress_to_ppu((void *)title_bg_tiles, 56);

  vram_adr(PPU_PATTERN_TABLE_1);
  Donut::decompress_to_ppu((void *)spr_tiles, 4096 / 64);

  vram_adr(NAMETABLE_D);
  zx02_decompress_to_vram((void *)title_nametable, NAMETABLE_D);
  load_title_palette();
}

void load_map_assets() {
  ScopedBank scopedBank(ASSETS_BANK);
  vram_adr(PPU_PATTERN_TABLE_0);
  Donut::decompress_to_ppu((void *)base_bg_tiles, 4096 / 64);
  vram_adr(PPU_PATTERN_TABLE_0 + 0xf0 * 0x10);
  Donut::decompress_to_ppu((void *)spare_characters, 3);

  vram_adr(NAMETABLE_A);
  zx02_decompress_to_vram((void *)map_nametable, NAMETABLE_A);

  load_title_palette();
}

void load_stage_palette() {
  ScopedBank scopedBank(PALETTES_BANK);
  pal_bg(level_bg_palettes[(u8)current_stage]);
  pal_spr(level_spr_palettes[(u8)current_stage]);
}

void load_gameplay_assets() {
  ScopedBank scopedBank(ASSETS_BANK);
  vram_adr(PPU_PATTERN_TABLE_0);
  u8 bg_blocks = level_bg_tile_blocks[(u8)current_stage];
  Donut::decompress_to_ppu((void *)base_bg_tiles, 4096 / 64 - bg_blocks);
  if (bg_blocks > 0) {
    Donut::decompress_to_ppu(level_bg_tiles[(u8)current_stage], bg_blocks);
  }

  vram_adr(NAMETABLE_B);
  zx02_decompress_to_vram(level_nametables[(u8)current_stage], NAMETABLE_B);

  // mode labels start at tile $84, both require 1 donut block (64 bytes)
  if (current_game_mode == GameMode::TimeTrial) {
    vram_adr(PPU_PATTERN_TABLE_0 + 0x84 * 0x10);
    Donut::decompress_to_ppu((void *)(current_stage == Stage::StarlitStables
                                          ? starlit_time_label_tiles
                                          : time_label_tiles),
                             1);
    vram_adr(NTADR_C(6, 21));
    vram_write(time_trial_prompt[0], 20);
    vram_adr(NTADR_C(6, 23));
    vram_write(time_trial_prompt[1], 20);
    vram_adr(NTADR_C(6, 25));
    vram_write(time_trial_prompt[2], 20);
  } else if (current_game_mode == GameMode::Endless) {
    vram_adr(PPU_PATTERN_TABLE_0 + 0x84 * 0x10);
    Donut::decompress_to_ppu((void *)(current_stage == Stage::StarlitStables
                                          ? starlit_level_label_tiles
                                          : level_label_tiles),
                             1);
    vram_adr(NTADR_C(6, 21));
    vram_write(endless_prompt[0], 20);
    vram_adr(NTADR_C(6, 23));
    vram_write(endless_prompt[1], 20);
    vram_adr(NTADR_C(6, 25));
    vram_write(endless_prompt[2], 20);
  }

  load_stage_palette();
}

void change_uni_palette() {
  ScopedBank scopedBank(PALETTES_BANK);
  for (u8 i = 0; i < 16; i++) {
    pal_col(0x10 | i, level_spr_palettes[(u8)current_stage][i]);
  }
}
