#include "cheats.hpp"
#include "ggsound.hpp"
#include "soundtrack.hpp"

__attribute__((used)) const SFX egg_sfx[] = {SFX::Lineclear1, SFX::Lineclear2,
                                             SFX::Lineclear3, SFX::Lineclear4};

Cheats::Cheats()
    : cheat_code{0, 0, 0, 0}, cheat_code_index(0), higher_score(false),
      infinite_energy(false) {}

void Cheats::push_code(u8 code) {
  GGSound::play_sfx(egg_sfx[cheat_code_index], GGSound::SFXPriority::One);

  cheat_code[cheat_code_index] = code;
  cheat_code_index++;
  if (cheat_code_index == 4) {
    cheat_code_index = 0;
  }
}

void Cheats::reset() {
  cheat_code_index = 0;
  higher_score = false;
  infinite_energy = false;
}