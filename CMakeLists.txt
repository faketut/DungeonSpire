cmake_minimum_required(VERSION 3.16)
project(DungeonSpire CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# Shared warning flags mirror the existing Makefile
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
    -Wall -Wextra -Wsuggest-override -Werror=vla
)

# Optional sanitizer support: -DENABLE_ASAN=ON / -DENABLE_UBSAN=ON
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(ENABLE_UBSAN "Build with UndefinedBehaviorSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(project_warnings INTERFACE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(project_warnings INTERFACE -fsanitize=address)
endif()
if(ENABLE_UBSAN)
    target_compile_options(project_warnings INTERFACE -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(project_warnings INTERFACE -fsanitize=undefined)
endif()

# Optional coverage: -DENABLE_COVERAGE=ON (gcc/clang only)
option(ENABLE_COVERAGE "Build with coverage instrumentation" OFF)
if(ENABLE_COVERAGE)
    target_compile_options(project_warnings INTERFACE --coverage -O0 -g)
    target_link_options(project_warnings INTERFACE --coverage)
endif()

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
