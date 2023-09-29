#pragma once

#include "common.hpp"

namespace Attributes {
  // Sets the attribute for a single metatile (at meta_x, meta_y)
  // on the shadow table
  void set(u8 meta_x, u8 meta_y, u8 attribute);

  // Sends all attributes to VRAM (rendering must be off)
  void update_vram();

  // Next writes, until the next flush, will use vram buffer
  void enable_vram_buffer();

  // Send any pending writes to vram buffer, then disables it
  void flush_vram_update();
} // namespace Attributes
