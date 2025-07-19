# Find Ruby executable
find_program(RUBY_EXECUTABLE ruby)

if(NOT RUBY_EXECUTABLE)
    message(FATAL_ERROR "Ruby is required but not found. Please install Ruby.")
endif()

# Check Ruby version
execute_process(
    COMMAND ${RUBY_EXECUTABLE} --version
    OUTPUT_VARIABLE RUBY_VERSION_OUTPUT
    RESULT_VARIABLE RUBY_VERSION_RESULT
)

if(RUBY_VERSION_RESULT EQUAL 0)
    string(REGEX MATCH "ruby ([0-9]+\\.[0-9]+\\.[0-9]+)" RUBY_VERSION_MATCH "${RUBY_VERSION_OUTPUT}")
    if(RUBY_VERSION_MATCH)
        set(RUBY_VERSION "${CMAKE_MATCH_1}")
        message(STATUS "Found Ruby version: ${RUBY_VERSION}")
    endif()
endif() 