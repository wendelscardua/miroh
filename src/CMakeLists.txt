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

add_custom_command(
  OUTPUT metasprites.s ${CMAKE_CURRENT_BINARY_DIR}/metasprites.hpp
  COMMAND ${GENERATE_METASPRITES} generate ${CMAKE_CURRENT_BINARY_DIR}/metasprites.s ${CMAKE_CURRENT_BINARY_DIR}/metasprites.hpp ${CMAKE_SOURCE_DIR}/assets/metasprites.nss --bank .prg_rom_last --label-prefix metasprite
  DEPENDS ${CMAKE_SOURCE_DIR}/assets/metasprites.nss
)

add_custom_command(
  OUTPUT soundtrack.asm
  COMMAND ${Python3_EXECUTABLE} ${ft_to_asm} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm
  DEPENDS ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.hpp
  COMMAND ${SOUNDTRACK_ENUMS} generate ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.hpp ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
  DEPENDS ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
)

add_custom_command(
  OUTPUT polyominos.s
  COMMAND ${POLYOMINO} generate ${CMAKE_CURRENT_BINARY_DIR}/polyominos.s ${CMAKE_SOURCE_DIR}/assets/polyominos.json --max-size 3 --bank .prg_rom_1
  DEPENDS ${POLYOMINO} ${CMAKE_SOURCE_DIR}/assets/polyominos.json
)

add_library(SourceObj
  OBJECT

  config.s

  main.cpp
  title-screen.cpp
  gameplay.cpp

  attributes.cpp
  board.cpp
  donut.cpp
  donut.s
  fruits.cpp
  ggsound.cpp
  ggsound.s
  log.cpp
  player.cpp
  polyomino.cpp

  chr-data.s
  nametables.s
  palettes.s
  soundtrack.s
  polyominos.s

  soundtrack.hpp

  ${CMAKE_CURRENT_BINARY_DIR}/metasprites.s
)

add_donut_asset(SOURCE "bg.chr")
add_donut_asset(SOURCE "sprites.chr")

set_property(
  SOURCE
  chr-data.s
  PROPERTY
  OBJECT_DEPENDS
  bg.chr.donut
  sprites.chr.donut
)

add_raw_asset(SOURCE "bg.pal")
add_raw_asset(SOURCE "sprites-player.pal")
add_raw_asset(SOURCE "sprites-polyomino.pal")

set_property(
  SOURCE
  palettes.s
  PROPERTY
  OBJECT_DEPENDS
  bg.pal
  sprites-player.pal
  sprites-polyomino.pal
)

add_raw_asset(SOURCE "title.nam")
add_raw_asset(SOURCE "gameplay.nam")
add_raw_asset(SOURCE "game-over.nam")

set_property(
  SOURCE
  nametables.s
  PROPERTY
  OBJECT_DEPENDS
  title.nam
  gameplay.nam
  game-over.nam
)

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
