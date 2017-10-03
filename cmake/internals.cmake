# Internal utility macros and functions

###############################################################################
# Set maximum warning level, and treat warnings as errors
# Applies on a target, must be called after target has been defined with
# 'add_library' or 'add_executable'.
function(set_maximum_warnings TARGET_NAME)
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR APPLE OR VS_USE_CLANG)
		target_compile_options(${TARGET_NAME} PRIVATE -Wall -Werror -g)
	elseif(MSVC)
		# Don't use Wall on MSVC, it prints too many stupid warnings
		target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
	endif()
endfunction(set_maximum_warnings)

###############################################################################
# Set the DEBUG define in debug mode
# Applies on a target, must be called after target has been defined with
# 'add_library' or 'add_executable'.
function(set_debug_define TARGET_NAME)
	# Flags to add for DEBUG
	target_compile_options(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-DDEBUG>)
endfunction(set_debug_define)

###############################################################################
# Remove VisualStudio useless deprecated warnings (CRT, CRT_SECURE, WINSOCK)
# Applies on a target, must be called after target has been defined with
# 'add_library' or 'add_executable'.
function(remove_vs_deprecated_warnings TARGET_NAME)
	if(MSVC)
		target_compile_options(${TARGET_NAME} PRIVATE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS -D_WINSOCK_DEPRECATED_NO_WARNINGS)
	endif()
endfunction(remove_vs_deprecated_warnings)

###############################################################################
# Force symbols file generation for build configs (pdb or dSYM)
# Applies on a target, must be called after target has been defined with
# 'add_library' or 'add_executable'.
function(force_symbols_file TARGET_NAME)
	get_target_property(targetType ${TARGET_NAME} TYPE)

	if(MSVC)
		if(VS_USE_CLANG)
			target_compile_options(${TARGET_NAME} PRIVATE -g2 -gdwarf-2)
		else()
			target_compile_options(${TARGET_NAME} PRIVATE /Zi)
		endif()
		set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /OPT:REF /OPT:ICF /INCREMENTAL:NO")
	elseif(APPLE)
		target_compile_options(${TARGET_NAME} PRIVATE -g)

		if(${targetType} STREQUAL "STATIC_LIBRARY")
			# MacOS do not support dSYM file for static libraries
			set_target_properties(${TARGET_NAME} PROPERTIES
				XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf"
				XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf"
				XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Debug] "NO"
				XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] "NO"
			)
		else()
			set_target_properties(${TARGET_NAME} PROPERTIES
				XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf-with-dsym"
				XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Release] "dwarf-with-dsym"
				XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Debug] "YES"
				XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Release] "YES"
			)
		endif()
	endif()
endfunction(force_symbols_file)

###############################################################################
# Setup symbols for a target.
function(setup_symbols TARGET_NAME)
	# Force symbols file generation
	force_symbols_file(${TARGET_NAME})
endfunction(setup_symbols)

###############################################################################
# Setup common options for a library target
function(setup_library_options TARGET_NAME)
	if(MSVC)
		# Set WIN32 version since we want to target WinVista minimum
		target_compile_options(${TARGET_NAME} PRIVATE -D_WIN32_WINNT=0x0600)
	endif()
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		if(NOT WIN32)
			# Build using fPIC
			target_compile_options(${TARGET_NAME} PRIVATE -fPIC)
		endif()
	endif()

	# Set full warnings (including treat warnings as error)
	set_maximum_warnings(${TARGET_NAME})
	
	# Set the "DEBUG" define in debug compilation mode
	set_debug_define(${TARGET_NAME})
	
	# Prevent visual studio deprecated warnings about CRT and Sockets
	remove_vs_deprecated_warnings(${TARGET_NAME})
	
	# Add a postfix in debug mode
	set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "-d")

	# Use cmake folders
	set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Libraries")

	# Setup debug symbols
	setup_symbols(${TARGET_NAME})

endfunction(setup_library_options)

###############################################################################
# Setup common install rules for a library target
function(setup_library_install_rules TARGET_NAME)
	# Get target type for specific options
	get_target_property(targetType ${TARGET_NAME} TYPE)

	# Static library install rules
	if(${targetType} STREQUAL "STATIC_LIBRARY")
		install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME} ARCHIVE DESTINATION lib)
		install(EXPORT ${TARGET_NAME} DESTINATION cmake)

	# Shared library install rules
	elseif(${targetType} STREQUAL "SHARED_LIBRARY")
		install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
		install(EXPORT ${TARGET_NAME} DESTINATION cmake)

	# Interface library install rules
	elseif(${targetType} STREQUAL "INTERFACE_LIBRARY")
		install(TARGETS ${TARGET_NAME} EXPORT ${TARGET_NAME})
		install(EXPORT ${TARGET_NAME} DESTINATION cmake)
	
	# Unsupported target type
	else()
		message(FATAL_ERROR "Unsupported target type for setup_library_install_rules macro: ${targetType}")
	endif()

endfunction(setup_library_install_rules)

###############################################################################
# Setup common options for an executable target.
function(setup_executable_options TARGET_NAME)
	if(MSVC)
		# Set WIN32 version since we want to target WinVista minimum
		target_compile_options(${TARGET_NAME} PRIVATE -D_WIN32_WINNT=0x0600)
	endif()

	# Add a postfix in debug mode
	set_target_properties(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "-d")

	# Setup debug symbols
	setup_symbols(${TARGET_NAME})

	# Set rpath for MacOs
	if(APPLE)
		get_target_property(isBundle ${TARGET_NAME} MACOSX_BUNDLE)
		if(${isBundle})
			set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@executable_path/../Frameworks")
			# Directly use install rpath for app bundles, since we copy dylibs into the bundle during post build
			set_target_properties(${TARGET_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
		else()
			set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@executable_path/../lib")
			# Directly use install rpath for command line apps too
			set_target_properties(${TARGET_NAME} PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
		endif()
	endif()
	# Set rpath for linux
	if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
		set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "../lib")
	endif()
	
endfunction(setup_executable_options)

###############################################################################
# Copy the runtime part of MODULE_NAME to the output folder of TARGET_NAME for
# easy debugging from the IDE.
function(copy_runtime TARGET_NAME MODULE_NAME)
	# Get module type
	get_target_property(moduleType ${MODULE_NAME} TYPE)
	# Module is not a shared library, no need to copy
	if(NOT ${moduleType} STREQUAL "SHARED_LIBRARY")
		return()
	endif()

	get_target_property(isBundle ${TARGET_NAME} MACOSX_BUNDLE)
	# For mac non-bundle apps, we copy the dylibs to the lib sub folder, so it matches the same rpath than when installing (since we use install_rpath)
	if(APPLE AND NOT ${isBundle})
		set(addSubDestPath "/../lib")
	else()
		set(addSubDestPath "")
	endif()

	# Copy shared library to output folder as post-build (for easy test/debug)
	add_custom_command(
		TARGET ${TARGET_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>${addSubDestPath}"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${MODULE_NAME}> "$<TARGET_FILE_DIR:${TARGET_NAME}>${addSubDestPath}"
		COMMENT "Copying ${MODULE_NAME} shared library to ${TARGET_NAME} output folder for easy debug"
		VERBATIM
	)
endfunction(copy_runtime)

###############################################################################
# Global variables (must stay at the end of the file)
set(LUARUNNER_ROOT_FOLDER "${PROJECT_SOURCE_DIR}") # Folder containing the main CMakeLists.txt for the repository including this file
