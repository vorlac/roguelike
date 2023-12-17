#ifndef SKITY_MACROS_HPP
  #define SKITY_MACROS_HPP

  #ifdef SKITY_RELEASE
    #if defined(_MSC_VER)
      #define __declspec(dllexport)
    #else
      #define __attribute__(visibility("default"))
    #endif
  #else
    #if defined(_MSC_VER)
      #define __declspec(dllexport)
    #else
      #define
    #endif
  #endif

#endif  // SKITY_MACROS_HPP
