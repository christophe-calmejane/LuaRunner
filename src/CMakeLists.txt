# LuaRunner target

project(LuaRunner LANGUAGES C CXX VERSION ${LUARUNNER_VERSION})

# Configure file based on CMakeLists.txt
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
	"${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

# Public files
set(HEADER_FILES_PUBLIC
	luaRunnerPlugin.hpp
)

# Common files
set(HEADER_FILES_COMMON
	${CMAKE_CURRENT_BINARY_DIR}/config.h
	execute.hpp
	pluginManager.hpp
	builtin.hpp
)

set(SOURCE_FILES_COMMON
	luaRunner.cpp
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

# Define target
add_executable(LuaRunner ${HEADER_FILES_PUBLIC} ${HEADER_FILES_COMMON} ${SOURCE_FILES_COMMON} ${TEST_SCRIPT_FILES})
target_include_directories(LuaRunner PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(LuaRunner PRIVATE "-DLUARUNNER_IMPORTS")
target_link_libraries(LuaRunner liblua)

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
install(TARGETS LuaRunner RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
