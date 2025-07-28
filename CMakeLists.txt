cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(tremor)

# base options
add_compile_options(-std=c++20 -Wall -Wextra -Wpedantic -fno-rtti)

# select the rendering API
# the selected api will be used as a pass-through to send pixels to the screen
add_compile_options(-Dm_RAYLIB=1)

# faster
add_compile_options(-O1)

# i am speed
# add_compile_options(-ftree-vectorize -msse2 -mfpmath=sse -O3 -flto)
# add_link_options(-flto)
# even more speed
# add_compile_options(-DNDEBUG)

# for debugging
# add_compile_options(-ggdb3 -fsanitize=address -fsanitize=undefined)
# add_link_options(-fsanitize=address -fsanitize=undefined)

# release build options
# add_compile_options(-O3 -flto -DNDEBUG)
# add_link_options(-flto)

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(tremor "${Sources}")

target_link_libraries(tremor -lraylib -lGL -lm -lpthread -ldl -lrt -lX11)
