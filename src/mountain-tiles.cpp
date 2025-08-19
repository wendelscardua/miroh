#include "mountain-tiles.hpp"

#include "assets.hpp"

namespace MountainTiles {
  const u8 CLOSED_MOUTH[] = {MOUNTAIN_MOUTH_BASE_TILE + 0,
                             MOUNTAIN_MOUTH_BASE_TILE + 1};
  const u8 OPEN_MOUTH[] = {MOUNTAIN_MOUTH_BASE_TILE + 2,
                           MOUNTAIN_MOUTH_BASE_TILE + 3};

  const u8 EMPTY_PREVIEW[] = {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE};

  const u8 STREAM[][4] = {{PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                          {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE},
                          {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE,
                           PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 2,
                           PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                          {PREVIEW_BASE_TILE, PREVIEW_BASE_TILE + 8,
                           PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8},
                          {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                           PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2},
                          {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8,
                           PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                           PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 8,
                           PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 2,
                           PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 8, PREVIEW_BASE_TILE + 10,
                           PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 2, PREVIEW_BASE_TILE + 10,
                           PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10},
                          {PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10,
                           PREVIEW_BASE_TILE + 10, PREVIEW_BASE_TILE + 10}};
} // namespace MountainTiles