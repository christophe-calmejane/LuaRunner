# LuaRunner project

# Minimum CMake version
cmake_minimum_required(VERSION 3.6)

############ Versioning

# Using Semantic Versioning (http://semver.org/spec/v2.0.0.html)
# Project version number x.y.z on main branch, x.y.z.w on dev and task branches
# Example: 1.5.0 is the upcoming release version (main branch) of current 1.5.0.3 devel version (dev/task)
set(LUARUNNER_VERSION 1.1.0)

############ CMake configuration

project(LuaRunner)

# Using c++14 standard for all targets
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Set minimum OSX version
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE INTERNAL "Force the target to be at least a Mac OS X 10.9" FORCE)

# Enable cmake folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Configure installation path: we override the default installation path.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	set(CMAKE_INSTALL_PREFIX "./Install" CACHE PATH "default install path" FORCE)
endif()

# Include our internal macros
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/internals.cmake)

############ Define some variables

set(TARGET_ARCH 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
	set(TARGET_ARCH 64)
endif()
set(VS_USE_CLANG FALSE)
if(MSVC AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	set(VS_USE_CLANG TRUE)
endif()

############ Add projects

# Add 3rd party
message(STATUS "Building 3rd party")
add_subdirectory(externals/3rdparty)

# Add main library
message(STATUS "Building LuaRunner")
add_subdirectory(src)

# Add plugins
message(STATUS "Building plugins")
add_subdirectory(plugins)

# Set VisualStudio startup project
set_directory_properties(PROPERTIES VS_STARTUP_PROJECT LuaRunner)
