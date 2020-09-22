# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   message(FATAL_ERROR "CMake >= 2.6.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 2.6...3.17)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_targetsDefined)
set(_targetsNotDefined)
set(_expectedTargets)
foreach(_expectedTarget spirv-cross-c)
  list(APPEND _expectedTargets ${_expectedTarget})
  if(NOT TARGET ${_expectedTarget})
    list(APPEND _targetsNotDefined ${_expectedTarget})
  endif()
  if(TARGET ${_expectedTarget})
    list(APPEND _targetsDefined ${_expectedTarget})
  endif()
endforeach()
if("${_targetsDefined}" STREQUAL "${_expectedTargets}")
  unset(_targetsDefined)
  unset(_targetsNotDefined)
  unset(_expectedTargets)
  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT "${_targetsDefined}" STREQUAL "")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_targetsDefined}\nTargets not yet defined: ${_targetsNotDefined}\n")
endif()
unset(_targetsDefined)
unset(_targetsNotDefined)
unset(_expectedTargets)


# Create imported target spirv-cross-c
add_library(spirv-cross-c STATIC IMPORTED)

set_target_properties(spirv-cross-c PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "C:/Users/eugch/OneDrive/Documents/programmation/C++/vulkan/Graphics-Experimental-Engine/libs/SPIRV-Cross"
)

# Import target "spirv-cross-c" for configuration "Debug"
set_property(TARGET spirv-cross-c APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(spirv-cross-c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_DEBUG "spirv-cross-glsl;spirv-cross-hlsl;spirv-cross-msl;spirv-cross-cpp;spirv-cross-reflect"
  IMPORTED_LOCATION_DEBUG "C:/Users/eugch/OneDrive/Documents/programmation/C++/vulkan/Graphics-Experimental-Engine/libs/SPIRV-Cross/Debug/spirv-cross-cd.lib"
  )

# Import target "spirv-cross-c" for configuration "Release"
set_property(TARGET spirv-cross-c APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(spirv-cross-c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELEASE "spirv-cross-glsl;spirv-cross-hlsl;spirv-cross-msl;spirv-cross-cpp;spirv-cross-reflect"
  IMPORTED_LOCATION_RELEASE "C:/Users/eugch/OneDrive/Documents/programmation/C++/vulkan/Graphics-Experimental-Engine/libs/SPIRV-Cross/Release/spirv-cross-c.lib"
  )

# Import target "spirv-cross-c" for configuration "MinSizeRel"
set_property(TARGET spirv-cross-c APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(spirv-cross-c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_MINSIZEREL "spirv-cross-glsl;spirv-cross-hlsl;spirv-cross-msl;spirv-cross-cpp;spirv-cross-reflect"
  IMPORTED_LOCATION_MINSIZEREL "C:/Users/eugch/OneDrive/Documents/programmation/C++/vulkan/Graphics-Experimental-Engine/libs/SPIRV-Cross/MinSizeRel/spirv-cross-c.lib"
  )

# Import target "spirv-cross-c" for configuration "RelWithDebInfo"
set_property(TARGET spirv-cross-c APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(spirv-cross-c PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LINK_INTERFACE_LIBRARIES_RELWITHDEBINFO "spirv-cross-glsl;spirv-cross-hlsl;spirv-cross-msl;spirv-cross-cpp;spirv-cross-reflect"
  IMPORTED_LOCATION_RELWITHDEBINFO "C:/Users/eugch/OneDrive/Documents/programmation/C++/vulkan/Graphics-Experimental-Engine/libs/SPIRV-Cross/RelWithDebInfo/spirv-cross-c.lib"
  )

# Make sure the targets which have been exported in some other
# export set exist.
unset(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)
foreach(_target "spirv-cross-glsl" "spirv-cross-hlsl" "spirv-cross-msl" "spirv-cross-cpp" "spirv-cross-reflect" )
  if(NOT TARGET "${_target}" )
    set(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets "${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets} ${_target}")
  endif()
endforeach()

if(DEFINED ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)
  if(CMAKE_FIND_PACKAGE_NAME)
    set( ${CMAKE_FIND_PACKAGE_NAME}_FOUND FALSE)
    set( ${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE "The following imported targets are referenced, but are missing: ${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets}")
  else()
    message(FATAL_ERROR "The following imported targets are referenced, but are missing: ${${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets}")
  endif()
endif()
unset(${CMAKE_FIND_PACKAGE_NAME}_NOT_FOUND_MESSAGE_targets)

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
