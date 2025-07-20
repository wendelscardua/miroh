cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Os -flto")
set(CMAKE_CXX_FLAGS_DEBUG "-Os")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -Os -flto")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -flto")
# Assembler complains when -DNDEBUG is here
set(CMAKE_ASM_FLAGS_RELEASE "-Os -flto")
set(CMAKE_ASM_FLAGS_DEBUG "-Os")
set(CMAKE_ASM_FLAGS_MINSIZEREL "-Os -flto")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "-Os -flto")

set(LLVM_MOS_PLATFORM nes-unrom)
find_package(llvm-mos-sdk REQUIRED)

project(miroh-jr CXX ASM)

add_compile_options(
  -g -gdwarf-4
  -std=c++23
  -Wall -Wextra -Wconversion -Wsign-conversion
)

add_link_options(
  -g -gdwarf-4
  -mreserve-zp=85
  # donut: 18, ggsound: 58 ?, zx02: 7

  # make functions non-recursive by default
  -fnonreentrant

  # -Map generates a file with information about how the final binary's memory
  # is laid out. This will eventually be used to show the remaining space in
  # CHR-ROM, PRG-ROM, PRG-RAM, System RAM, and ZEROPAGE

  # --lto-whole-program-visibility helps ensure clang does as many
  # optimizations as possible
  -Wl,-Map,${CMAKE_PROJECT_NAME}.map,--lto-whole-program-visibility
)

set(ROM ${CMAKE_PROJECT_NAME}.nes)

include(Assets)

add_subdirectory(src)
add_subdirectory(ca65src)

add_executable(${ROM})

target_link_libraries(${ROM} PRIVATE
  # Second-party, included in llvm-mos
  neslib
  nesdoug
  # subdirectory stuff
  SourceObj
  Ca65Obj
)
