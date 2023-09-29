#pragma once

#define GET_BANK(symbol) [] (){ \
        register u8 bank asm("a"); \
        asm ( \
            "ld%0 #mos24bank(" # symbol ")\n"  \
            : "=r" (bank) \
            : "r" (bank) \
            : "a" \
            ); \
        return bank; \
    }()
