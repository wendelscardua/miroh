
#include "donut.hpp"

extern "C" void _asm_donut_decompress_to_ppu(void *stream_ptr, char num_blocks);

namespace Donut {
  void decompress_to_ppu(void *stream_ptr, char num_blocks) {
    _asm_donut_decompress_to_ppu(stream_ptr, num_blocks);
  }
} // namespace Donut
