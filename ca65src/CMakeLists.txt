find_package(Python3 REQUIRED)

# Check for ca65 (part of cc65 toolchain)
find_program(CA65_EXECUTABLE ca65)
if(NOT CA65_EXECUTABLE)
    message(FATAL_ERROR "ca65 is required but not found. Please install cc65 toolchain.")
endif()

find_file(
  ft_to_asm
  NAMES ft_txt_to_asm.py
  PATHS "${CMAKE_SOURCE_DIR}/tools"
)

if (NOT ft_to_asm)
  message(FATAL_ERROR "Cannot convert soundtrack: Unable to find conversor script ft_txt_to_asm.py")
endif()

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm # ${CMAKE_CURRENT_BINARY_DIR}/soundtrack_dpcm.asm
  COMMAND ${Python3_EXECUTABLE} ${ft_to_asm} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm
  DEPENDS ${ft_to_asm} ${CMAKE_SOURCE_DIR}/music/soundtrack.txt
)

add_library(Ca65Obj
  OBJECT

  ggsound.o
  soundtrack.o
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ggsound.o
    COMMAND ca65 --include-dir ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/ggsound.o ${CMAKE_CURRENT_SOURCE_DIR}/ggsound.asm
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ggsound.asm
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ggsound.inc
    VERBATIM)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.o
    COMMAND ca65 --include-dir ${CMAKE_CURRENT_BINARY_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.o ${CMAKE_CURRENT_SOURCE_DIR}/soundtrack.s
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/soundtrack.s
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/soundtrack.asm
    # DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/soundtrack_dpcm.asm
    VERBATIM)

set_target_properties(Ca65Obj PROPERTIES LINKER_LANGUAGE CXX)

