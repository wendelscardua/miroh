#pragma once

#include "ggsound.hpp"

// draws metasprite from the metasprite bank w/ vertical scroll culling
extern "C" void banked_oam_meta_spr(char x, int y, const void *data);

// draws metasprite from the metasprite bank w/ horizontal scroll culling
extern "C" void banked_oam_meta_spr_horizontal(int x, char y, const void *data);

// plays song from the song bank
void banked_play_song(Song song);

// plays sfx from the sfx bank
void banked_play_sfx(SFX sfx, GGSound::SFXPriority priority);

// [assets bank] loads title assets
void load_title_assets();

void load_gameplay_assets();