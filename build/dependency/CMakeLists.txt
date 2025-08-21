cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0076 NEW)

# Debug paths
message("xerr CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message("xerr Source file: ${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h")
    message(STATUS "xerr.h found at ${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h")
else()
    message(WARNING "xerr.h NOT found at ${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h")
endif()

# Define xerr as an interface library
if(NOT TARGET xerr)
    add_library(xerr INTERFACE)
    target_include_directories(xerr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

    # Define file list
    set(XERR_FILES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../../readme.md"
    )

    # Set global properties
    set_property(GLOBAL PROPERTY XERR_GROUP "dependencies/xcore/xerr")
    set_property(GLOBAL PROPERTY XERR_FILES "${XERR_FILES_LIST}")
    set_property(GLOBAL APPEND PROPERTY COMPONENT_REGISTRY "XERR")
endif()
