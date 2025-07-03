cmake_minimum_required(VERSION 3.10)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(quakent)

# add_compile_options(-ftree-vectorize -msse2 -mfpmath=sse -Wall -Wextra -Wpedantic -O2)

#For debugging
add_compile_options(-Wall -Wextra -Wpedantic -ggdb3 -fsanitize=address)
add_link_options(-fsanitize=address)

file(GLOB_RECURSE Sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(quakent "${Sources}")

target_link_libraries(quakent -lraylib -lGL -lm -lpthread -ldl -lrt -lX11)
