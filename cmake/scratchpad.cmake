
#
# Header install example
#

file(GLOB_RECURSE
  headers_list
    "${CMAKE_CURRENT_SOURCE_DIR}/extern/glad/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

target_sources(${PROJECT_NAME}
  PUBLIC
    FILE_SET test_headers
	TYPE HEADERS
	BASE_DIRS
	  "${CMAKE_CURRENT_SOURCE_DIR}/src"
	  "${CMAKE_CURRENT_SOURCE_DIR}/extern"
	FILES
	  ${headers_list}
)

install(TARGETS ${PROJECT_NAME}
  FILE_SET test_headers
  DESTINATION
    "${CMAKE_CURRENT_SOURCE_DIR}/install_test/"
)
