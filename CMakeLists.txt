cmake_minimum_required(VERSION 3.22)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

include(vcpkg-init)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(roguelike
    LANGUAGES
      C CXX
    VERSION
      0.0.1
)

option(
  AUTOFORMAT_SRC_ON_CONFIGURE
    "If enabled, clang-format will be used to format all sources in src/ during configuration"
      ON
)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" host_os)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" architecture)
set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")

file(GLOB_RECURSE
  raylib_dummy_sources
    CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/buildtrees/raylib/src/*/src/*.[hc]"
      "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/buildtrees/raylib/src/*/src/*.[hc]pp"
)

add_library(raylib_dummy EXCLUDE_FROM_ALL ${raylib_dummy_sources})

target_include_directories(raylib_dummy PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/buildtrees/raylib/src/*/src/"
)

target_compile_definitions(raylib_dummy PUBLIC
  PLATFORM_DESKTOP
)

set(raylib_VERBOSE TRUE)
find_package(argparse CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

file(GLOB_RECURSE
  roguelike_sources
    CONFIGURE_DEPENDS
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
  NOMINMAX

  $<$<CONFIG:Debug>:
    ROGUELIKE_DEBUG
    ROGUELIKE_LOGGER_ENABLED
    DEBUG_HEAP=0
  >

  $<${compiler_is_msvc}:
    ECS_TARGET_MSVC
    ECS_TARGET_WINDOWS
  >
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<${compiler_is_msvc}:
    /Zc:preprocessor
    /utf-8
    /EHsc
    /nologo

    $<$<CONFIG:Debug>:
      /MDd
    >

    $<$<CONFIG:Release>:
      /MD
      /Ox
    >

    $<$<CONFIG:RelWithDebInfo>:
      /MD
      /Ox
    >
  >
)

target_include_directories(${PROJECT_NAME}
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE argparse::argparse
  PRIVATE box2d::box2d
  PRIVATE flecs::flecs_static
  PRIVATE fmt::fmt
  PRIVATE imgui::imgui
  PRIVATE glfw
  PRIVATE raylib
  PRIVATE simdjson::simdjson
  PRIVATE Vulkan::Vulkan
  PRIVATE winmm
)

include(clang-format)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  PROPERTY VS_STARTUP_PROJECT
    ${PROJECT_NAME}
)

set_property(TARGET ${PROJECT_NAME}
  PROPERTY VS_DEBUGGER_ENVIRONMENT
    "PATH=${CMAKE_CURRENT_INSTALL_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}"
)

add_custom_command(TARGET ${PROJECT_NAME}
  POST_BUILD COMMAND
    ${CMAKE_COMMAND} -E copy_directory_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/data
      $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)

include(cmake-utils)
