#include "banked-asset-helpers.hpp"
#include "bank-helper.hpp"
#include "ggsound.hpp"
#include "soundtrack.hpp"
#include <bank.h>
#include <neslib.h>

struct Sprite {
  s8 delta_x;
  s8 delta_y;
  u8 tile;
  u8 attribute;
};

void banked_oam_meta_spr(char x, int y, const void *data) {
  ScopedBank scopedBank(GET_BANK(metasprite_list));

  for (auto sprites = (const Sprite *)data; sprites->delta_x != -128;
       sprites++) {
    int spr_y = y + sprites->delta_y;
    if (spr_y < 0 || spr_y > 0xef) {
      continue;
    }
    u8 spr_x = (u8)(x + sprites->delta_x);
    if (spr_x < 0 || spr_x > 0xff) {
      continue;
    }
    oam_spr(spr_x, (u8)spr_y, sprites->tile, sprites->attribute);
  }
}

void banked_play_song(Song song) {
  ScopedBank scoopedBank(GET_BANK(song_list));
  GGSound::play_song(song);
}

void banked_play_sfx(SFX sfx, GGSound::SFXPriority priority) {
  ScopedBank scopedBank(GET_BANK(sfx_list));
  GGSound::play_sfx(sfx, priority);
}