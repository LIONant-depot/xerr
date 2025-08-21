# The library
cmake_minimum_required(VERSION 3.10)

# Define as an interface library only if not already defined
if(NOT TARGET xerr)
  add_library(xerr INTERFACE)
  target_include_directories(xerr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../..")

  # Define file list for source_group (adjust paths based on actual xerr files)
  set(XERR_FILES_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/../../source/xerr.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../README.md"
    )  

  # Add sources to the parent target (optional, only if xerr provides compilable files)
  # target_sources(${TARGET_PROJECT} PRIVATE ${XERR_FILES_LIST})
  # Define component grouping info
  set(XERR_GROUP "dependencies/xcore/xerr" PARENT_SCOPE)
  set(XERR_FILES ${XERR_FILES_LIST} PARENT_SCOPE)
  
  # Append to COMPONENT_REGISTRY
  list(APPEND COMPONENT_REGISTRY "XERR")
  set(COMPONENT_REGISTRY ${COMPONENT_REGISTRY} PARENT_SCOPE)    

endif()