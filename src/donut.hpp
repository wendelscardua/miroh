#pragma once

extern "C" char donut_block_buffer[64];

namespace Donut {
  // Decompress num_blocks * 64 bytes from stream_ptr to the PPU.
  // Remember to turn off rendering before using.
  void decompress_to_ppu(void *stream_ptr, char num_blocks);
} // namespace Donut
