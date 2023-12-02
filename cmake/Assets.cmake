function(add_raw_asset)
  set(options)
  set(oneValueArgs SOURCE TARGET)
  set(multiValueArgs)
  cmake_parse_arguments(ASSET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ASSET_SOURCE)
    message(FATAL_ERROR "Raw asset FILE is required!")
  endif()

  set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/${ASSET_SOURCE}")

  if (ASSET_TARGET)
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_TARGET}")
  else()
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_SOURCE}")
  endif()

  get_filename_component(BASE_NAME ${ASSET_SRC} NAME)

  add_custom_command(
    OUTPUT ${ASSET_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy ${ASSET_SRC} ${ASSET_DEST}
    DEPENDS ${ASSET_SRC}
  )

  add_custom_target(${BASE_NAME} DEPENDS ${ASSET_DEST})
endfunction()

function(add_donut_asset)
  set(options)
  set(oneValueArgs SOURCE TARGET)
  set(multiValueArgs)
  cmake_parse_arguments(ASSET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ASSET_SOURCE)
    message(FATAL_ERROR "Raw asset FILE is required!")
  endif()

  find_program(
    DONUT_TOOL
    donut
    PATHS "${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}"
  )
  if (NOT DONUT_TOOL)
    message(FATAL_ERROR "The donut tool is required!")
  endif()

  set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/${ASSET_SOURCE}")

  if (ASSET_TARGET)
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_TARGET}")
  else()
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_SOURCE}.donut")
  endif()

  get_filename_component(BASE_NAME ${ASSET_DEST} NAME)

  add_custom_command(
    OUTPUT ${ASSET_DEST}
    COMMAND ${DONUT_TOOL} -f ${ASSET_SRC} -o ${ASSET_DEST}
    DEPENDS ${DONUT_TOOL} ${ASSET_SRC}
  )
  add_custom_target(${BASE_NAME} DEPENDS ${ASSET_DEST})
endfunction()


function(add_rle_asset)
  set(options)
  set(oneValueArgs SOURCE TARGET)
  set(multiValueArgs)
  cmake_parse_arguments(ASSET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ASSET_SOURCE)
    message(FATAL_ERROR "Raw asset FILE is required!")
  endif()

  find_program(
    RLE_TOOL
    rle-compress
    PATHS "${CMAKE_SOURCE_DIR}/tools"
  )
  if (NOT RLE_TOOL)
    message(FATAL_ERROR "The rle-compress tool is required!")
  endif()

  set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/${ASSET_SOURCE}")

  if (ASSET_TARGET)
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_TARGET}")
  else()
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_SOURCE}.rle")
  endif()

  get_filename_component(BASE_NAME ${ASSET_DEST} NAME)

  add_custom_command(
    OUTPUT ${ASSET_DEST}
    COMMAND ${RLE_TOOL} ${ASSET_SRC} ${ASSET_DEST}
    DEPENDS ${RLE_TOOL} ${ASSET_SRC}
  )
  add_custom_target(${BASE_NAME} DEPENDS ${ASSET_DEST})
endfunction()

function(add_zx02_asset)
  set(options)
  set(oneValueArgs SOURCE ALT TARGET)
  set(multiValueArgs)
  cmake_parse_arguments(ASSET "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT ASSET_SOURCE)
    message(FATAL_ERROR "Raw asset FILE is required!")
  endif()

  find_program(
    ZX02_TOOL
    zx02
    PATHS "${CMAKE_SOURCE_DIR}/tools/${CMAKE_HOST_SYSTEM_NAME}"
  )
  if (NOT ZX02_TOOL)
    message(FATAL_ERROR "The zx02 tool is required!")
  endif()

  set(ASSET_SRC "${CMAKE_SOURCE_DIR}/assets/${ASSET_SOURCE}")

  set(ASSET_TEMP "${CMAKE_BINARY_DIR}/assets/${ASSET_SOURCE}")

  if (ASSET_ALT)
    set(ASSET_ALT_SRC "${CMAKE_SOURCE_DIR}/assets/${ASSET_ALT}")
  endif()

  if (ASSET_TARGET)
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_TARGET}")
  else()
    set(ASSET_DEST "${CMAKE_BINARY_DIR}/assets/${ASSET_SOURCE}.zx02")
  endif()

  get_filename_component(BASE_NAME ${ASSET_DEST} NAME)

  add_custom_command(
    OUTPUT ${ASSET_DEST}
    COMMAND cat ${ASSET_SRC} ${ASSET_ALT_SRC} > ${ASSET_TEMP} && ${ZX02_TOOL} -f ${ASSET_TEMP} ${ASSET_DEST}
    DEPENDS ${ZX02_TOOL} ${ASSET_SRC} ${ASSET_ALT_SRC}
  )
  add_custom_target(${BASE_NAME} DEPENDS ${ASSET_DEST})
endfunction()
