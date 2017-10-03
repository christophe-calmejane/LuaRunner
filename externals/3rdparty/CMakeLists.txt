# Lua library 3rd party target

project(liblua)

set(LUA_VERSIONMAJ 5)
set(LUA_VERSIONMIN 3)
set(LUA_VERSIONSUB 4)

set(LUA_SRC_DIR lua-${LUA_VERSIONMAJ}.${LUA_VERSIONMIN}.${LUA_VERSIONSUB}/src)

set(LUA_SOURCES_CORE
	${LUA_SRC_DIR}/lapi.c
	${LUA_SRC_DIR}/lcode.c
	${LUA_SRC_DIR}/lctype.c
	${LUA_SRC_DIR}/ldebug.c
	${LUA_SRC_DIR}/ldo.c
	${LUA_SRC_DIR}/ldump.c
	${LUA_SRC_DIR}/lfunc.c
	${LUA_SRC_DIR}/lgc.c
	${LUA_SRC_DIR}/llex.c
	${LUA_SRC_DIR}/lmem.c
	${LUA_SRC_DIR}/lobject.c
	${LUA_SRC_DIR}/lopcodes.c
	${LUA_SRC_DIR}/lparser.c
	${LUA_SRC_DIR}/lstate.c
	${LUA_SRC_DIR}/lstring.c
	${LUA_SRC_DIR}/ltable.c
	${LUA_SRC_DIR}/ltm.c
	${LUA_SRC_DIR}/lundump.c
	${LUA_SRC_DIR}/lvm.c
	${LUA_SRC_DIR}/lzio.c
)
set(LUA_SOURCES_LIB
	${LUA_SRC_DIR}/lauxlib.c
	${LUA_SRC_DIR}/lbaselib.c
	${LUA_SRC_DIR}/lbitlib.c
	${LUA_SRC_DIR}/lcorolib.c
	${LUA_SRC_DIR}/ldblib.c
	${LUA_SRC_DIR}/linit.c
	${LUA_SRC_DIR}/liolib.c
	${LUA_SRC_DIR}/lmathlib.c
	${LUA_SRC_DIR}/loadlib.c
	${LUA_SRC_DIR}/loslib.c
	${LUA_SRC_DIR}/lstrlib.c
	${LUA_SRC_DIR}/ltablib.c
	${LUA_SRC_DIR}/lutf8lib.c
)

set(LUA_PUBLIC_HEADERS
	${LUA_SRC_DIR}/lua.h
	${LUA_SRC_DIR}/lua.hpp
	${LUA_SRC_DIR}/luaconf.h
	${LUA_SRC_DIR}/lualib.h
	${LUA_SRC_DIR}/lauxlib.h
)

set(LUA_HEADERS
	${LUA_SRC_DIR}/lapi.h
	${LUA_SRC_DIR}/lcode.h
	${LUA_SRC_DIR}/lctype.h
	${LUA_SRC_DIR}/ldebug.h
	${LUA_SRC_DIR}/ldo.h
	${LUA_SRC_DIR}/lfunc.h
	${LUA_SRC_DIR}/lgc.h
	${LUA_SRC_DIR}/llex.h
	${LUA_SRC_DIR}/llimits.h
	${LUA_SRC_DIR}/lmem.h
	${LUA_SRC_DIR}/lobject.h
	${LUA_SRC_DIR}/lopcodes.h
	${LUA_SRC_DIR}/lparser.h
	${LUA_SRC_DIR}/lprefix.h
	${LUA_SRC_DIR}/lstate.h
	${LUA_SRC_DIR}/lstring.h
	${LUA_SRC_DIR}/ltable.h
	${LUA_SRC_DIR}/ltm.h
	${LUA_SRC_DIR}/lundump.h
	${LUA_SRC_DIR}/lvm.h
	${LUA_SRC_DIR}/lzio.h
)

set(LUA_RESOURCES
)

# Generate win32 resource file
if(WIN32)
	configure_file(
		"${CMAKE_CURRENT_SOURCE_DIR}/resources.rc.in"
		"${CMAKE_CURRENT_BINARY_DIR}/resources.rc"
	)
	list(APPEND LUA_RESOURCES "${CMAKE_CURRENT_BINARY_DIR}/resources.rc")
endif()

# Group sources
source_group("Header Files" FILES ${LUA_HEADERS})
source_group("Header Files\\Public" FILES ${LUA_PUBLIC_HEADERS})
source_group("Source Files\\Core" FILES ${LUA_SOURCES_CORE})
source_group("Source Files\\Libs" FILES ${LUA_SOURCES_LIB})
source_group("Resources" FILES ${LUA_RESOURCES})

# Define target
add_library(liblua SHARED ${LUA_SOURCES_CORE} ${LUA_SOURCES_LIB} ${LUA_HEADERS} ${LUA_PUBLIC_HEADERS} ${LUA_RESOURCES})
# Additional include directories
target_include_directories(liblua PUBLIC $<INSTALL_INTERFACE:include/lua> $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${LUA_SRC_DIR}>)
# Additional private compile options
target_compile_options(liblua PUBLIC -DLUA_BUILD_AS_DLL)
# Add a postfix in debug mode
set_target_properties(liblua PROPERTIES DEBUG_POSTFIX "-d")
# Use cmake folders
set_target_properties(liblua PROPERTIES FOLDER "Libraries/3rdParty")
# Setup install rules
lr_setup_library_install_rules(liblua)
install(FILES ${LUA_PUBLIC_HEADERS} DESTINATION include/lua)
