#pragma once

#include "common.hpp"

namespace Attributes {
  // Sets the attribute for a single metatile (at meta_x, meta_y)
  // on the shadow table; note that attribute value must be 4 identical crumbles
  // (e.g. an attribute 2 must be passed as 0b10101010)
  void set(u8 meta_x, u8 meta_y, u8 attribute);

  // Sends all attributes to VRAM (rendering must be off)
  void update_vram();

  // Next writes, until the next flush, will use vram buffer
  void enable_vram_buffer();

  // Send any pending writes to vram buffer, then disables it
  void flush_vram_update();

  // Resets shadow copy of attributes, reading them from PPU
  // (rendering must be off)
  void reset_shadow();
} // namespace Attributes
