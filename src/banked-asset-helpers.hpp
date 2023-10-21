#pragma once

// draws metasprite from the metasprite bank
#include "ggsound.hpp"
void banked_oam_meta_spr(char x, char y, const void *data);

// plays song from the song bank
void banked_play_song(Song song);

// plays sfx from the sfx bank
void banked_play_sfx(SFX sfx, GGSound::SFXPriority priority);