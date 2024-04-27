// #pragma once

#define LITERAL(_s_) _s_

#define USE_NAMESPACE true

#if USE_NAMESPACE
  // defines the optional namespace to
  // wrap around all SDL3 C library code
  #define SDL3_NAMESPACE  SDL3_C
  #define SDL3            LITERAL(SDL3_NAMESPACE)
  // include wrapper for SDL3, only really needed
  // if the SDL3 namespace is defined above
  #define SDL_C_LIB_BEGIN namespace SDL3 {
  #define SDL_C_LIB_END   }
#else
  #define SDL3_NAMESPACE
  #define SDL3 LITERAL(SDL3_NAMESPACE)
  #define SDL_C_LIB_BEGIN
  #define SDL_C_LIB_END
#endif
