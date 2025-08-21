cmake_minimum_required(VERSION 3.10)

# Define xerr as an interface library
if(NOT TARGET xerr)
    add_library(xerr INTERFACE)

    # path to xerr for the include files
    set(XERR_INCLUDE_LIST "${CMAKE_CURRENT_SOURCE_DIR}/../.."
    )

    # target_include_directories(xerr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

    # Define file list
    set(XERR_FILES_LIST "${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h"
                        "${CMAKE_CURRENT_SOURCE_DIR}/../../readme.md"
    )

    # Set global properties
    set_property(GLOBAL PROPERTY XERR_GROUP "dependencies/xcore/xerr")
    set_property(GLOBAL PROPERTY XERR_FILES "${XERR_FILES_LIST}")
    set_property(GLOBAL PROPERTY XERR_INCLUDES "${XERR_INCLUDE_LIST}")
    set_property(GLOBAL APPEND PROPERTY COMPONENT_REGISTRY "XERR")
endif()
