# LuaWrapper library 3rd party target

project(luawrapper)

# Plugin interface target
add_library(${PROJECT_NAME}_interface INTERFACE)
# Additional include directories
target_include_directories(${PROJECT_NAME}_interface INTERFACE $<INSTALL_INTERFACE:include> $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)
# Setup install rules
if(INSTALL_LUARUNNER_LIBS)
	lr_setup_library_install_rules(${PROJECT_NAME}_interface)
endif()
# Set installation rules
if(INSTALL_LUARUNNER_HEADERS)
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/LuaWrapper CONFIGURATIONS Release DESTINATION include FILES_MATCHING PATTERN "*.hpp")
endif()
