# frozen_string_literal: true

module NEXXT
  # Structured representation of a NEXXT session file
  class Session
    attr_reader :text, :flat_table, :table, :chr_main, :chr_copy, :metasprites_offset, :metasprites

    def initialize(text)
      @text = text
      lines = text.lines(chomp: true).compact
      @flat_table = lines.select { |line| line =~ /=/ }.map { |line| line.split(/=/, 2) }.to_h
      @table = Session.parse_table(@flat_table)
      @chr_main = Session.decode_hex(@table.dig('CHR', 'Main'))
      @chr_copy = Session.decode_hex(@table.dig('CHR', 'Copy'))
      @chr_undo = Session.decode_hex(@table.dig('CHR', 'Undo'))
      @metasprites_offset = @table.dig('Var', 'Sprite', 'Grid').then do |grid|
        {
          x: grid['X'].to_i,
          y: grid['Y'].to_i
        }
      end
      @metasprites = Session.make_metasprites(
        names: Session.metasprite_names(@table.dig('Meta', 'Sprite')),
        bytes: Session.decode_hex(@table.dig('Meta', 'Sprites')), # : Array[Integer]
        offset: @metasprites_offset
      )
    end

    def self.read(file)
      new(
        File.read(file)
      )
    end

    def self.parse_table(flat_table)
      table = {}
      flat_table.each do |key, value|
        path = decompose_key(key)
        path.reduce(table) do |a, e|
          a[e] = { '_root' => a[e] } unless a[e].nil? || a[e].is_a?(Hash)
          a[e] ||= {}
        end
        if path.size > 1
          table.dig(
            *path[..-2] # : Array[String]
          )[path[-1]] = value
        else
          table[path.first] = value
        end
      end
      table
    end

    def self.decompose_key(key)
      key.split(/(?<=[^A-Z0-9])(?=[A-Z0-9])|(?<=CHR)/).map { |part| part.gsub(/_/, '') }
    end

    # decompress NEXXT's rle'd hexadecimal data
    def self.decode_hex(string)
      return if string.nil? || string.empty?

      values = []
      until string.empty?
        match = string.match(/\A(?:\[(?<rle>[0-9a-f]+)\]|(?<literal>[0-9a-f]{2}))(?<rest>.*)/)
        raise "Invalid string #{string}" unless match

        if (rle = match['rle'])
          values += [(values[-1] || 0)] * (rle.to_i(16) - 1)
        elsif (literal = match['literal'])
          values << literal.to_i(16)
        end
        string = match['rest']
      end
      values
    end

    def self.metasprite_names(table)
      table.select { |key, value| value.is_a?(String) && key =~ /\d/ }
           .map { |key, value| [value, key.to_i] }
           .to_h
    end

    def self.make_metasprites(names:, bytes:, offset:)
      sprites = bytes.each_slice(256)
                     .to_a
                     .map do |meta_bytes|
        meta_bytes.each_slice(4)
                  .reject { |row| row[0] == 255 && row[2] == 255 && row[3] == 255 } # tile can be different from $ff
                  .map do |y, tile, attribute, x|
          raise "Invalid bytes #{bytes}" if y.nil? || tile.nil? || attribute.nil? || x.nil?

          Sprite.new(y: y - offset[:y], tile:, attribute:, x: x - offset[:x])
        end
      end
      names.map do |name, index|
        Metasprite.new(name:, sprites: sprites[index])
      end
    end
  end

  Metasprite = Data.define(:name, :sprites)

  Sprite = Data.define(:x, :y, :tile, :attribute)

  # Structured representation of a NEXXT map file
  class MapFile
    attr_reader :raw_bytes,
                :width,
                :height,
                :tiles,
                :attributes,
                :metatiles

    def initialize(raw_bytes)
      @raw_bytes = raw_bytes.dup
      @width, @height = MapFile.extract_dimensions(@raw_bytes)
      @tiles = MapFile.organize_in_tiles(@raw_bytes, @width, @height)
      @attributes = @raw_bytes[(@width * @height)..]

      @metatiles = MapFile.organize_in_metatiles(@tiles, @attributes, @width, @height)
    end

    def self.read(file)
      new(
        File.read(file).unpack('C*') # : Array[Integer]
      )
    end

    def self.extract_dimensions(bytes)
      if bytes.size == 1024
        # this is a .nam instead of a .map
        [32, 30]
      else
        width_l, width_h, height_l, height_h = bytes.pop(4)
        raise 'Invalid sizes' if width_l.nil? ||
                                 width_h.nil? ||
                                 height_l.nil? ||
                                 height_h.nil?

        [width_h * 256 + width_l, height_h * 256 + height_l]
      end
    end

    def self.organize_in_tiles(bytes, width, height)
      Array.new(height) do |row|
        Array.new(width) do |column|
          bytes[row * width + column]
        end
      end
    end

    def self.extract_attribute(attributes, width, meta_row, meta_column)
      attribute = attributes[(meta_row / 2) * (width / 4) + (meta_column / 2)]
      attribute >>= 4 if meta_row.odd?
      attribute >>= 2 if meta_column.odd?
      attribute & 0b11
    end

    def self.extract_metatile_tiles(tiles, meta_row, meta_column)
      upper = meta_row * 2
      left = meta_column * 2
      lower = upper + 1
      right = left + 1
      [
        tiles[upper][left],
        tiles[upper][right],
        tiles[lower][left],
        tiles[lower][right]
      ]
    end

    def self.organize_in_metatiles(tiles, attributes, width, height)
      Array.new(height / 2) do |meta_row|
        Array.new(width / 2) do |meta_column|
          meta_tiles = MapFile.extract_metatile_tiles(tiles, meta_row, meta_column)
          Metatile.new(
            meta_tiles[0], meta_tiles[1], meta_tiles[2], meta_tiles[3],
            MapFile.extract_attribute(attributes, width, meta_row, meta_column)
          )
        end
      end.flatten
    end

    # Represents a single metatile (4 tiles and an attribute)
    class Metatile
      attr_reader :upper_left, :upper_right, :lower_left, :lower_right, :attribute

      def initialize(upper_left, upper_right, lower_left, lower_right, attribute)
        @upper_left = upper_left
        @upper_right = upper_right
        @lower_left = lower_left
        @lower_right = lower_right
        @attribute = attribute
      end

      def to_a
        [
          upper_left,
          upper_right,
          lower_left,
          lower_right,
          attribute
        ]
      end
    end
  end
end
