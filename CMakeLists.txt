cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS "-Wall")

project(LZMA_Test)

file(GLOB_RECURSE dummy_SOURCES "src/*.cpp" "src/*.c")
file(GLOB_RECURSE dummy_HEADERS "src/*.hpp" "src/*.h")

find_package(LibLZMA REQUIRED)
include_directories(${LIBLZMA_INCLUDE_DIR})
set(LIBS ${LIBS} ${LIBLZMA_LIBRARIES})

add_executable(LZMA_Test ${dummy_SOURCES} ${dummy_HEADERS})
target_link_libraries(LZMA_Test ${LIBS})