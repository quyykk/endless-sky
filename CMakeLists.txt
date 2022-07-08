# The lowest supported CMake is the one from Ubuntu 18, CMake 3.10.
cmake_minimum_required(VERSION 3.10...3.23)
if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# Some options.
option(GLES "Build the game with OpenGL ES on Linux" OFF)

# Use C++11 without any compiler specific extensions.
set(CMAKE_CXX_STANDARD 11 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# The lowest macOS we support is 10.9.
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
# Use LTO for Release builds only.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

# Support only Debug and Release configurations, with Debug being the default.
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
endif()

# Add our custom triplets for vcpkg.
set(VCPKG_OVERLAY_TRIPLETS "${CMAKE_CURRENT_SOURCE_DIR}/custom-triplets")

# Setup vcpkg.
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
	CACHE STRING "Vcpkg toolchain file")

project("Endless Sky" VERSION 0.9.15
	DESCRIPTION "Space exploration, trading, and combat game."
	LANGUAGES CXX)

find_package(SDL2 CONFIG REQUIRED)
find_package(OpenAL CONFIG REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(GLEW REQUIRED)
# libmad doesn't provide native cmake support.
find_path(LIBMAD_INCLUDE_DIR mad.h)
find_library(LIBMAD_LIB_DEBUG NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH)
find_library(LIBMAD_LIB_RELEASE NAMES mad libmad NAMES_PER_DIR PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH)

# Create library target. The source files are populated inside source/CMakeLists.txt.
add_library(EndlessSkyLib STATIC "")
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
		    # Get the relative path from the root folder to every file.
		    file(RELATIVE_PATH NEW_FILE "${CMAKE_CURRENT_SOURCE_DIR}" ${FILE})

			# Resources belong under Resources/.
		    set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${NEW_FILE}")

			# IDE support for XCode.
		    source_group("Resources/${NEW_FILE}" FILES "${FILE}")
		endforeach()
	endforeach()
	# Now do the same to standone files.
	# TODO: Make this a CMake function to avoid duplication.
	foreach(file "license.txt" "keys.txt" "icon.png" "credits.txt" "copyright" "changelog")
		target_sources(EndlessSky PRIVATE ${file})
		set_property(SOURCE ${file} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${file}")
		source_group("Resources/${file}" FILES "${file}")
	endforeach()

	# Add custom plist and iconset to bundle.
	target_sources(EndlessSky PRIVATE ${CMAKE_CURRENT_LIST_DIR}/XCode/EndlessSky-Info.plist)
	target_sources(EndlessSky PRIVATE ${CMAKE_CURRENT_LIST_DIR}/XCode/Images.xcassets)
	set_target_properties(EndlessSky PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_LIST_DIR}/XCode/EndlessSky-Info.plist
		RESOURCE ${CMAKE_CURRENT_LIST_DIR}/XCode/Images.xcassets
		XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME endless-sky)

elseif(WIN32)
	add_executable(EndlessSky WIN32 source/main.cpp)
	target_sources(EndlessSky PRIVATE source/WinApp.rc)
else()
	add_executable(EndlessSky source/main.cpp)
endif()

# Create unit test target. The source files are populated inside tests/unit/CMakeLists.txt.
add_executable(EndlessSkyTests "")
add_subdirectory(tests/unit)

# The 'mingw32' lib needs to appear first.
if(MINGW)
	target_link_libraries(EndlessSky PRIVATE mingw32)
	target_link_libraries(EndlessSkyTests PRIVATE mingw32)
endif()
target_link_libraries(EndlessSky PRIVATE EndlessSkyLib SDL2::SDL2main)
target_link_libraries(EndlessSkyTests PRIVATE EndlessSkyLib)

# Choose the correct libraries for the target platform. If we're building without vcpkg link
# the corresponding system libraries as well.
target_link_libraries(EndlessSkyLib PUBLIC SDL2::SDL2 PNG::PNG ${JPEG_LIBRARIES} OpenAL::OpenAL
	$<IF:$<CONFIG:Debug>,${LIBMAD_LIB_DEBUG},${LIBMAD_LIB_RELEASE}>)
target_include_directories(EndlessSkyLib PUBLIC ${JPEG_INCLUDE_DIR})
target_include_directories(EndlessSkyLib PUBLIC ${LIBMAD_INCLUDE_DIR})

if(WIN32)
	target_link_libraries(EndlessSkyLib PUBLIC rpcrt4 Winmm)
else()
	if(APPLE)
		# Use the system uuid library.
		find_library(UUID_LIB NAMES System PATHS /lib /usr/lib /usr/local/lib)
		find_path(UUID_INCLUDE uuid/uuid.h /usr/local/include /usr/include)
		target_link_libraries(EndlessSkyLib PUBLIC ${UUID_LIB})
		target_include_directories(EndlessSkyLib PUBLIC ${UUID_INCLUDE})
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
	if(WIN32 OR APPLE)
		target_link_libraries(EndlessSky PUBLIC GLEW::GLEW)
	else()
		target_link_libraries(EndlessSky PUBLIC GL GLEW::GLEW)
	endif()
endif()

# Set the appropriate compiler flags.
if(MSVC)
	target_compile_options(EndlessSkyLib PUBLIC "/W3" "/permissive-" "/wd4267" "/wd4244" "/wd4804" "/wd4805"
		"/wd4305" "/wd4068" "/wd4018" "/Gm-" "/sdl" "/GR-" "/MP" "/analyze-"
		"$<$<CONFIG:Release>:/Gy;/WX;/Oi>")
	target_compile_definitions(EndlessSkyLib PUBLIC "_UNICODE" "UNICODE" "$<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>")
else()
	target_compile_options(EndlessSkyLib PUBLIC "-Wall" "-fno-rtti" "$<$<CONFIG:Release>:-Werror>")
endif()

# Various helpful options for IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EndlessSky)

# Only install on Linux.
if(UNIX AND NOT APPLE)
	# Use lowercase target name on Linux.
	set_target_properties(EndlessSky PROPERTIES OUTPUT_NAME "endless-sky")
	set_target_properties(EndlessSkyTests PROPERTIES OUTPUT_NAME "endless-sky-tests")

	# Install the binary.
	install(TARGETS EndlessSky CONFIGURATIONS Release RUNTIME DESTINATION games)

	# Install the desktop file.
	install(FILES endless-sky.desktop DESTINATION share/applications)

	# Install app center metadata.
	install(FILES endless-sky.appdata.xml DESTINATION share/metainfo)

	# Install icons, keeping track of all the paths.
	# Most Ubuntu apps supply 16, 22, 24, 32, 48, and 256, and sometimes others.
	foreach(size "16x16" "22x22" "24x24" "32x32" "48x48" "128x128" "256x256" "512x512")
		install(FILES "icons/icon_${size}.png" DESTINATION "share/icons/hicolor/${size}/apps"
			RENAME endless-sky.png)
	endforeach()

	# TODO: update icon cache as well?
	# TODO: Install the man page.

	# Install the data files.
	install(DIRECTORY data DESTINATION share/games/endless-sky)
	install(DIRECTORY images DESTINATION share/games/endless-sky)
	install(DIRECTORY sounds DESTINATION share/games/endless-sky)
	install(FILES credits.txt DESTINATION share/games/endless-sky)
	install(FILES keys.txt DESTINATION share/games/endless-sky)
endif()
