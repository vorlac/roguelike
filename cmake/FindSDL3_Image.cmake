# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#  Copyright 2019 Amine Ben Hassouna <amine.benhassouna@gmail.com>
#  Copyright 2000-2019 Kitware, Inc. and Contributors
#  All rights reserved.

#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:

#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.

#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.

#  * Neither the name of Kitware, Inc. nor the names of Contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.

#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindSDL3_image
--------------

Locate SDL3_image library

This module defines the following 'IMPORTED' target:

::

  SDL3::Image
    The SDL3_image library, if found.
    Have SDL3::Core as a link dependency.



This module will set the following variables in your project:

::

  SDL3_IMAGE_LIBRARIES, the name of the library to link against
  SDL3_IMAGE_INCLUDE_DIRS, where to find the headers
  SDL3_IMAGE_FOUND, if false, do not try to link against
  SDL3_IMAGE_VERSION_STRING - human-readable string containing the
                              version of SDL3_image



This module responds to the following cache variables:

::

  SDL3_IMAGE_PATH
    Set a custom SDL3_image Library path (default: empty)

  SDL3_IMAGE_NO_DEFAULT_PATH
    Disable search SDL3_image Library in default path.
      If SDL3_IMAGE_PATH (default: ON)
      Else (default: OFF)

  SDL3_IMAGE_INCLUDE_DIR
    SDL3_image headers path.

  SDL3_IMAGE_LIBRARY
    SDL3_image Library (.dll, .so, .a, etc) path.


Additional Note: If you see an empty SDL3_IMAGE_LIBRARY in your project
configuration, it means CMake did not find your SDL3_image library
(SDL3_image.dll, libsdl2_image.so, etc). Set SDL3_IMAGE_LIBRARY to point
to your SDL3_image library, and  configure again. This value is used to
generate the final SDL3_IMAGE_LIBRARIES variable and the SDL3::Image target,
but when this value is unset, SDL3_IMAGE_LIBRARIES and SDL3::Image does not
get created.


$SDL3IMAGEDIR is an environment variable that would correspond to the
./configure --prefix=$SDL3IMAGEDIR used in building SDL3_image.

$SDL3DIR is an environment variable that would correspond to the
./configure --prefix=$SDL3DIR used in building SDL3.



Created by Amine Ben Hassouna:
  Adapt FindSDL_image.cmake to SDL3_image (FindSDL3_image.cmake).
  Add cache variables for more flexibility:
    SDL3_IMAGE_PATH, SDL3_IMAGE_NO_DEFAULT_PATH (for details, see doc above).
  Add SDL3 as a required dependency.
  Modernize the FindSDL3_image.cmake module by creating a specific target:
    SDL3::Image (for details, see doc above).

Original FindSDL_image.cmake module:
  Created by Eric Wing.  This was influenced by the FindSDL.cmake
  module, but with modifications to recognize OS X frameworks and
  additional Unix paths (FreeBSD, etc).
#]=======================================================================]

# SDL3 Library required
find_package(SDL3 QUIET)
if(NOT SDL3_FOUND)
  set(SDL3_IMAGE_SDL3_NOT_FOUND "Could NOT find SDL3 (SDL3 is required by SDL3_image).")
  if(SDL3_image_FIND_REQUIRED)
    message(FATAL_ERROR ${SDL3_IMAGE_SDL3_NOT_FOUND})
  else()
      if(NOT SDL3_image_FIND_QUIETLY)
        message(STATUS ${SDL3_IMAGE_SDL3_NOT_FOUND})
      endif()
    return()
  endif()
  unset(SDL3_IMAGE_SDL3_NOT_FOUND)
endif()


# Define options for searching SDL3_image Library in a custom path

set(SDL3_IMAGE_PATH "" CACHE STRING "Custom SDL3_image Library path")

set(_SDL3_IMAGE_NO_DEFAULT_PATH OFF)
if(SDL3_IMAGE_PATH)
  set(_SDL3_IMAGE_NO_DEFAULT_PATH ON)
