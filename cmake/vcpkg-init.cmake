# entityx submodule init/update
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/entityx/entityx")
    message(NOTICE "entityx submodule sources not found")
    message(NOTICE "initializing/updating the entityx submodule...")
    execute_process(
        COMMAND git submodule update --init extern/entityx
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# VCPKG submodule init/update
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/ports")
    message(NOTICE "VCPKG package manager sources not found")
    message(NOTICE "initializing/updating the vcpkg submodule...")
    execute_process(
        COMMAND git submodule update --init extern/vcpkg
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY
    )
endif()

# VCPKG toolchain file
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(toolchain_file_path "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/scripts/buildsystems/vcpkg.cmake")
    if (EXISTS "${toolchain_file_path}")
        set(CMAKE_TOOLCHAIN_FILE "${toolchain_file_path}")
    else()
        message(WARNING "VCPKG toolchain file not found: ${toolchain_file_path}")
    endif()
endif()

# =======================================================================
# VCPKG bootstrap / initialization.
# =======================================================================
if(WIN32)
    set(vcpkg_executable "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/vcpkg.exe")
else()
    set(vcpkg_executable "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/vcpkg")
endif()

if(EXISTS "${vcpkg_executable}")
    message(NOTICE "Found VCPKG Executable: ${vcpkg_executable}")
else()
    message(NOTICE "Could not find VCPKG Executable: ${vcpkg_executable}")
    message(NOTICE "Calling VCPKG bootstrap scripts.")

    if(WIN32)
        execute_process(
            COMMAND powershell -c "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/bootstrap-vcpkg.bat"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_ERROR_IS_FATAL ANY
        )
    elseif(UNIX)
        execute_process(
            COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/extern/vcpkg/bootstrap-vcpkg.sh"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMAND_ERROR_IS_FATAL ANY
        )
    endif()

    # fail out if vcpkg isn't found after setup
    if(NOT EXISTS "${vcpkg_executable}")
        message(FATAL_ERROR "ERROR: '${vcpkg_executable}' not found!")
    endif()
endif()
