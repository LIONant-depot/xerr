# The library
cmake_minimum_required(VERSION 3.10)

# Define as an interface library only if not already defined
if(NOT TARGET xerr)
  add_library(xerr INTERFACE)
  target_include_directories(xerr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")
endif()