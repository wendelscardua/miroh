#include "banked-asset-helpers.hpp"
#include "bank-helper.hpp"
#include "ggsound.hpp"
#include "soundtrack.hpp"
#include <bank.h>
#include <neslib.h>

void banked_oam_meta_spr(char x, char y, const void *data) {
  ScopedBank scopedBank(GET_BANK(metasprite_list));
  oam_meta_spr(x, y, data);
}

void banked_play_sfx(SFX sfx, GGSound::SFXPriority priority) {
  ScopedBank scopedBank(GET_BANK(sfx_list));
  GGSound::play_sfx(sfx, priority);
}