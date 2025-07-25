#pragma once

#include "ggsound.hpp"

// draws metasprite from the metasprite bank w/ vertical scroll culling
extern "C" void banked_oam_meta_spr(char bank, char x, int y, const void *data);

// draws metasprite from the metasprite bank w/ horizontal scroll culling
extern "C" void banked_oam_meta_spr_horizontal(int x, char y, const void *data);

// plays song from the song bank
__attribute((noinline)) void banked_play_song(Song song);

// plays sfx from the sfx bank
__attribute((noinline)) void banked_play_sfx(SFX sfx,
                                             GGSound::SFXPriority priority);

// loads title assets
__attribute((noinline)) void load_title_assets();

// loads map assets
__attribute((noinline)) void load_map_assets();

// loads game assets
__attribute((noinline)) void load_gameplay_assets();

// loads palette for uni map
__attribute((noinline)) void change_uni_palette();