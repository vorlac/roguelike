
find_program(CLANG_FORMAT_PROGRAM NAMES clang-format)

if (CLANG_FORMAT_PROGRAM)
    execute_process(
        COMMAND "${CLANG_FORMAT_PROGRAM}" --version
        OUTPUT_VARIABLE CLANG_FORMAT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    message("Using clang-format: ${CLANG_FORMAT_PROGRAM} (${CLANG_FORMAT_VERSION})")

    file(GLOB_RECURSE
      format_src_list
      RELATIVE
        "${CMAKE_CURRENT_SOURCE_DIR}"
          "src/*.[hc]"
          "src/*.[hc]pp"
    )

    set(skip_stb_pattern "src/gfx/stb/stb.*")
    set(skip_ico_pattern "src/gfx/text/icons.*")

    foreach(_src_file ${format_src_list})
        unset(MATCHED_STB)
        unset(MATCHED_ICO)

        string(REGEX MATCH ${skip_stb_pattern} MATCHED_STB ${_src_file})
        string(REGEX MATCH ${skip_ico_pattern} MATCHED_ICO ${_src_file})

        if (MATCHED_STB OR MATCHED_ICO)
            message("      skipping => ${_src_file}")
        else()
            message("    formatting => ${_src_file}")
                execute_process(
                COMMAND "${CLANG_FORMAT_PROGRAM}" --style=file -i "${_src_file}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            )
        endif()
    endforeach()

    unset(CLANG_FORMAT_VERSION)
endif()
