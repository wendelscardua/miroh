                                ; config.s
        .global __chr_rom_size
        .global __chr_ram_size
        .global __mirroring

                                ; Kilobytes
        __prg_rom_size = 64
        __chr_rom_size = 0
        __chr_ram_size = 32

                                ; Flags
        __mirroring = 1 ; horizontal mirroring
