#include "cheats.hpp"
#include "charset.hpp"
#include "common.hpp"
#include "ggsound.hpp"
#include "soundtrack.hpp"
#include <nesdoug.h>
#include <neslib.h>
#include <string.h>

__attribute__((used)) const SFX cheat_code_sfx[] = {
    SFX::Lineclear1, SFX::Lineclear2, SFX::Lineclear3, SFX::Lineclear4};

Cheats::Cheats()
    : cheat_code{0, 0, 0, 0}, cheat_code_index(0), higher_level(false),
      infinite_energy(false) {}

void Cheats::push_code(u8 code) {
  GGSound::play_sfx(cheat_code_sfx[cheat_code_index],
                    GGSound::SFXPriority::One);

  cheat_code[cheat_code_index] = code;
  cheat_code_index++;
  if (cheat_code_index == 4) {
    cheat_code_index = 0;
    // Compare the 4-byte cheat_code array to the string literals directly
    if (memcmp(cheat_code, "high"_ts, 4) == 0) {
      higher_level = true;
      multi_vram_buffer_horz("high"_ts, 4, NTADR_D(4, 27));
    } else if (memcmp(cheat_code, "cafe"_ts, 4) == 0) {
      infinite_energy = true;
      multi_vram_buffer_horz("cafe"_ts, 4, NTADR_D(10, 27));
    } else if (memcmp(cheat_code, "fate"_ts, 4) == 0) {
      story_completion[(u8)Stage::StarlitStables] = true;
      story_completion[(u8)Stage::RainbowRetreat] = true;
      story_completion[(u8)Stage::FairyForest] = true;
      story_completion[(u8)Stage::GlitteryGrotto] = true;
      multi_vram_buffer_horz("fate"_ts, 4, NTADR_D(16, 27));
    }
  }
}

void Cheats::reset() {
  cheat_code_index = 0;
  higher_level = false;
  infinite_energy = false;
}