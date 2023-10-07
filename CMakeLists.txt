cmake_minimum_required(VERSION 3.22)

project(roguelike LANGUAGES
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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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

add_executable(${PROJECT_NAME}
    ${roguelike_sources}
)

find_package(raylib CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

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

  $<$<NOT:${compiler_is_msvc}>:
    -g

    $<$<CONFIG:Debug>:
      -fno-omit-frame-pointer
      -O0
    >

    $<$<CONFIG:Release>:
      -fomit-frame-pointer
      -O3
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

# =======================================================================
# Dependency linkage
# =======================================================================

target_link_libraries(${PROJECT_NAME} 
    PRIVATE raylib
    PRIVATE EnTT::EnTT
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE winmm
)

#target_link_libraries(${PROJECT_NAME} PRIVATE argparse::argparse)
#target_link_libraries(${PROJECT_NAME} PRIVATE box2d::box2d)
#target_link_libraries(${PROJECT_NAME} PRIVATE $<IF:$<TARGET_EXISTS:flecs::flecs>,flecs::flecs,flecs::flecs_static>)
#target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
#target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
#target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)

# =======================================================================
# Print configuration report
# =======================================================================

# set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
include(cmake-utils)
print_project_variables()
