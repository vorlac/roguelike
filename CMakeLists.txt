cmake_minimum_required(VERSION 3.23)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

set(ENABLE_CCACHE
  OFF CACHE FORCE BOOL
    "Enables ccache"
)

set(AUTOFORMAT_SRC_ON_CONFIGURE
  ON CACHE FORCE BOOL
    "Enables clang-format to autoformat all sources within src/"
)

set(REBUILD_RESOURCES_ON_CONFIGURE
  ON CACHE FORCE BOOL
    "Enables shader (.glsl) and font (.ttf) hexdumps to be built into resource hpp/cpp files"
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

if (REBUILD_RESOURCES_ON_CONFIGURE MATCHES ON)
	include(resources)
endif()

string(TOLOWER "${CMAKE_SYSTEM_NAME}" host_os)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" architecture)
set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")

set(FORCE_STATIC_VCRT ON)
set(BUILD_SHARED_LIBS OFF)

find_package(glm CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nanobench CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)

# SDL3 build options
set(SDL_LIBC OFF)
set(SDL_VULKAN OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# SDL3 test program targets
set(SDL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl)

# option(TRACY_ENABLE "" ON)
# option(TRACY_ON_DEMAND "" ON)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/tracy)

file(GLOB_RECURSE
  roguelike_sources
  CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/gl.[hc]"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp"
)

add_executable(${PROJECT_NAME}
  ${roguelike_sources}
  ${CMAKE_CURRENT_BINARY_DIR}/resources/fonts.cpp
)

include(compiler-warnings)

target_compile_features(${PROJECT_NAME}
  PRIVATE
    cxx_std_23
)

target_compile_definitions(${PROJECT_NAME}
  PRIVATE
    ROGUELIKE_TESTS_ENABLED
	SPDLOG_NO_EXCEPTIONS
	SPDLOG_WCHAR_TO_UTF8_SUPPORT
    KHRONOS_STATIC
	NANOVG_GL3
    NOMINMAX
	UNICODE

    $<${compiler_is_msvc}:
        _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
        _CRT_SECURE_NO_WARNINGS

        $<$<CONFIG:Debug>:
			# _DISABLE_STRING_ANNOTATION=1 # needed with ASAN enabled because of fmt
			# _DISABLE_VECTOR_ANNOTATION=1 # needed with ASAN enabled because of fmt
            _NO_DEBUG_HEAP=1
        >
    >
)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<${compiler_is_msvc}:
        /std:c++latest
        /Zc:preprocessor
        /utf-8
        /EHsc
        /nologo

        $<$<CONFIG:Debug>:
			#/fsanitize=fuzzer
		    ## /fsanitize=address
            # /Z7 # embed debug info into .o files instead of pdb (for ccache)
            # /Zi # enable edit and continue
        >

        $<$<CONFIG:Release>:
            /O2 # maximize speed
            /Oi # generate intrinsics
            /Oy # frame pointer omission
            #/Gw # optimize global data
            #/GS- # disable buffer security checks
            #/GA # optimize for windows application
            /GL # whole program optimization
            #/fp:fast # prefer fast over accurate floating point math
            #/favor:INTEL64

            #/Ox # enable most speed optimizations
            #/Ot # favor fast code
            #/Ob2 # inline function expansion
            #/GR- # disable RTTI
            #/EHa- # disable exception handling
            #/Gr # __vectorcall calling convention, Gr for __fastcall
            #/order # order functions for optimal paging
        >
    >

    $<${compiler_is_clang}:
        -march=native
        -flto
        -DNDEBUG
    >

    $<${compiler_is_gnu}:
        -std=c++23
    >
)

target_link_options(${PROJECT_NAME}
  PRIVATE
    $<${compiler_is_msvc}:
        # /verbose

        $<$<CONFIG:Release>:
            # /machine:x64
            # /ltcg # link time code gen
            /opt:ref # eliminate unreferenced functions/data
            # /FASTGENPROFILE # generate optimization profile
        >

        $<$<CONFIG:Debug>:
            /opt:ref # eliminate unreferenced functions/data
			## /INFERASANLIBS
            # /natvis:<path> # link natvis into pdb
            # /libpath:dir # additional lib paths
            # /wholearchive # include all library object files in binary
        >
    >

    $<$<NOT:${compiler_is_msvc}>:
        $<$<CONFIG:Release>:
            -fomit-frame-pointer
        >

        -static-libgcc
        -static-libstdc++
        -Wl,-R,'$$ORIGIN'
    >
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
  SYSTEM
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include"
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE fmt::fmt
  PRIVATE spdlog::spdlog_header_only
  PRIVATE nanobench::nanobench
  PRIVATE SDL3::SDL3-static
  PRIVATE glm::glm
  PRIVATE cpptrace::cpptrace
)

if (ENABLE_CCACHE MATCHES ON)
  include(ccache)
endif()

if (AUTOFORMAT_SRC_ON_CONFIGURE MATCHES ON)
  include(clang-format)
endif()


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
      ${CMAKE_CURRENT_SOURCE_DIR}/src/graphics/gl/shaders
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# file(GLOB_RECURSE
#   headers_list
#     "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/*.h"
#     "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
# )
#
# target_sources(${PROJECT_NAME}
#   PUBLIC
#     FILE_SET test_headers
# 	TYPE HEADERS
# 	BASE_DIRS
# 	  "${CMAKE_CURRENT_SOURCE_DIR}/src"
# 	  "${CMAKE_CURRENT_SOURCE_DIR}/extern"
# 	FILES
# 	  ${headers_list}
# )
#
# install(TARGETS ${PROJECT_NAME}
#   FILE_SET test_headers
#   DESTINATION
#     "${CMAKE_CURRENT_SOURCE_DIR}/install_test/"
# )

include(cmake-utils)
