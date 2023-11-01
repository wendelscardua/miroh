#include "attributes.hpp"
#include <nesdoug.h>
#include <neslib.h>

namespace Attributes {
  u8 shadow[64];
  u8 dirty_index;
  bool buffered = false;

  void send_dirty_to_vram_buffer() {
    if (dirty_index == 0xff)
      return;

    one_vram_buffer(shadow[dirty_index], NAMETABLE_A + 0x3c0 + dirty_index);
    dirty_index = 0xff;
  }

  void flush_vram_update() {
    send_dirty_to_vram_buffer();
    buffered = false;
  }

  void set(unsigned char meta_x, unsigned char meta_y,
           unsigned char attribute) {
    unsigned char attribute_x = meta_x >> 1;
    unsigned char attribute_y = meta_y >> 1;

    unsigned char attribute_index = attribute_y * 8 + attribute_x;

    unsigned char mask;

    if (meta_y & 0b1) {
      if (meta_x & 0b1) {
        mask = 0b11000000;
      } else {
        mask = 0b110000;
      }
    } else if (meta_x & 0b1) {
      mask = 0b1100;
    } else {
      mask = 0b11;
    }

    shadow[attribute_index] =
        (shadow[attribute_index] & (~mask)) | (attribute & mask);

    if (buffered && attribute_index != dirty_index) {
      send_dirty_to_vram_buffer();
      dirty_index = attribute_index;
    }
  }

  void update_vram() {
    vram_adr(NAMETABLE_A + 0x3c0);
    vram_write(shadow, 64);
  }

  void enable_vram_buffer() {
    buffered = true;
    dirty_index = 0xff;
  }

  void reset_shadow() {
    vram_adr(NAMETABLE_A + 0x3c0);
    vram_read(shadow, 64);
  }
} // namespace Attributes
