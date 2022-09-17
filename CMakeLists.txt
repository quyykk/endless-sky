cmake_minimum_required(VERSION 3.12...3.23)

option(ES_GLES "Build the game with OpenGL ES on Linux" OFF)
option(ES_STEAM "Build the game for the Steam Linux runtime" OFF)
option(ES_STEAM_X86 "Build the game for the x86 Steam Linux runtime" OFF)
option(ES_USE_SYSTEM_LIBRARIES "For Linux use the distro system libraries instead of the vcpkg ones." ON)
option(ES_RELEASE "Whether the current version is a published release" OFF)
option(ES_INCLUDE_HASH "Whether to include the current git hash as part of the version. Doesn't update unless you regenerate!" OFF)

# Use C++11 without any compiler specific extensions.
set(CMAKE_CXX_STANDARD 11 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Use LTO for Release builds only.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# On Linux use relative RPATH.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Support only Debug and Release configurations, with Debug being the default.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# Add our overlays  for vcpkg and setup vcpkg.
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")
set(VCPKG_OVERLAY_PORTS "${CMAKE_CURRENT_SOURCE_DIR}/overlays")
set(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
	CACHE STRING "Vcpkg toolchain file")

# Using system libraries instead of vcpkg libraries is only supported on Linux.
if(ES_USE_SYSTEM_LIBRARIES AND (NOT UNIX OR APPLE))
	set(ES_USE_SYSTEM_LIBRARIES OFF)
endif()

if(ES_USE_SYSTEM_LIBRARIES)
	# If using the system libraries from the distro, statically link every other library needed.
	# This is to make sure that the resulting binary has no dependencies on libraries (other than
	# the system ones) and can thus be distributed as a simple binary.
	set(VCPKG_TARGET_TRIPLET "x64-linux")
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

project("Endless Sky" VERSION 0.9.15
	DESCRIPTION "Space exploration, trading, and combat game."
	LANGUAGES CXX)

# Configure various files that reference the current version number.
string(TIMESTAMP ES_CURRENT_DATE "%d %b %Y")
if(NOT ES_RELEASE)
	if(ES_INCLUDE_HASH)
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
	else()
		set(ES_VERSION_SUFFIX "-alpha")
	endif()
endif()
configure_file("./source/Version.h.in" "${CMAKE_CURRENT_SOURCE_DIR}/source/Version.h" @ONLY)
configure_file("./credits.txt.in" "${CMAKE_CURRENT_SOURCE_DIR}/credits.txt" @ONLY)
configure_file("./resources/endless-sky.6.in" "${CMAKE_CURRENT_SOURCE_DIR}/resources/endless-sky.6" @ONLY)

# Use the system libraries if specified and if not use the ones built from vcpkg.
if(ES_USE_SYSTEM_LIBRARIES)
	find_library(SDL2 SDL2)
	find_library(OpenAL openal)
	find_library(PNG png)
	find_library(JPEG jpeg)
	find_library(GLEW GLEW)

	if(NOT ES_STEAM)
		find_library(LIBMAD mad)
	endif()
else()
	find_package(SDL2 CONFIG REQUIRED)
	find_package(OpenAL CONFIG REQUIRED)
	find_package(PNG REQUIRED)
	find_package(JPEG REQUIRED)
	if(NOT APPLE)
		find_package(GLEW REQUIRED)
	endif()
endif()

if(NOT ES_USE_SYSTEM_LIBRARIES OR ES_STEAM)
	# libmad doesn't provide native cmake support.
	find_path(LIBMAD_INCLUDE_DIR mad.h)
	find_library(LIBMAD_LIB_DEBUG NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
	find_library(LIBMAD_LIB_RELEASE NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)
endif()

find_package(Catch2 CONFIG REQUIRED)

# Create library target. The source files are populated inside source/CMakeLists.txt.
add_library(EndlessSkyLib STATIC)
add_subdirectory(source)

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
	foreach(file "license.txt" "icon.png" "keys.txt" "credits.txt" "copyright" "changelog" "resources/endless-sky.icns")
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
endif()

# The unit tests target.
add_executable(EndlessSkyTests EXCLUDE_FROM_ALL)
add_subdirectory(tests/unit)

# CTest integration
include(CTest)
include(Catch)
catch_discover_tests(EndlessSkyTests
	PROPERTIES LABELS unit)
catch_discover_tests(EndlessSkyTests
	TEST_SPEC [!benchmark]
	PROPERTIES LABELS benchmark)

# Integration tests.
if(UNIX AND NOT APPLE)
	add_custom_target(IntegrationTests
		COMMAND ${CMAKE_COMMAND} -DES=$<TARGET_FILE:EndlessSky>
			-DBINARY_PATH="${CMAKE_CURRENT_BINARY_DIR}"
			-P ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/IntegrationTests.cmake
		DEPENDS ${INTEGRATION_TESTS} ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/IntegrationTests.cmake
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
	add_dependencies(IntegrationTests EndlessSky)
	set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_BINARY_DIR}/IntegrationTests_tests.cmake")
endif()

# The 'mingw32' lib needs to appear first.
if(MINGW)
	target_link_libraries(EndlessSky PRIVATE mingw32)
	target_link_libraries(EndlessSkyTests PRIVATE mingw32)

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
			endif()
		endforeach()
	endif()
endif()

target_link_libraries(EndlessSky PRIVATE EndlessSkyLib)
if(NOT ES_USE_SYSTEM_LIBRARIES)
	target_link_libraries(EndlessSky PRIVATE SDL2::SDL2main)
endif()
target_link_libraries(EndlessSkyTests PRIVATE EndlessSkyLib)

# Choose the correct libraries for the target platform. If we're building without vcpkg link
# the corresponding system libraries as well.
if(ES_USE_SYSTEM_LIBRARIES)
	target_link_libraries(EndlessSky PRIVATE ${SDL2} ${PNG} ${JPEG} ${OpenAL} pthread)
	if(NOT ES_STEAM)
		target_link_libraries(EndlessSky PRIVATE ${LIBMAD})
	endif()
else()
	target_link_libraries(EndlessSky PRIVATE SDL2::SDL2 PNG::PNG ${JPEG_LIBRARIES} OpenAL::OpenAL)
	target_include_directories(EndlessSkyLib PUBLIC ${JPEG_INCLUDE_DIR})
endif()
if(NOT ES_USE_SYSTEM_LIBRARIES OR ES_STEAM)
	target_link_libraries(EndlessSky PRIVATE
		$<IF:$<CONFIG:Debug>,${LIBMAD_LIB_DEBUG},${LIBMAD_LIB_RELEASE}>)
	target_include_directories(EndlessSkyLib PUBLIC ${LIBMAD_INCLUDE_DIR})
endif()
target_link_libraries(EndlessSkyTests PRIVATE Catch2::Catch2)

if(WIN32)
	target_link_libraries(EndlessSkyLib PUBLIC rpcrt4 Winmm)
else()
	if(APPLE)
		# Use the system uuid library.
		find_library(UUID_LIB NAMES System PATHS /lib /usr/lib /usr/local/lib)
		find_path(UUID_INCLUDE uuid/uuid.h /usr/local/include /usr/include)
		target_link_libraries(EndlessSkyLib PUBLIC ${UUID_LIB})
		target_include_directories(EndlessSkyLib PUBLIC ${UUID_INCLUDE})
	elseif(ES_USE_SYSTEM_LIBRARIES)
		find_library(UUID_LIB uuid)
		target_link_libraries(EndlessSkyLib PUBLIC ${UUID_LIB})
	else()
		find_package(unofficial-libuuid CONFIG REQUIRED)
		target_link_libraries(EndlessSkyLib PUBLIC unofficial::UUID::uuid pthread)
	endif()
endif()

# Use OpenGL or OpenGL ES.
if(ES_GLES AND UNIX AND NOT APPLE)
	target_link_libraries(EndlessSky PRIVATE GLESv2)
	target_compile_definitions(EndlessSkyLib PUBLIC ES_GLES)
else()
	if(WIN32)
		target_link_libraries(EndlessSky PRIVATE GLEW::GLEW)
	elseif(APPLE)
		find_library(OPENGL OpenGL)
		target_link_libraries(EndlessSky PRIVATE ${OPENGL})
		# Apple deprecated OpenGL in MacOS 10.14, but we don't care.
		target_compile_definitions(EndlessSkyLib PUBLIC GL_SILENCE_DEPRECATION)
	elseif(ES_USE_SYSTEM_LIBRARIES)
		target_link_libraries(EndlessSky PRIVATE GL ${GLEW})
	else()
		target_link_libraries(EndlessSky PRIVATE GL GLEW::GLEW)
	endif()
endif()

# Set the appropriate compiler flags.
if(MSVC)
	target_compile_options(EndlessSkyLib PUBLIC "/W3" "/permissive-" "/Gm-" "/sdl" "/GR-" "/MP" "/analyze-"
		"-Wno-nonportable-include-path" "$<$<CONFIG:Release>:/Gy;/WX;/Oi>")
	target_compile_definitions(EndlessSkyLib PUBLIC "_UNICODE" "UNICODE" "$<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>")
else()
	target_compile_options(EndlessSkyLib PUBLIC "-Wall" "-fno-rtti" "$<$<CONFIG:Release>:-Werror>")
endif()

# Various helpful options for IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EndlessSky)

# Installation configurations.
if(APPLE)
elseif(WIN32)

elseif(UNIX)
	# Use lowercase target name on Linux.
	set_target_properties(EndlessSky PROPERTIES OUTPUT_NAME "endless-sky")
	set_target_properties(EndlessSkyTests PROPERTIES OUTPUT_NAME "endless-sky-tests")

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

	# Install the data files.
	install(DIRECTORY data DESTINATION share/games/endless-sky)
	install(DIRECTORY images DESTINATION share/games/endless-sky)
	install(DIRECTORY sounds DESTINATION share/games/endless-sky)
	install(FILES credits.txt DESTINATION share/games/endless-sky)
	install(FILES keys.txt DESTINATION share/games/endless-sky)
endif()
