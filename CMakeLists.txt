cmake_minimum_required(VERSION 3.22)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

option(
  ENABLE_CCACHE
    "Enable build intermediate file caching"
      OFF
)


option(
  AUTOFORMAT_SRC_ON_CONFIGURE
    "If enabled, clang-format will be used to format all sources in src/ during configuration"
      ON
)

option(
  ASAN_ENABLED
    "Enables address sanitizer"
	  ON
)

include(vcpkg-init)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_DIRECTORY_LABELS ON)

project(roguelike
  LANGUAGES
    C CXX
  VERSION
    0.0.1
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" host_os)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" architecture)
# set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")

file(GLOB_RECURSE
  roguelike_sources
    CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp"
      "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/*.[hc]"
)

add_executable(${PROJECT_NAME}
  ${roguelike_sources}
)

include(compiler-warnings)

if (ASAN_ENABLED MATCHES ON)
	string(REGEX REPLACE "/RTC(su|[1su])" "" 
		CMAKE_CXX_FLAGS 
	      "${CMAKE_CXX_FLAGS}"
	)

	target_compile_options(${PROJECT_NAME} PUBLIC 
		$<${compiler_is_msvc}:
			/fsanitize=address
		>
		$<$<NOT:${compiler_is_msvc}>:
			-fsanitize=address
		>
	)

	target_compile_definitions(${PROJECT_NAME} PUBLIC
		$<${compiler_is_msvc}:
			_DISABLE_VECTOR_ANNOTATION
			_DISABLE_STRING_ANNOTATION
		>
	)

	target_link_options(${PROJECT_NAME} PUBLIC
		$<${compiler_is_msvc}:
			/INCREMENTAL:NO
			/INFERASANLIBS
			/MTD
		>
		$<$<NOT:${compiler_is_msvc}>:
			-fsanitize=address
			-fno-omit-frame-pointer
			-incremental=no
		>
	)

	target_link_libraries(${PROJECT_NAME} PUBLIC
		"C:\\Program Files\\LLVM\\lib\\clang\\17\\lib\\windows\\clang_rt.asan_static-x86_64.lib"
	)

else()
	set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
		"$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,
			$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,
			$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>
		>"
	)
endif()

target_compile_features(${PROJECT_NAME}
  PRIVATE
    cxx_std_23
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  NOMINMAX
  KHRONOS_STATIC
  ROGUELIKE_TESTS_ENABLED

  $<$<CONFIG:Debug>:
    DEBUG_HEAP=0
  >
)

target_compile_options(${PROJECT_NAME} PUBLIC
  $<${compiler_is_msvc}:
      /std:c++latest
      /Zc:preprocessor
      /utf-8
      /EHsc
      /nologo

	    $<$<CONFIG:Debug>:
        # /Z7 # embed debug info into .o files instead of pdb (for ccache)
        # /ZI # enable edit and continue
      >

      $<$<CONFIG:Release>:
        /Ox
      >
    >

    $<${compiler_is_gnu}:
      -std=c++23
    >
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<${compiler_is_msvc}:
		  /LTCG
	  >
    $<$<NOT:${compiler_is_msvc}>:
      -static-libgcc
      -static-libstdc++
      -Wl,-R,'$$ORIGIN'
    >
)

set(SDL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)

if (ASAN_ENABLED MATCHES ON)
	set(SDL_LIBC OFF)
	set(SDL_STATIC ON)
	set(SDL_SHARED OFF)
	set(FORCE_STATIC_VCRT OFF)
	set(BUILD_SHARED_LIBS OFF)
else()
	set(SDL_LIBC OFF)
	set(SDL_STATIC ON)
	set(SDL_SHARED OFF)
	set(FORCE_STATIC_VCRT OFF)
	set(BUILD_SHARED_LIBS OFF)
endif()


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_ttf)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_image)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_mixer)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(libusb CONFIG REQUIRED)

target_include_directories(${PROJECT_NAME}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include"
  SYSTEM "${LIBUSB_INCLUDE_DIRS}"
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE ${LIBUSB_LIBRARIES}
  PRIVATE SDL3::SDL3-static
  PRIVATE simdjson::simdjson
  PRIVATE argparse::argparse
  PRIVATE flecs::flecs_static
  PRIVATE imgui::imgui
  PRIVATE glm::glm
  PRIVATE fmt::fmt
)

if (ENABLE_CCACHE MATCHES ON)
	include(ccache)
endif()

if (AUTOFORMAT_SRC_ON_CONFIGURE MATCHES ON)
	include(clang-format)
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT
    ${PROJECT_NAME}
)

set_property(TARGET ${PROJECT_NAME}
  PROPERTY VS_DEBUGGER_ENVIRONMENT
    "PATH=${CMAKE_CURRENT_INSTALL_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
)

# permissions fix to allow this on windows:
# 1. Open gpedit.msc
# 2. Computer Configuration
#      => Windows Settings 
#      => Security Settings 
#      => Local Policies 
#      => User Rights Assignment 
#      => Create symbolic links
# 3. Add username, then reboot machine
add_custom_command(TARGET ${PROJECT_NAME}
  PRE_BUILD COMMAND
    ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_CURRENT_SOURCE_DIR}/data
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)

include(cmake-utils)
