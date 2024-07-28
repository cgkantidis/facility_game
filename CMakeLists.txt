cmake_minimum_required(VERSION 3.25)

project(
  facility_game
  VERSION 0.0.1
  LANGUAGES CXX)

# set the C++ standard
set(CMAKE_CXX_STANDARD 20)

# generate the compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# enable all warnings and treat them as errors
add_compile_options(
  -Wall
  -Wextra
  -Wshadow
  -Wnon-virtual-dtor
  -pedantic
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wdouble-promotion
  -Wformat=2)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_compile_options(
    -Wmisleading-indentation
    -Wduplicated-cond
    -Wduplicated-branches
    -Wlogical-op
    -Wnull-dereference
    -Wuseless-cast
    -fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(-Wimplicit-fallthrough -fcolor-diagnostics -ferror-limit=0)
endif()

add_compile_options(-Werror)

set(allowedBuildTypes
    Debug
    Release
    ASAN
    MSAN
    UBSAN
    TSAN
    Profile
    HeapProfile)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
  message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
endif()

# in case of compilation error in the pegtl templates, this will show the full
# stacktrace
add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-ftemplate-backtrace-limit=0>)

set(CMAKE_C_FLAGS_DEBUG
    "-g -O0"
    CACHE STRING "Debug build" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG
    "-g -O0"
    CACHE STRING "Debug build" FORCE)
set(CMAKE_C_FLAGS_RELEASE
    "-DNDEBUG -O3"
    CACHE STRING "Release build" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE
    "-DNDEBUG -O3"
    CACHE STRING "Release build" FORCE)
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0"
    CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_ASAN
    "-fsanitize=address -fno-sanitize-recover=all -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0"
    CACHE STRING "Address Sanitizer build" FORCE)
set(CMAKE_C_FLAGS_MSAN
    "-fsanitize=memory -fno-sanitize-recover=all -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0"
    CACHE STRING "Memory Sanitizer build" FORCE)
set(CMAKE_CXX_FLAGS_MSAN
    "-fsanitize=memory -fno-sanitize-recover=all -fsanitize-memory-track-origins -fno-omit-frame-pointer -fno-optimize-sibling-calls -g -O0"
    CACHE STRING "Memory Sanitizer build" FORCE)
set(CMAKE_C_FLAGS_UBSAN
    "-fsanitize=undefined -fno-sanitize-recover=all -g -O0"
    CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_UBSAN
    "-fsanitize=undefined -fno-sanitize-recover=all -g -O0"
    CACHE STRING "Undefined Behaviour Sanitizer" FORCE)
set(CMAKE_C_FLAGS_TSAN
    "-fsanitize=thread -fno-sanitize-recover=all -g -O1"
    CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_CXX_FLAGS_TSAN
    "-fsanitize=thread -fno-sanitize-recover=all -g -O1"
    CACHE STRING "Thread Sanitizer" FORCE)
set(CMAKE_C_FLAGS_PROFILE
    "-pg -g"
    CACHE STRING "gprof profiler" FORCE)
set(CMAKE_CXX_FLAGS_PROFILE
    "-pg -g"
    CACHE STRING "gprof profiler" FORCE)
set(CMAKE_C_FLAGS_HEAPPROFILE
    "-g -O0"
    CACHE STRING "Heap Profiler" FORCE)
set(CMAKE_CXX_FLAGS_HEAPPROFILE
    "-g -O0"
    CACHE STRING "Heap Profiler" FORCE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "MSAN" AND "${CMAKE_CXX_COMPILER_ID}"
                                             STREQUAL "GNU")
  message(
    FATAL_ERROR
      "Memory sanitizer is not currently supported by gcc. Try clang instead.")
endif()

include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  OVERRIDE_FIND_PACKAGE)
find_package(fmt REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(facility_game facility_game.cpp)

target_link_libraries(facility_game PRIVATE fmt::fmt)

if(CMAKE_BUILD_TYPE STREQUAL Profile)
  target_link_options(facility_game_server PRIVATE "-pg")
  target_link_options(facility_game_client PRIVATE "-pg")
elseif(CMAKE_BUILD_TYPE STREQUAL HeapProfile)
  target_link_libraries(facility_game_server PRIVATE tcmalloc)
  target_link_libraries(facility_game_client PRIVATE tcmalloc)
elseif(CMAKE_BUILD_TYPE STREQUAL TSAN)
  target_link_options(facility_game_server PRIVATE -fsanitize=thread)
  target_link_options(facility_game_client PRIVATE -fsanitize=thread)
elseif(CMAKE_BUILD_TYPE STREQUAL MSAN)
  target_link_options(facility_game_server PRIVATE -fsanitize=memory)
  target_link_options(facility_game_client PRIVATE -fsanitize=memory)
endif()

install(TARGETS facility_game)
