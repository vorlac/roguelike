
find_program(CCACHE_PROGRAM NAMES ccache)

if (CCACHE_PROGRAM)
    execute_process(
        COMMAND "${CCACHE_PROGRAM}" --version
        OUTPUT_VARIABLE CCACHE_VERSION
    )
    #set(CCACHE_PROGRAM "${CCACHE_PROGRAM} --show-stats"
    string(REGEX MATCH "[^\r\n]*" CCACHE_VERSION ${CCACHE_VERSION})
    message(STATUS "Using ccache: ${CCACHE_PROGRAM} (${CCACHE_VERSION})")

    
    #set(CCACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/zzzz")
    #set(CCACHE_PROGRAM "${CCACHE_PROGRAM} --show-stats")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM} --verbose")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD COMMAND "${CCACHE_PROGRAM}" --show-stats)
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND "${CCACHE_PROGRAM}" --show-stats)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ${CCACHE_PROGRAM})
    unset(CCACHE_VERSION)
endif()
