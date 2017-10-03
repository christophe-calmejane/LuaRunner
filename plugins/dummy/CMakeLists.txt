# Dummy plugin

project(Dummy LANGUAGES C CXX VERSION 1.0.0)

# Common files
set(HEADER_FILES_COMMON
)

set(SOURCE_FILES_COMMON
	dummy.cpp
)

# Group sources
source_group("Header Files" FILES ${HEADER_FILES_COMMON})
source_group("Source Files" FILES ${SOURCE_FILES_COMMON})

# Define target
add_library(Dummy SHARED ${HEADER_FILES_COMMON} ${SOURCE_FILES_COMMON})
target_compile_options(Dummy PRIVATE "-D_CRT_SECURE_NO_WARNINGS")
target_link_libraries(Dummy luaRunnerPlugin_interface)
target_link_libraries(Dummy liblua)

# Copy shared library to output folder as post-build (for easy test/debug)
add_custom_command(
	TARGET Dummy
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Dummy> "${LuaRunner_BINARY_DIR}"
	COMMENT "Copying Dummy plugin to LuaRunner output folder for easy debug"
	VERBATIM
)