endif()

set(SDL3_IMAGE_NO_DEFAULT_PATH ${_SDL3_IMAGE_NO_DEFAULT_PATH}
    CACHE BOOL "Disable search SDL3_image Library in default path")
unset(_SDL3_IMAGE_NO_DEFAULT_PATH)

set(SDL3_IMAGE_NO_DEFAULT_PATH_CMD)
if(SDL3_IMAGE_NO_DEFAULT_PATH)
  set(SDL3_IMAGE_NO_DEFAULT_PATH_CMD NO_DEFAULT_PATH)
endif()

# Search for the SDL3_image include directory
find_path(SDL3_IMAGE_INCLUDE_DIR SDL_image.h
  HINTS
    ENV SDL3IMAGEDIR
    ENV SDL3DIR
    ${SDL3_IMAGE_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES SDL3
                # path suffixes to search inside ENV{SDL3DIR}
                # and ENV{SDL3IMAGEDIR}
                include/SDL3 include
  PATHS ${SDL3_IMAGE_PATH}
  DOC "Where the SDL3_image headers can be found"
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

# Search for the SDL3_image library
find_library(SDL3_IMAGE_LIBRARY
  NAMES SDL3_image SDL3_image-static
  HINTS
    ENV SDL3IMAGEDIR
    ENV SDL3DIR
    ${SDL3_IMAGE_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
  PATHS ${SDL3_IMAGE_PATH}
  DOC "Where the SDL3_image Library can be found"
)

# Read SDL3_image version
if(SDL3_IMAGE_INCLUDE_DIR AND EXISTS "${SDL3_IMAGE_INCLUDE_DIR}/SDL_image.h")
  file(STRINGS "${SDL3_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL3_IMAGE_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL3_IMAGE_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_IMAGE_INCLUDE_DIR}/SDL_image.h" SDL3_IMAGE_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_IMAGE_VERSION_MAJOR "${SDL3_IMAGE_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_IMAGE_VERSION_MINOR "${SDL3_IMAGE_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_IMAGE_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL3_IMAGE_VERSION_PATCH "${SDL3_IMAGE_VERSION_PATCH_LINE}")
  set(SDL3_IMAGE_VERSION_STRING ${SDL3_IMAGE_VERSION_MAJOR}.${SDL3_IMAGE_VERSION_MINOR}.${SDL3_IMAGE_VERSION_PATCH})
  unset(SDL3_IMAGE_VERSION_MAJOR_LINE)
  unset(SDL3_IMAGE_VERSION_MINOR_LINE)
  unset(SDL3_IMAGE_VERSION_PATCH_LINE)
  unset(SDL3_IMAGE_VERSION_MAJOR)
  unset(SDL3_IMAGE_VERSION_MINOR)
  unset(SDL3_IMAGE_VERSION_PATCH)
endif()

set(SDL3_IMAGE_LIBRARIES ${SDL3_IMAGE_LIBRARY})
set(SDL3_IMAGE_INCLUDE_DIRS ${SDL3_IMAGE_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL3_image
                                  REQUIRED_VARS SDL3_IMAGE_LIBRARIES SDL3_IMAGE_INCLUDE_DIRS
                                  VERSION_VAR SDL3_IMAGE_VERSION_STRING)


mark_as_advanced(SDL3_IMAGE_PATH
                 SDL3_IMAGE_NO_DEFAULT_PATH
                 SDL3_IMAGE_LIBRARY
                 SDL3_IMAGE_INCLUDE_DIR)


if(SDL3_IMAGE_FOUND)

  # SDL3::Image target
  if(SDL3_IMAGE_LIBRARY AND NOT TARGET SDL3::Image)
    add_library(SDL3::Image UNKNOWN IMPORTED)
    set_target_properties(SDL3::Image PROPERTIES
                          IMPORTED_LOCATION "${SDL3_IMAGE_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL3_IMAGE_INCLUDE_DIR}"
                          INTERFACE_LINK_LIBRARIES SDL3::Core)
  endif()
endif()