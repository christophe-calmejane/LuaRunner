# LuaRunner target

project(LuaRunner LANGUAGES C CXX VERSION ${LUARUNNER_VERSION})

# Configure file based on CMakeLists.txt
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# Public files
set(HEADER_FILES_PUBLIC
	${LUARUNNER_ROOT_FOLDER}/include/luaRunner/version.hpp
	${LUARUNNER_ROOT_FOLDER}/include/luaRunner/execute.hpp
	${LUARUNNER_ROOT_FOLDER}/include/luaRunner/plugin.hpp
)

# Common files
set(HEADER_FILES_COMMON
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	pluginManager.hpp
	builtin.hpp
)

set(SOURCE_FILES_COMMON
	version.cpp
	execute.cpp
	pluginManager.cpp
	builtin.cpp
)

set(TEST_SCRIPT_FILES
	${LUARUNNER_ROOT_FOLDER}/tests/helloWorld.lua
)

# Group sources
source_group("Header Files\\Public" FILES ${HEADER_FILES_PUBLIC})
source_group("Header Files" FILES ${HEADER_FILES_COMMON})
source_group("Source Files" FILES ${SOURCE_FILES_COMMON})
source_group("Test Scripts" FILES ${TEST_SCRIPT_FILES})

# Library target
add_library(luaRunner_static STATIC ${HEADER_FILES_PUBLIC} ${HEADER_FILES_COMMON} ${SOURCE_FILES_COMMON})
target_include_directories(luaRunner_static PUBLIC $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${LUARUNNER_ROOT_FOLDER}/include> PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(luaRunner_static PRIVATE "-DLUARUNNER_IMPORTS")
target_link_libraries(luaRunner_static PUBLIC liblua)

set(SOURCE_FILES_BINARY
	luaRunner.cpp
)
# Binary target
add_executable(LuaRunner ${SOURCE_FILES_BINARY})
target_link_libraries(LuaRunner luaRunner_static)

# Copy liblua to output folder as post-build (for easy test/debug)
add_custom_command(
	TARGET LuaRunner
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:liblua> "${CMAKE_CURRENT_BINARY_DIR}"
	COMMENT "Copying liblua to LuaRunner output folder for easy debug"
	VERBATIM
)


# Define install rules
install(FILES ${HEADER_FILES_PUBLIC} DESTINATION include/luaRunner)
install(TARGETS LuaRunner luaRunner_static RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
