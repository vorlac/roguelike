cmake_minimum_required(VERSION 3.22)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

include(vcpkg-init)

set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_C_STANDARD 23)
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

include(CPack)

string(TOLOWER "${CMAKE_SYSTEM_NAME}" host_os)
string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type)
string(TOLOWER "${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" architecture)
set(CMAKE_DEBUG_POSTFIX ".${build_type}.${architecture}")

set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")


set(SDL_LIBC OFF)
set(SDL_STATIC ON)
set(SDL_SHARED OFF)
set(FORCE_STATIC_VCRT OFF)
set(SDL_VULKAN ON)
set(SDL_TESTS ON)
set(SDL_TEST_LIBRARY ON)
set(SDL3IMAGE_VENDORED OFF)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_image)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_ttf)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_mixer)

find_package(fmt CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
#find_package(FFMPEG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(Vulkan REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

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
  ROGUELIKE_TESTS_ENABLED

  $<$<CONFIG:Debug>:
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
  >
)

target_include_directories(${PROJECT_NAME}
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src"
  PRIVATE ${OPENGL_INCLUDE_DIR}
  # PRIVATE ${FFMPEG_INCLUDE_DIRS}
)

# target_link_directories(${PROJECT_NAME}
#   PRIVATE ${FFMPEG_LIBRARY_DIRS}
# )

target_link_libraries(${PROJECT_NAME}
  # PRIVATE ${FFMPEG_LIBRARIES}
  PRIVATE argparse::argparse
  PRIVATE flecs::flecs_static
  PRIVATE imgui::imgui
  PRIVATE Vulkan::Vulkan
  PRIVATE OpenGL::GL
  PRIVATE SDL3::SDL3
  PRIVATE SDL3_image::SDL3_image
  PRIVATE simdjson::simdjson
  PRIVATE fmt::fmt
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
