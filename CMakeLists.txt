cmake_minimum_required(VERSION 3.22)

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

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" host_os)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" architecture)
set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")

file(GLOB_RECURSE
  roguelike_sources
    CONFIGURE_DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/*.[hc]pp"
)


set(raylib_VERBOSE 1)
find_package(raylib CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
  ${roguelike_sources}
)

include(compiler-warnings)

target_compile_features(${PROJECT_NAME}
  PRIVATE
    cxx_std_23
    c_std_17
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  $<$<CONFIG:Debug>:
    ROGUELIKE_DEBUG
    ROGUELIKE_LOGGER_ENABLED
  >
  NOMINMAX
)

target_compile_options(${PROJECT_NAME} PUBLIC
  $<${compiler_is_msvc}:
    /utf-8
    /Zc:preprocessor

    $<$<CONFIG:Debug>:
      /MDd
    >

    $<$<CONFIG:Release>:
      /MD
      /O2
    >
  >
)

target_include_directories(${PROJECT_NAME}
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_link_options(${PROJECT_NAME}
  PRIVATE
    $<$<NOT:${compiler_is_msvc}>:
      -static-libgcc
      -static-libstdc++
    >
)

target_link_libraries(${PROJECT_NAME} 
  PRIVATE raylib
  PRIVATE EnTT::EnTT
  PRIVATE nlohmann_json::nlohmann_json
  PRIVATE fmt::fmt
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

# set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
include(cmake-utils)
print_project_variables()
