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
     OFF
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
set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")

set(GLFW_INSTALL OFF)
find_package(Eigen3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)

set(SDL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)

set(SDL_LIBC OFF)
set(SDL_VULKAN ON)

set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(FORCE_STATIC_VCRT OFF)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_ttf)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_image)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_mixer)

#set(NANOGUI_BUILD_GLAD OFF)
#set(NANOGUI_BUILD_GLFW OFF)
#set(NANOGUI_BUILD_PYTHON OFF)
#set(NANOGUI_BUILD_SHARED OFF)
#set(NANOGUI_BUILD_EXAMPLES OFF)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/nanogui-sdl)

file(GLOB_RECURSE
  roguelike_sources
    CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/*.[hc]"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp"
)

add_executable(${PROJECT_NAME}
  ${roguelike_sources}
)

include(compiler-warnings)

target_compile_features(${PROJECT_NAME} PRIVATE
  PRIVATE
    cxx_std_23
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  ROGUELIKE_TESTS_ENABLED
  KHRONOS_STATIC
  NOMINMAX

  $<${compiler_is_msvc}:
      _CRT_SECURE_NO_WARNINGS

      $<$<CONFIG:Debug>:
        _NO_DEBUG_HEAP=1
      >
  >
)

target_compile_options(${PROJECT_NAME} PRIVATE
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
          /O3 # maximize speed
          #/Ox # enable most speed optimizations
          #/Ot # favor fast code
          #/Ob2 # inline function expansion
          /Oi # generate intrinsics
          /Oy # frame pointer omission
          /Gw # optimize global data
          #/GR- # disable RTTI
          #/EHa- # disable exception handling
          /GS- # disable buffer security checks
          /GA # optimize for windows application
          /GL # whole program optimization
          #/Gr # __vectorcall calling convention, Gr for __fastcall
          /fp:fast # prefer fast over accurate floating point math
          /favor:INTEL64
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

target_link_options(${PROJECT_NAME} PRIVATE
    $<${compiler_is_msvc}:
        # /verbose

        $<$<CONFIG:Release>:
            /machine:x64
            /ltcg # link time code gen
            /opt:ref # eliminate unreferenced functions/data
            # /FASTGENPROFILE # generate optimization profile
        >

        $<$<CONFIG:Debug>:
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
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include"
)

# TODO: figure out how to get rid of this
#target_include_directories(nanogui PRIVATE
#  # needed so nanogui can find GLFW headers
#  SYSTEM "${PACKAGE_PREFIX_DIR}/include"
#  # needed so nanogui can find glad/gl.h header
#  SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/include"
#)


target_link_libraries(${PROJECT_NAME}
  PRIVATE SDL3::SDL3-static
  PRIVATE SDL3_ttf::SDL3_ttf-static
  PRIVATE SDL3_image::SDL3_image-static
  PRIVATE simdjson::simdjson
  PRIVATE argparse::argparse
  PRIVATE flecs::flecs_static
  PRIVATE Eigen3::Eigen
  PRIVATE glfw
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

set_property(
  TARGET ${PROJECT_NAME}
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
      ${CMAKE_CURRENT_SOURCE_DIR}/src/gl/shaders
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)


include(cmake-utils)
