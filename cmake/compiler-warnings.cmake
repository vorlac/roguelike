string(TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPERCASE)

if (NOT MSVC)
    option(${PROJECT_NAME_UPPERCASE}_WARN_EVERYTHING "Turn on all warnings (not recommended - used for lib development)" OFF)
endif()

option(${PROJECT_NAME_UPPERCASE}_WARNING_AS_ERROR "Treat warnings as errors" ON)

# Add warnings based on compiler
# Set some helper variables for readability
set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")

target_compile_options(${PROJECT_NAME}
    PRIVATE
        # MSVC only
        $<${compiler_is_msvc}:
            # Enable all warnings
            #/Wall
            /W4

            # Treat all warning as errors
            /WX 

            # Disable warnings which bleed through from godot-cpp's macros.
            /wd4514 # unreferenced inline function has been removed
            /wd4866 # compiler may not enforce left-to-right evaluation order for call
            /wd4189 # local variable is initialized but not referenced

            # coming from argparse...
            /wd5045 # compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
            # sdl/tests/data/images.hpp
			/wd4125 # decimal digit terminates octal escape sequence
			/wd4100 # formal parameter unused
		>

        # Clang and GNU
        $<$<OR:${compiler_is_clang},${compiler_is_gnu}>:
            -Wall
            -Wcast-align
            -Wctor-dtor-privacy
            -Wextra
            -Wformat=2
            -Wnon-virtual-dtor
            -Wnull-dereference
            -Woverloaded-virtual
            -Wpedantic
            -Wshadow
            -Wunused
            -Wwrite-strings

            # Disable warnings which bleed through from godot-cpp's macros.
            -Wno-unused-parameter
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-pre-c++20-compat-pedantic
            -Wno-exit-time-destructors
            -Wno-global-constructors
            -Wno-missing-prototypes
            -Wno-c++20-extensions
            -Wno-c++20-compat
			-Wno-double-promotion
			-Wno-invalid-constexpr
			-Wno-unused-variable
			-Wno-uninitialized
			-Wno-unsafe-buffer-usage
			-Wno-zero-as-null-pointer-constant
			-Wno-format-security
			-Wno-reserved-macro-identifier
			-Wno-old-style-cast
			-Wno-microsoft-enum-value
            -Wno-documentation
			-Wno-documentation-unknown-command
			-Wno-sign-conversion
			-Wno-switch-enum
			-Wno-nonportable-system-include-path # #include <windows.h> in SDL3 (SDL_syswm.h)
			-Wno-unused-but-set-variable # test_renderer.hpp
			-Wno-unused-function # test_renderer.hpp
			-Wno-reserved-identifier
			-Wno-implicit-float-conversion
        >

        # Clang only
        $<${compiler_is_clang}:
            -Wimplicit-fallthrough

        >

        # GNU only
        $<${compiler_is_gnu}:
            -Walloc-zero
            -Wduplicated-branches
            -Wduplicated-cond
            -Wlogical-op
        >
)

# Turn on (almost) all warnings on Clang, Apple Clang, and GNU.
# Useful for internal development, but too noisy for general development.
function(set_warn_everything)
    message(STATUS "[${PROJECT_NAME}] Turning on (almost) all warnings")

    target_compile_options(${PROJECT_NAME}
        PRIVATE
            # Clang and GNU
            $<$<OR:${compiler_is_clang},${compiler_is_gnu}>:
                -Weverything
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-padded
				# prints extra hardening suggestions for unsafe buffer access
				-fsafe-buffer-usage-suggestions
            >
   )
endfunction()

if (NOT MSVC AND ${PROJECT_NAME_UPPERCASE}_WARN_EVERYTHING)
    set_warn_everything()
endif()

# Treat warnings as errors
function(set_warning_as_error)
    message(STATUS "[${PROJECT_NAME}] Treating warnings as errors")

    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
        set_target_properties(${PROJECT_NAME}
            PROPERTIES
                COMPILE_WARNING_AS_ERROR ON
       )
    else()
        target_compile_options(${PROJECT_NAME}
            PRIVATE
            $<${compiler_is_msvc}:/WX>
            $<$<OR:${compiler_is_clang},${compiler_is_gnu}>:-Werror>
       )
    endif()
endfunction()

if (${PROJECT_NAME_UPPERCASE}_WARNING_AS_ERROR)
    set_warning_as_error()
endif()
