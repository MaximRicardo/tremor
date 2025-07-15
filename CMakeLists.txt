cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(quakent)

add_compile_options(-std=c++20 -Wall -Wextra -Wpedantic -Dm_RAYLIB=1)

# i am speed
# add_compile_options(-ftree-vectorize -msse2 -mfpmath=sse -O2)

# for debugging
 add_compile_options(-ggdb3 -fsanitize=address -fsanitize=undefined)
 add_link_options(-fsanitize=address -fsanitize=undefined)

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(quakent "${Sources}")

target_link_libraries(quakent -lraylib -lGL -lm -lpthread -ldl -lrt -lX11)
target_include_directories(quakent PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
