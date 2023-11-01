#include "banked-asset-helpers.hpp"
#include "bank-helper.hpp"
#include "ggsound.hpp"
#include "soundtrack.hpp"
#include <mapper.h>
#include <neslib.h>

struct Sprite {
  s8 delta_x;
  s8 delta_y;
  u8 tile;
  u8 attribute;
};

void banked_play_song(Song song) {
  ScopedBank scoopedBank(GET_BANK(song_list));
  GGSound::play_song(song);
}

void banked_play_sfx(SFX sfx, GGSound::SFXPriority priority) {
  ScopedBank scopedBank(GET_BANK(sfx_list));
  GGSound::play_sfx(sfx, priority);
}