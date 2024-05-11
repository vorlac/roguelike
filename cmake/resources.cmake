cmake_minimum_required (VERSION 3.22)


# for each binary file:
#   1. Get filename
#   2. Replace filename spaces & extension separator for C compatibility
#   3. Convert to lower case
#   4. Read hex data from file
#   5. Convert hex data for C compatibility
#   6. Append data to c file
#   7. Append extern definitions to h file


# output directory
set(binary_resource_dir "${CMAKE_CURRENT_BINARY_DIR}/resources")
file(MAKE_DIRECTORY "${binary_resource_dir}")

# output files
set(shaders_hpp "${binary_resource_dir}/shaders.hpp")
set(shaders_cpp "${binary_resource_dir}/shaders.cpp")
set(fonts_hpp "${binary_resource_dir}/fonts.hpp")
set(fonts_cpp "${binary_resource_dir}/fonts.cpp")



# ======================================= #
#                 fonts                   #
# ======================================= #

file(GLOB font_resources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/data/fonts/*.ttf"
)

# Concatenate resource files
# into a comma separated string
string(REGEX REPLACE "([^\\]|^);" "\\1,"
  font_resources_string "${font_resources}"
)
string(REGEX REPLACE "[\\](.)" "\\1" 
  font_resources_string "${font_resources_string}"
)
string(REPLACE "," ";" 
  fonts_list ${font_resources_string}
)


file(WRITE ${fonts_hpp}
  "#include <stdint.h>\n\n"
)
file(WRITE ${fonts_cpp}
  "#include <stdint.h>\n\n"
  "#include \"resources/fonts.hpp\"\n\n"
)

message("\n\n===== ${PROJECT_NAME} resources =====")
message("\nPacking ttf hexdump into ${fonts_cpp}:")

foreach(bin ${fonts_list})
    string(REGEX MATCH "([^/]+)$" filename ${bin})
    message("> resources/${filename}")

    string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
    string(TOLOWER ${filename} filename)
    file(READ ${bin} filedata HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata "${filedata}")

    file(APPEND ${fonts_hpp} "extern const uint8_t ${filename}[];\n")
    file(APPEND ${fonts_hpp} "extern const uint32_t ${filename}_size;\n\n")

    file(APPEND ${fonts_cpp} "const uint8_t ${filename}[] = {${filedata}0x00};\n")
    file(APPEND ${fonts_cpp} "const uint32_t ${filename}_size = sizeof(${filename}) - 1;\n\n")
endforeach()


# ======================================= #
#                shaders                  #
# ======================================= #


file(GLOB shader_resources CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/gfx/gl/shaders/*.glsl"
)

file(WRITE ${shaders_cpp}
    "#include <stdint.h>\n\n"
    "#include \"resources/shaders.hpp\"\n\n"
)
file(WRITE ${shaders_hpp}
    "#include <stdint.h>\n\n"
)

message("\n\n===== ${PROJECT_NAME} shaders =====")
message("\nPacking glsl hexdump into ${shaders_hpp}:")

foreach(bin ${shader_resources})
    string(REGEX MATCH "([^/]+)$" filename ${bin})
    message("> resources/${filename}")

    string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
    string(TOLOWER ${filename} filename)
    file(READ ${bin} filedata HEX)
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})

    file(APPEND ${shaders_hpp} "extern const uint8_t ${filename}[];\n")
    file(APPEND ${shaders_hpp} "extern const uint32_t ${filename}_size;\n\n")
    file(APPEND ${shaders_cpp} "const uint8_t ${filename}[] = {${filedata}};\n")
    file(APPEND ${shaders_cpp} "const uint32_t ${filename}_size = sizeof(${filename});\n\n")
endforeach()
