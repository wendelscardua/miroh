#pragma once

#define RAND_UP_TO(n) ((u8)(((u16)rand8() * (u16)(n)) >> 8))
#define RAND_UP_TO_POW2(n) (rand8() & ((1 << n) - 1))