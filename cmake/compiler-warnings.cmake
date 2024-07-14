set(compiler_is_clang "$<OR:$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:Clang>>")
set(compiler_is_gnu "$<CXX_COMPILER_ID:GNU>")
set(compiler_is_msvc "$<CXX_COMPILER_ID:MSVC>")

target_compile_options(${PROJECT_NAME}
    PRIVATE
        # MSVC only
        $<${compiler_is_msvc}:
            /W4
		>

        # Clang and GNU
        $<${compiler_is_clang}:
            -Wall
            -Wcast-align
            -Wctor-dtor-privacy
            #-Wextra
            -Wformat=2
            -Wnon-virtual-dtor
            -Wnull-dereference
            -Woverloaded-virtual
            #-Wpedantic
            #-Wshadow
            #-Wunused
            #-Wwrite-strings

            # Disable warnings which bleed through from godot-cpp's macros.
            -Wno-switch-default
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
			#-Wno-unsafe-buffer-usage
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
			#-Wno-cast-function-type-strict
			-Wno-strict-prototypes
			-Wno-language-extension-token
			-Wno-unused-macros
            -Wno-covered-switch-default
            -Wno-ctad-maybe-unsupported
            -Wno-disabled-macro-expansion
            -Wimplicit-fallthrough
        >

        # GNU only
        $<${compiler_is_gnu}:
            -Wall
            -Walloc-zero
            -Wduplicated-branches
            -Wduplicated-cond
            -Wlogical-op

            -Wno-narrowing
            -Wno-duplicated-branches
            -fpermissive
        >
)

if (ENABLE_ALL_WARNINGS MATCHES ON)
    message(NOTICE "[${PROJECT_NAME}] Enabling ALL warnings")

    target_compile_options(${PROJECT_NAME}
        PRIVATE
            $<${compiler_is_msvc}:
				/Wall
			>

			$<${compiler_is_gnu}:
				-fsafe-buffer-usage-suggestions
			>

            $<$<OR:${compiler_is_clang},${compiler_is_gnu}>:
                -Weverything
                -Wno-c++98-compat
                -Wno-c++98-compat-pedantic
                -Wno-padded
            >
	)
endif()


if (ENABLE_WARNINGS_AS_ERRORS MATCHES ON)
    message(NOTICE "[${PROJECT_NAME}] Warnings being treated as errors")

    set_target_properties(${PROJECT_NAME}
      PROPERTIES
        COMPILE_WARNING_AS_ERROR ON
    )
endif()
