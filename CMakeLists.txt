cmake_minimum_required(VERSION 3.22)
message(STATUS "Using CMake ${CMAKE_VERSION}")

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/"
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_DEBUG_POSTFIX ".debug")

include(vcpkg-init)

project(roguelike
    LANGUAGES
      C CXX
    VERSION
      0.1.0
)

include(vcpkg-configure)

find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(raylib CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

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
        /EHsc
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
            -O3
        >
    >
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
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
    PRIVATE fmt::fmt
    PRIVATE fmt::fmt-header-only
    PRIVATE spdlog::spdlog_header_only
    PRIVATE raylib
    PRIVATE box2d::box2d
    PRIVATE EnTT::EnTT
    PRIVATE glm::glm
)

# =======================================================================
# Print configuration report
# =======================================================================

# set(CMAKE_MESSAGE_LOG_LEVEL DEBUG)
include(cmake-utils)
print_project_variables()
