cmake_minimum_required(VERSION 3.13..3.24)

include(CMakeDependentOption)
cmake_dependent_option(ES_GLES "Build the game with OpenGL ES" OFF UNIX OFF)
cmake_dependent_option(ES_STEAM "Build the game for the Steam Linux runtime" OFF UNIX OFF)
cmake_dependent_option(ES_STEAM_X86 "Build the game for the x86 Steam Linux runtime" OFF UNIX OFF)

option(ES_USE_SYSTEM_LIBRARIES "Use system libraries instead of the vcpkg ones." ${UNIX})

set(ES_VERSION_TYPE "alpha" CACHE STRING "What kind of version is being built. Controls the version string.")
set_property(CACHE ES_VERSION_TYPE PROPERTY STRINGS "alpha" "hash" "release")

# Support Debug and Release configurations.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "" FORCE)

# Use C++11 without any compiler specific extensions.
set(CMAKE_CXX_STANDARD 11 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Use LTO for Release builds only.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# On Linux use relative RPATH.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Add our overlays  for vcpkg and setup vcpkg.
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")
set(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics")
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
	CACHE STRING "Vcpkg toolchain file" FORCE)

if(ES_USE_SYSTEM_LIBRARIES)
	if(UNIX AND NOT APPLE)
		# If using the system libraries from a Linux distro, statically link every other library needed.
		# This is to make sure that the resulting binary has no dependencies on libraries (other than
		# the system ones) and can thus be distributed as a simple binary.
		set(VCPKG_TARGET_TRIPLET "x64-linux")
	endif()
else()
	# Tell vcpkg to use the required libraries.
	list(APPEND VCPKG_MANIFEST_FEATURES "system-libs")
endif()

# Some system libraries aren't available on the default Sniper runtime and need to be built from source.
if(ES_STEAM OR ES_STEAM_X86)
	set(ES_STEAM ON)
	list(APPEND VCPKG_MANIFEST_FEATURES "steam-libs")
	if(ES_STEAM_X86)
		set(VCPKG_TARGET_TRIPLET "x86-linux")
	endif()
endif()

# Various helpful options for IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EndlessSky)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

project("Endless Sky" VERSION 0.9.16
	DESCRIPTION "Space exploration, trading, and combat game."
	HOMEPAGE_URL https://endless-sky.github.io/
	LANGUAGES CXX)

# Configure various files that reference the current version number.
string(TIMESTAMP ES_CURRENT_DATE "%d %b %Y")
if(ES_VERSION_TYPE STREQUAL "hash")
	# Figure out the hash of the current commit. This won't change unless you recreate the
	# CMake build!
	execute_process(COMMAND git rev-parse --short HEAD
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE ES_HASH
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	set(ES_VERSION_SUFFIX "-alpha (${ES_HASH})")

	# Also add an expanded commit description inside credits.txt.
	execute_process(COMMAND git --no-pager log "--pretty=Last change by %an: %n %s" -1
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		OUTPUT_VARIABLE ES_LAST_CHANGE
		OUTPUT_STRIP_TRAILING_WHITESPACE)

	string(TIMESTAMP ES_BUILT_DATE "%Y-%m-%d %H-%M UTC" UTC)
	set(ES_CREDITS_CHANGE "\nBuilt: ${ES_BUILT_DATE}\n${ES_LAST_CHANGE}\n")
elseif(ES_VERSION_SUFFIX STREQUAL "alpha")
	set(ES_VERSION_SUFFIX "-alpha")
endif()
configure_file("./source/Version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/source/Version.h" @ONLY)
configure_file("./credits.txt.in" "${CMAKE_CURRENT_SOURCE_DIR}/credits.txt" @ONLY)
configure_file("./resources/endless-sky.6.in" "${CMAKE_CURRENT_SOURCE_DIR}/resources/endless-sky.6" @ONLY)
configure_file("./resources/endless-sky.exe.manifest.in" "${CMAKE_CURRENT_SOURCE_DIR}/resources/endless-sky.exe.manifest" @ONLY)

# Find the needed library dependencies.
find_package(Catch2 CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(glad CONFIG REQUIRED)

# Only very new versions of OpenAL provide a config package, so try to find it manually.
find_package(OpenAL CONFIG QUIET)
if(NOT OpenAL_FOUND)
	find_library(OPENAL openal REQUIRED)
	add_library(OpenAL UNKNOWN IMPORTED)
	add_library(OpenAL::OpenAL ALIAS OpenAL)
	set_target_properties(OpenAL PROPERTIES IMPORTED_LOCATION ${OPENAL})
endif()

# libmad doesn't provide native cmake support.
find_path(LIBMAD_INCLUDE_DIR mad.h)
find_library(LIBMAD_LIB_DEBUG NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib")
find_library(LIBMAD_LIB_RELEASE NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib")

# Add the library target.
add_subdirectory(source)
# Setup for the testing frameworks.
include(CTest)
if(BUILD_TESTING)
	add_subdirectory(tests)
endif()

# Create game target.
if(APPLE)
	add_executable(EndlessSky MACOSX_BUNDLE source/main.cpp)

	# MacOS bundles are a bit special and need every resource specified when
	# creating the executable.
	foreach(folder "data" "images" "sounds")
		file(GLOB_RECURSE RESOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${folder}/*")
		target_sources(EndlessSky PRIVATE ${RESOURCES})

		foreach(FILE ${RESOURCES})
		    # Get the relative path from the root folder to the current file.
		    file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}" ${FILE})
			# Get the parent directory for the new location.
			get_filename_component(FILE_PATH ${NEW_FILE} DIRECTORY)

			# Resources belong under Resources/.
			set_source_files_properties(${FILE} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/${FILE_PATH}")
		endforeach()
	endforeach()

	# Create the icns file, required for the bundle icon.
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/resources/endless-sky.icns
		COMMAND iconutil -c icns -o resources/endless-sky.icns resources/endless-sky.iconset
		DEPENDS resources/endless-sky.iconset/icon_16x16.png resources/endless-sky.iconset/icon_16x16@2x.png
			resources/endless-sky.iconset/icon_32x32.png resources/endless-sky.iconset/icon_32x32@2x.png
			resources/endless-sky.iconset/icon_128x128.png resources/endless-sky.iconset/icon_128x128@2x.png
			resources/endless-sky.iconset/icon_256x256.png resources/endless-sky.iconset/icon_256x256@2x.png
			resources/endless-sky.iconset/icon_512x512.png resources/endless-sky.iconset/icon_512x512@2x.png
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

	# Now do the same to standone files.
	foreach(file "license.txt" "keys.txt" "credits.txt" "copyright" "changelog" "resources/endless-sky.icns")
		target_sources(EndlessSky PRIVATE ${file})
		set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/")
	endforeach()

	# Add plist to bundle.
	set_target_properties(EndlessSky PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/resources/EndlessSky-Info.plist.in
		XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME endless-sky)
elseif(WIN32)
	add_executable(EndlessSky WIN32 source/main.cpp)
	target_sources(EndlessSky PRIVATE source/WinApp.rc)
else()
	add_executable(EndlessSky source/main.cpp)
	set_target_properties(EndlessSky PROPERTIES OUTPUT_NAME "endless-sky")
endif()

# Link with the library dependencies.
target_link_libraries(EndlessSky PRIVATE EndlessSkyLib $<$<TARGET_EXISTS:SDL2::SDL2main>:SDL2::SDL2main>)

# Link with OpenGL or OpenGL ES.
if(ES_GLES)
	find_package(OpenGL REQUIRED OpenGL EGL)
	target_link_libraries(EndlessSky PRIVATE OpenGL::OpenGL OpenGL::EGL)
	target_compile_definitions(EndlessSkyLib PUBLIC ES_GLES)
else()
	# The Steam runtime can't rely on GLVND being available, so link against the legacy interface.
	if(ES_STEAM)
		set(OpenGL_GL_PREFERENCE "LEGACY")
	endif()

	find_package(OpenGL REQUIRED)
	target_link_libraries(EndlessSky PRIVATE OpenGL::GL)

	if(APPLE)
		# Apple deprecated OpenGL in MacOS 10.14, but we don't care.
		target_compile_definitions(EndlessSkyLib PUBLIC GL_SILENCE_DEPRECATION)
	elseif(UNIX)
		target_link_libraries(EndlessSky PRIVATE X11)
	endif()
endif()

if(MINGW)
	# On Windows copy the MinGW runtime DLLs to the output folder as well.
	# This is to avoid the situation where a user has other MinGW runtime DLLs
	# in their PATH that are incompatible with the MinGW used to compile ES.
	if(WIN32)
		get_filename_component(PARENT_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
		get_filename_component(MINGW_RUNTIME_DIR ${PARENT_DIR} DIRECTORY)

		# MinGW doesn't have seh exceptions support for 32-bit Windows unfortunately,
		# and requires libgcc_s_dw2-1.dll instead of libgcc_s_seh-1.dll. There's no
		# perfect way to figure out which one to copy, so we simply copy both.
		# The executable will choose the correct DLL anyways.
		foreach(lib "stdc++-6" "winpthread-1" "gcc_s_seh-1" "gcc_s_dw2-1")
			file(GLOB_RECURSE FILE_PATH "${MINGW_RUNTIME_DIR}/lib${lib}.dll")
			if(FILE_PATH)
				add_custom_command(TARGET EndlessSky POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE_PATH} $<TARGET_FILE_DIR:EndlessSky>
					COMMAND_EXPAND_LISTS)

				# Add an install rule for this DLLs, so that it is also included when installing.
				install(FILES ${FILE_PATH} DESTINATION .)
			endif()
		endforeach()
	endif()
endif()

# Installation configurations.
if(APPLE)
	install(TARGETS EndlessSky CONFIGURATIONS Release BUNDLE DESTINATION .)
elseif(WIN32)
	# Install the binary.
	install(TARGETS EndlessSky CONFIGURATIONS Release RUNTIME DESTINATION .)

	# The MinGW DLLs needed were already installed above, and vcpkg installs the library DLLs.

	# Install the resouce files.
	install(DIRECTORY data DESTINATION .)
	install(DIRECTORY images DESTINATION .)
	install(DIRECTORY sounds DESTINATION .)
	install(FILES credits.txt DESTINATION .)
	install(FILES keys.txt DESTINATION .)
	install(FILES copyright DESTINATION .)
	install(FILES changelog DESTINATION .)
elseif(UNIX)
	# Install the binary.
	install(TARGETS EndlessSky CONFIGURATIONS Release RUNTIME DESTINATION games)

	# Install the desktop file.
	install(FILES resources/endless-sky.desktop DESTINATION share/applications)

	# Install app center metadata.
	install(FILES resources/endless-sky.appdata.xml DESTINATION share/metainfo)

	# Install icons, keeping track of all the paths.
	# Most Ubuntu apps supply 16, 22, 24, 32, 48, and 256, and sometimes others.
	foreach(size "16x16" "22x22" "24x24" "32x32" "48x48" "128x128" "256x256" "512x512")
		install(FILES "resources/icons/icon_${size}.png" DESTINATION "share/icons/hicolor/${size}/apps"
			RENAME endless-sky.png)
	endforeach()

	# Gzip and install man file.
	add_custom_command(
		OUTPUT endless-sky.6.gz
		COMMAND gzip -c resources/endless-sky.6 > ${CMAKE_CURRENT_BINARY_DIR}/endless-sky.6.gz
		DEPENDS resources/endless-sky.6
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	target_sources(EndlessSky PRIVATE endless-sky.6.gz)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/endless-sky.6.gz DESTINATION share/man/man6)

	# Install the resouce files.
	install(DIRECTORY data DESTINATION share/games/endless-sky)
	install(DIRECTORY images DESTINATION share/games/endless-sky)
	install(DIRECTORY sounds DESTINATION share/games/endless-sky)
	install(FILES credits.txt DESTINATION share/games/endless-sky)
	install(FILES keys.txt DESTINATION share/games/endless-sky)
	install(FILES copyright DESTINATION share/doc/endless-sky)
	install(FILES changelog DESTINATION share/doc/endless-sky)
endif()
