# frozen_string_literal: true

# NESST-compatible RLE compression
module RLE
  def self.rle(bytes)
    rle_tag = 0
    rle_tag += 1 while bytes.include?(rle_tag)

    compressed_bytes = [rle_tag]
    last_byte = nil
    counter = 0
    bytes.each do |byte|
      if byte != last_byte || counter == 255
        if counter > 1
          compressed_bytes += [rle_tag, counter]
          counter = 0
        elsif counter == 1
          compressed_bytes << last_byte
          counter = 0
        end
        compressed_bytes << byte
        last_byte = byte
      else
        counter += 1
      end
    end
    compressed_bytes += [rle_tag, counter] if counter.positive?
    compressed_bytes += [rle_tag, 0]

    compressed_bytes
  end
end
