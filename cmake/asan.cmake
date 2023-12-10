
if (ASAN_ENABLED MATCHES ON)
	string(REGEX REPLACE "/RTC(su|[1su])" ""
		CMAKE_CXX_FLAGS
	      "${CMAKE_CXX_FLAGS}"
	)

	target_compile_options(${PROJECT_NAME} PUBLIC
		$<${compiler_is_msvc}:
			/fsanitize=address
		>
		$<$<NOT:${compiler_is_msvc}>:
			-fsanitize=address
		>
	)

	target_compile_definitions(${PROJECT_NAME} PUBLIC
		$<${compiler_is_msvc}:
			_DISABLE_VECTOR_ANNOTATION
			_DISABLE_STRING_ANNOTATION
		>
	)

	target_link_options(${PROJECT_NAME} PUBLIC
		$<${compiler_is_msvc}:
			/INCREMENTAL:NO
			/INFERASANLIBS
			/MTD
		>
		$<$<NOT:${compiler_is_msvc}>:
			-fsanitize=address
			-fno-omit-frame-pointer
			-incremental=no
		>
	)

	target_link_libraries(${PROJECT_NAME} PUBLIC
		"C:\\Program Files\\LLVM\\lib\\clang\\17\\lib\\windows\\clang_rt.asan_static-x86_64.lib"
	)
endif()

if (ASAN_ENABLED MATCHES ON)
	set(SDL_LIBC OFF)
	set(SDL_STATIC ON)
	set(SDL_SHARED OFF)
	set(FORCE_STATIC_VCRT OFF)
	set(BUILD_SHARED_LIBS OFF)
endif()
