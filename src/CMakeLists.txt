find_program(
  GENERATE_METASPRITES
  generate-metasprites
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT GENERATE_METASPRITES)
  message(FATAL_ERROR "The generate-metasprites tool is required!")
endif()

find_package(Python3 REQUIRED)

find_file(
  ft_to_asm
  NAMES ft_txt_to_asm.py
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT ft_to_asm)
  message(FATAL_ERROR "Cannot convert soundtrack: Unable to find conversor script ft_txt_to_asm.py")
endif()

find_program(
  SOUNDTRACK_ENUMS
  soundtrack-enums
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT SOUNDTRACK_ENUMS)
  message(FATAL_ERROR "The soundtrack-enums tool is required!")
endif()

find_program(
  POLYOMINO
  polyomino
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT POLYOMINO)
  message(FATAL_ERROR "The polyomino tool is required!")
endif()

find_program(
  MAZER
  mazer
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT MAZER)
  message(FATAL_ERROR "The mazer tool is required!")
endif()

add_custom_command(
  OUTPUT metasprites.s ${CMAKE_CURRENT_BINARY_DIR}/metasprites.hpp
  COMMAND ${GENERATE_METASPRITES} generate ${CMAKE_CURRENT_BINARY_DIR}/metasprites.s ${CMAKE_CURRENT_BINARY_DIR}/metasprites.hpp ${CMAKE_SOURCE_DIR}/assets/metasprites.nss --bank .prg_rom_1 --label-prefix metasprite
  DEPENDS ${GENERATE_METASPRITES} ${CMAKE_SOURCE_DIR}/assets/metasprites.nss
)

add_custom_command(
  OUTPUT soundtrack.asm
  COMMAND ${Python3_EXECUTABLE} ${ft_to_asm} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm
  DEPENDS ${ft_to_asm} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.hpp
  COMMAND ${SOUNDTRACK_ENUMS} generate ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.hpp ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
  DEPENDS ${SOUNDTRACK_ENUMS} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
)

add_custom_command(
  OUTPUT polyominos.s
  COMMAND ${POLYOMINO} generate ${CMAKE_CURRENT_BINARY_DIR}/polyominos.s ${CMAKE_SOURCE_DIR}/assets/polyominos.json --max-size 4 --bank .prg_rom_0
  DEPENDS ${POLYOMINO} ${CMAKE_SOURCE_DIR}/assets/polyominos.json
)

add_custom_command(
  OUTPUT maze-defs.s
  COMMAND ${MAZER} generate ${CMAKE_CURRENT_BINARY_DIR}/maze-defs.s ${CMAKE_SOURCE_DIR}/assets/mazes --bank .prg_rom_0
  DEPENDS ${MAZER} ${CMAKE_SOURCE_DIR}/assets/mazes
)

add_library(SourceObj
  OBJECT

  config.s

  main.cpp
  title-screen.cpp
  gameplay.cpp

  attributes.cpp
  banked-asset-helpers.s
  banked-asset-helpers.cpp
  board.cpp
  donut.s
  fruits.cpp
  ggsound.cpp
  ggsound.s
  log.cpp
  player.cpp
  polyomino.cpp
  polyomino-defs.cpp

  assets.s
  soundtrack.s
  polyominos.s
  maze-defs.s

  soundtrack.hpp

  ${CMAKE_CURRENT_BINARY_DIR}/metasprites.s
)

set_property(
  SOURCE
  assets.s
  PROPERTY
  OBJECT_DEPENDS
  how-to.nam.rle
  credits.nam.rle

  SPR.chr.donut

  TitleScreenBG.pal
  TitleBG.chr.donut
  Title.nam.rle
  
  StarlitStablesBG.pal
  StarlitStablesSPR.pal
  StarlitStablesBG.chr.donut
  StarlitStables.nam.rle
  StarlitStablesAlt.nam.rle

  RainbowRetreatBG.pal
  RainbowRetreatSPR.pal
  RainbowRetreatBG.chr.donut
  RainbowRetreat.nam.rle
  RainbowRetreatAlt.nam.rle

  FairyForestBG.pal
  FairyForestSPR.pal
  FairyForestBG.chr.donut
  FairyForest.nam.rle
  FairyForestAlt.nam.rle

  GlitteryGrottoBG.pal
  GlitteryGrottoSPR.pal
  GlitteryGrottoBG.chr.donut
  GlitteryGrotto.nam.rle
  GlitteryGrottoAlt.nam.rle
  
  MarshmallowMountainSPR.pal
)

add_rle_asset(SOURCE "how-to.nam")
add_rle_asset(SOURCE "credits.nam")

add_donut_asset(SOURCE "SPR.chr")

add_raw_asset(SOURCE "TitleScreenBG.pal")
add_rle_asset(SOURCE "Title.nam")
add_donut_asset(SOURCE "TitleBG.chr")

add_raw_asset(SOURCE "StarlitStablesBG.pal")
add_raw_asset(SOURCE "StarlitStablesSPR.pal")
add_donut_asset(SOURCE "StarlitStablesBG.chr")
add_rle_asset(SOURCE "StarlitStables.nam")
add_rle_asset(SOURCE "StarlitStablesAlt.nam")

add_raw_asset(SOURCE "RainbowRetreatBG.pal")
add_raw_asset(SOURCE "RainbowRetreatSPR.pal")
add_donut_asset(SOURCE "RainbowRetreatBG.chr")
add_rle_asset(SOURCE "RainbowRetreat.nam")
add_rle_asset(SOURCE "RainbowRetreatAlt.nam")

add_raw_asset(SOURCE "FairyForestBG.pal")
add_raw_asset(SOURCE "FairyForestSPR.pal")
add_donut_asset(SOURCE "FairyForestBG.chr")
add_rle_asset(SOURCE "FairyForest.nam")
add_rle_asset(SOURCE "FairyForestAlt.nam")

add_raw_asset(SOURCE "GlitteryGrottoBG.pal")
add_raw_asset(SOURCE "GlitteryGrottoSPR.pal")
add_donut_asset(SOURCE "GlitteryGrottoBG.chr")
add_rle_asset(SOURCE "GlitteryGrotto.nam")
add_rle_asset(SOURCE "GlitteryGrottoAlt.nam")

add_raw_asset(SOURCE "MarshmallowMountainSPR.pal")

set_property(
  SOURCE
  soundtrack.s
  PROPERTY
  OBJECT_DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm
  ${CMAKE_CURRENT_SOURCE_DIR}/ggsound.inc
)

set_property(
  SOURCE
  ggsound.s
  PROPERTY
  OBJECT_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/ggsound.inc
)

set_target_properties(SourceObj PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(
  SourceObj
  PRIVATE
  ${CMAKE_BINARY_DIR}/assets
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
)
