cmake_minimum_required(VERSION 3.20.0)
project(fl_music VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS off)
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/exe)

file(GLOB_RECURSE SRC src/*.cpp include/*.cpp)
# add files
add_executable(fl_music WIN32 ${SRC})

target_include_directories(fl_music PUBLIC include)
target_link_libraries(fl_music -municode fltk portaudio sndfile tag)
