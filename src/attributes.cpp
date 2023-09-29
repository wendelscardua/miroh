#include "attributes.hpp"
#include <nesdoug.h>
#include <neslib.h>

namespace Attributes {
  u8 shadow[128];
  u8 dirty_index;
  bool buffered = false;

  void send_dirty_to_vram_buffer() {
    if (dirty_index == 0xff)
      return;

    if (dirty_index < 64) {
      one_vram_buffer(shadow[dirty_index], NAMETABLE_A + 0x3c0 + dirty_index);
    } else {
      one_vram_buffer(shadow[dirty_index],
                      NAMETABLE_D + 0x3c0 + dirty_index - 64);
    }
  }

  void flush_vram_update() {
    send_dirty_to_vram_buffer();
    buffered = false;
  }

  void set(unsigned char meta_x, unsigned char meta_y,
           unsigned char attribute) {
    unsigned char attribute_x = (meta_x & 0x0f) >> 1;
    unsigned char attribute_y = meta_y >> 1;

    unsigned char attribute_index = attribute_y * 8 + attribute_x;
    if (meta_x & 0x10) {
      attribute_index += 64;
    }

    unsigned char mask = 0b11;
    unsigned char value = attribute;

    if (meta_y & 0b1) {
      value <<= 4;
      mask <<= 4;
    }
    if (meta_x & 0b1) {
      value <<= 2;
      mask <<= 2;
    }

    shadow[attribute_index] = (shadow[attribute_index] & (~mask)) | value;

    if (buffered && attribute_index != dirty_index) {
      send_dirty_to_vram_buffer();
      dirty_index = attribute_index;
    }
  }

  void update_vram() {
    vram_adr(NAMETABLE_A + 0x3c0);
    vram_write(shadow, 64);
    vram_adr(NAMETABLE_D + 0x3c0);
    vram_write(shadow + 64, 64);
  }

  void enable_vram_buffer() {
    buffered = true;
    dirty_index = 0xff;
  }
} // namespace Attributes
