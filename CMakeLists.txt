cmake_minimum_required(VERSION 3.22)
set(CMAKE_MESSAGE_LOG_LEVEL STATUS)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

include(vcpkg-init)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
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
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/sdl_mixer)

find_package(fmt CONFIG REQUIRED)
find_package(argparse CONFIG REQUIRED)
find_package(flecs CONFIG REQUIRED)
find_package(OpenGL REQUIRED COMPONENTS OpenGL)
find_package(Vulkan REQUIRED)
find_package(volk CONFIG REQUIRED)
find_package(simdjson CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)
find_package(unofficial-enet CONFIG REQUIRED)
find_package(unofficial-nana CONFIG REQUIRED)
find_package(nanobench CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Iconv)

find_path(PLF_COLONY_INCLUDE_DIRS "plf_colony.h")
find_path(PLF_HIVE_INCLUDE_DIRS "plf_hive.h")
find_path(PLF_NANOTIMER_INCLUDE_DIRS "plf_nanotimer.h")
find_path(PLF_STACK_INCLUDE_DIRS "plf_stack.h")
find_path(D3DX12_INCLUDE_DIRS "d3dx12.h")
find_path(MAGNUM_EXTRAS_INCLUDE_DIRS "Magnum/versionExtras.h")
find_path(MAGNUM_INTEGRATION_INCLUDE_DIRS "Magnum/versionIntegration.h")

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

  $<$<CONFIG:Release>:
    /Ox
  >
)

target_include_directories(${PROJECT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
  PRIVATE ${OPENGL_INCLUDE_DIR}
  PRIVATE ${D3DX12_INCLUDE_DIRS}
  PRIVATE ${PLF_STACK_INCLUDE_DIRS}
  PRIVATE ${PLF_NANOTIMER_INCLUDE_DIRS}
  PRIVATE ${PLF_HIVE_INCLUDE_DIRS}
  PRIVATE ${PLF_COLONY_INCLUDE_DIRS}
  PRIVATE ${MAGNUM_EXTRAS_INCLUDE_DIRS}
  PRIVATE ${MAGNUM_INTEGRATION_INCLUDE_DIRS}
  PRIVATE ${LIBUSB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE OpenGL::GL
  PRIVATE Vulkan::Vulkan
  PRIVATE SDL3::SDL3-static
  PRIVATE SDL3_ttf::SDL3_ttf-static
  PRIVATE SDL3_image::SDL3_image-static
  PRIVATE SDL3_mixer::SDL3_mixer-static
  PRIVATE volk::volk volk::volk_headers
  PRIVATE unofficial::nana::nana
  PRIVATE simdjson::simdjson
  PRIVATE argparse::argparse
  PRIVATE flecs::flecs_static
  PRIVATE imgui::imgui
  PRIVATE fmt::fmt
  PRIVATE Iconv::Iconv
  PRIVATE Eigen3::Eigen
  PRIVATE ${LIBUSB_LIBRARIES}
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
