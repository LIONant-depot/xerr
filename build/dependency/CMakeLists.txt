cmake_minimum_required(VERSION 3.10)

# Define xerr as an interface library
if(NOT TARGET xerr)
    add_library(xerr INTERFACE)
   
    #set the root for this project
    set(ROOT "${CMAKE_SOURCE_DIR}/dependencies/xerr")

    # path to xerr for the include files
    set(XERR_INCLUDE_LIST "${ROOT}"
    )
    
    # Define file list
    set(XERR_FILES_LIST "${ROOT}/source/xerr.h"
                        "${ROOT}/readme.md"
    )

    # Set global properties
    set_property(GLOBAL PROPERTY XERR_GROUP "dependencies/xcore/xerr")
    set_property(GLOBAL PROPERTY XERR_FILES "${XERR_FILES_LIST}")
    set_property(GLOBAL PROPERTY XERR_INCLUDES "${XERR_INCLUDE_LIST}")
    set_property(GLOBAL APPEND PROPERTY COMPONENT_REGISTRY "XERR")
endif()
