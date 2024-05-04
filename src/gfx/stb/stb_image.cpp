#include <array>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR)
  #include <cmath>  // ldexp, pow
#endif

#ifndef STBI_NO_STDIO
  #include <cstdio>
#endif

#ifndef STBI_ASSERT
  #include <cassert>
  #define STBI_ASSERT(x) assert(x)
#endif

#include "gfx/stb/stb_image.hpp"
#include "utils/numeric.hpp"

namespace rl::stb {

#if defined(STBI_ONLY_JPEG) || defined(STBI_ONLY_PNG) || defined(STBI_ONLY_BMP) || \
    defined(STBI_ONLY_TGA) || defined(STBI_ONLY_GIF) || defined(STBI_ONLY_PSD) ||  \
    defined(STBI_ONLY_HDR) || defined(STBI_ONLY_PIC) || defined(STBI_ONLY_PNM) ||  \
    defined(STBI_ONLY_ZLIB)
  #ifndef STBI_ONLY_JPEG
    #define STBI_NO_JPEG
  #endif
  #ifndef STBI_ONLY_PNG
    #define STBI_NO_PNG
  #endif
  #ifndef STBI_ONLY_BMP
    #define STBI_NO_BMP
  #endif
  #ifndef STBI_ONLY_PSD
    #define STBI_NO_PSD
  #endif
  #ifndef STBI_ONLY_TGA
    #define STBI_NO_TGA
  #endif
  #ifndef STBI_ONLY_GIF
    #define STBI_NO_GIF
  #endif
  #ifndef STBI_ONLY_HDR
    #define STBI_NO_HDR
  #endif
  #ifndef STBI_ONLY_PIC
    #define STBI_NO_PIC
  #endif
  #ifndef STBI_ONLY_PNM
    #define STBI_NO_PNM
  #endif
#endif

#if defined(STBI_NO_PNG) && !defined(STBI_SUPPORT_ZLIB) && !defined(STBI_NO_ZLIB)
  #define STBI_NO_ZLIB
#endif

#ifdef __cplusplus
  #define STBI_EXTERN extern "C"
#else
  #define STBI_EXTERN extern
#endif

#ifndef _MSC_VER
  #ifdef __cplusplus
    #define stbi_inline inline
  #else
    #define stbi_inline
  #endif
#else
  #define stbi_inline __forceinline
#endif

#ifndef STBI_NO_THREAD_LOCALS
  #if defined(__cplusplus) && __cplusplus >= 201103L
    #define STBI_THREAD_LOCAL thread_local
  #elif defined(__GNUC__) && __GNUC__ < 5
    #define STBI_THREAD_LOCAL __thread
  #elif defined(_MSC_VER)
    #define STBI_THREAD_LOCAL __declspec(thread)
  #elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
    #define STBI_THREAD_LOCAL _Thread_local
  #endif

  #ifndef STBI_THREAD_LOCAL
    #if defined(__GNUC__)
      #define STBI_THREAD_LOCAL __thread
    #endif
  #endif
#endif

#if defined(_MSC_VER) || defined(__SYMBIAN32__)
    typedef unsigned short StbiUint16;
    typedef signed short StbiInt16;
    typedef unsigned int StbiUint32;
    typedef signed int StbiInt32;
#else
    typedef uint16_t StbiUint16;
    typedef int16_t StbiInt16;
    typedef uint32_t StbiUint32;
    typedef int32_t StbiInt32;
#endif

    // should produce compiler error if size is wrong
    typedef unsigned char validate_uint32[sizeof(StbiUint32) == 4 ? 1 : -1];

#ifdef _MSC_VER
  #define STBI_NOTUSED(v) (void)(v)
#else
  #define STBI_NOTUSED(v) (void)sizeof(v)
#endif

#ifdef _MSC_VER
  #define STBI_HAS_LROTL
#endif

#ifdef STBI_HAS_LROTL
  #define stbi_lrot(x, y) _lrotl(x, y)
#else
  #define stbi_lrot(x, y) (((x) << (y)) | ((x) >> (-(y) & 31)))
#endif

#if defined(STBI_MALLOC) && defined(STBI_FREE) && \
    (defined(STBI_REALLOC) || defined(STBI_REALLOC_SIZED))
// ok
#elif !defined(STBI_MALLOC) && !defined(STBI_FREE) && !defined(STBI_REALLOC) && \
    !defined(STBI_REALLOC_SIZED)
// ok
#else
  #error \
      "Must define all or none of STBI_MALLOC, STBI_FREE, and STBI_REALLOC (or STBI_REALLOC_SIZED)."
#endif

#ifndef STBI_MALLOC
  #define STBI_MALLOC(sz)        malloc(sz)
  #define STBI_REALLOC(p, newsz) realloc(p, newsz)
  #define STBI_FREE(p)           free(p)
#endif

#ifndef STBI_REALLOC_SIZED
  #define STBI_REALLOC_SIZED(p, oldsz, newsz) STBI_REALLOC(p, newsz)
#endif

// x86/x64 detection
#if defined(__x86_64__) || defined(_M_X64)
  #define STBI_X64_TARGET
#elif defined(__i386) || defined(_M_IX86)
  #define STBI__X86_TARGET
#endif

#if defined(__GNUC__) && defined(STBI__X86_TARGET) && !defined(__SSE2__) && !defined(STBI_NO_SIMD)
  // gcc doesn't support sse2 intrinsics unless you compile with -msse2,
  // which in turn means it gets to use SSE2 everywhere. This is unfortunate,
  // but previous attempts to provide the SSE2 functions with runtime
  // detection caused numerous issues. The way architecture extensions are
  // exposed in GCC/Clang is, sadly, not really suited for one-file libs.
  // New behavior: if compiled with -msse2, we use SSE2 without any
  // detection; if not, we don't use it at all.
  #define STBI_NO_SIMD
#endif

#if defined(__MINGW32__) && defined(STBI__X86_TARGET) && \
    !defined(STBI_MINGW_ENABLE_SSE2) && !defined(STBI_NO_SIMD)
  // Note that __MINGW32__ doesn't actually mean 32-bit, so we have to avoid STBI__X64_TARGET
  //
  // 32-bit MinGW wants ESP to be 16-byte aligned, but this is not in the
  // Windows ABI and VC++ as well as Windows DLLs don't maintain that invariant.
  // As a result, enabling SSE2 on 32-bit MinGW is dangerous when not
  // simultaneously enabling "-mstackrealign".
  //
  // See https://github.com/nothings/stb/issues/81 for more information.
  //
  // So default to no SSE2 on 32-bit MinGW. If you've read this far and added
  // -mstackrealign to your build settings, feel free to #define STBI_MINGW_ENABLE_SSE2.
  #define STBI_NO_SIMD
#endif

#if !defined(STBI_NO_SIMD) && (defined(STBI__X86_TARGET) || defined(STBI__X64_TARGET))
  #define STBI_SSE2
  #include <emmintrin.h>

  #ifdef _MSC_VER

    #if _MSC_VER >= 1400   // not VC6
      #include <intrin.h>  // __cpuid

    static int stbi_cpuid3(void)
    {
        int info[4];
        __cpuid(info, 1);
        return info[3];
    }
    #else
    static int stbi__cpuid3(void)
    {
        int res;
        __asm {
      mov  eax,1
      cpuid
      mov  res,edx
        }
        return res;
    }
    #endif

    #define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name

    #if !defined(STBI_NO_JPEG) && defined(STBI_SSE2)
    static int stbi_sse2_available(void)
    {
        int info3 = stbi_cpuid3();
        return ((info3 >> 26) & 1) != 0;
    }
    #endif

  #else  // assume GCC-style if not VC++
    #define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))

    #if !defined(STBI_NO_JPEG) && defined(STBI_SSE2)
    static int stbi__sse2_available(void)
    {
        // If we're even attempting to compile this on GCC/Clang, that means
        // -msse2 is on, which means the compiler is allowed to use SSE2
        // instructions at will, and so are we.
        return 1;
    }
    #endif

  #endif
#endif

// ARM NEON
#if defined(STBI_NO_SIMD) && defined(STBI_NEON)
  #undef STBI_NEON
#endif

#ifdef STBI_NEON
  #include <arm_neon.h>
  #ifdef _MSC_VER
    #define STBI_SIMD_ALIGN(type, name) __declspec(align(16)) type name
  #else
    #define STBI_SIMD_ALIGN(type, name) type name __attribute__((aligned(16)))
  #endif
#endif

#ifndef STBI_SIMD_ALIGN
  #define STBI_SIMD_ALIGN(type, name) type name
#endif

#ifndef STBI_MAX_DIMENSIONS
  #define STBI_MAX_DIMENSIONS (1 << 24)
#endif

    ///////////////////////////////////////////////
    //
    //  stbi__context struct and start_xxx functions

    // stbi__context structure is our basic context used by all images, so it
    // contains all the IO context, plus some basic image information
    struct StbiContext
    {
        StbiUint32 img_x, img_y;
        int img_n, img_out_n;

        stbi_io_callbacks io;
        void* io_user_data;

        int read_from_callbacks;
        int buflen;
        stbi_uc buffer_start[128];
        int callback_already_read;

        stbi_uc *img_buffer, *img_buffer_end;
        stbi_uc *img_buffer_original, *img_buffer_original_end;
    };

    static void stbi_refill_buffer(StbiContext* s);

    // initialize a memory-decode context
    static void stbi_start_mem(StbiContext* s, const stbi_uc* buffer, const int len)
    {
        s->io.read = nullptr;
        s->read_from_callbacks = 0;
        s->callback_already_read = 0;
        s->img_buffer = s->img_buffer_original = const_cast<stbi_uc*>(buffer);
        s->img_buffer_end = s->img_buffer_original_end = const_cast<stbi_uc*>(buffer) + len;
    }

    // initialize a callback-based context
    static void stbi_start_callbacks(StbiContext* s, const stbi_io_callbacks* c, void* user)
    {
        s->io = *c;
        s->io_user_data = user;
        s->buflen = sizeof(s->buffer_start);
        s->read_from_callbacks = 1;
        s->callback_already_read = 0;
        s->img_buffer = s->img_buffer_original = s->buffer_start;
        stbi_refill_buffer(s);
        s->img_buffer_original_end = s->img_buffer_end;
    }

#ifndef STBI_NO_STDIO

    static int stbi_stdio_read(void* user, char* data, const int size)
    {
        return static_cast<int>(fread(data, 1, size, (FILE*)user));
    }

    static void stbi_stdio_skip(void* user, const int n)
    {
        std::fseek(static_cast<FILE*>(user), n, SEEK_CUR);
        const int ch = fgetc(static_cast<FILE*>(user)); /* have to read a byte to reset feof()'s
                                                           flag */
        if (ch != EOF)
            ungetc(ch, static_cast<FILE*>(user)); /* push byte back onto stream if valid. */
    }

    static int stbi_stdio_eof(void* user)
    {
        return feof(static_cast<FILE*>(user)) || ferror(static_cast<FILE*>(user));
    }

    static stbi_io_callbacks stbi_stdio_callbacks = {
        stbi_stdio_read,
        stbi_stdio_skip,
        stbi_stdio_eof,
    };

    static void stbi_start_file(StbiContext* s, FILE* f)
    {
        stbi_start_callbacks(s, &stbi_stdio_callbacks, (void*)f);
    }

    // static void stop_file(stbi__context *s) { }

#endif  // !STBI_NO_STDIO

    static void stbi_rewind(StbiContext* s)
    {
        // conceptually rewind SHOULD rewind to the beginning of the stream,
        // but we just rewind to the beginning of the initial buffer, because
        // we only use it after doing 'test', which only ever looks at at most 92 bytes
        s->img_buffer = s->img_buffer_original;
        s->img_buffer_end = s->img_buffer_original_end;
    }

    enum {
        STBI_ORDER_RGB,
        STBI_ORDER_BGR
    };

    struct StbiResultInfo
    {
        int bits_per_channel;
        int num_channels;
        int channel_order;
    };

#ifndef STBI_NO_JPEG
    static int stbi_jpeg_test(StbiContext* s);
    static void* stbi_jpeg_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                                const StbiResultInfo* ri);
    static int stbi_jpeg_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_PNG
    static int stbi_png_test(StbiContext* s);
    static void* stbi_png_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               StbiResultInfo* ri);
    static int stbi_png_info(StbiContext* s, int* x, int* y, int* comp);
    static int stbi_png_is16(StbiContext* s);
#endif

#ifndef STBI_NO_BMP
    static int stbi_bmp_test(StbiContext* s);
    static void* stbi_bmp_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               StbiResultInfo* ri);
    static int stbi_bmp_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_TGA
    static int stbi_tga_test(StbiContext* s);
    static void* stbi_tga_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               const StbiResultInfo* ri);
    static int stbi_tga_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_PSD
    static int stbi_psd_test(StbiContext* s);
    static void* stbi_psd_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               StbiResultInfo* ri, int bpc);
    static int stbi_psd_info(StbiContext* s, int* x, int* y, int* comp);
    static int stbi_psd_is16(StbiContext* s);
#endif

#ifndef STBI_NO_HDR
    static int stbi_hdr_test(StbiContext* s);
    static float* stbi_hdr_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                                const StbiResultInfo* ri);
    static int stbi_hdr_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_PIC
    static int stbi_pic_test(StbiContext* s);
    static void* stbi_pic_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               const StbiResultInfo* ri);
    static int stbi_pic_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_GIF
    static int stbi_gif_test(StbiContext* s);
    static void* stbi_gif_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               const StbiResultInfo* ri);
    static void* stbi_load_gif_main(StbiContext* s, int** delays, int* x, int* y, int* z, int* comp,
                                    int req_comp);
    static int stbi_gif_info(StbiContext* s, int* x, int* y, int* comp);
#endif

#ifndef STBI_NO_PNM
    static int stbi_pnm_test(StbiContext* s);
    static void* stbi_pnm_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               StbiResultInfo* ri);
    static int stbi_pnm_info(StbiContext* s, int* x, int* y, int* comp);
    static int stbi_pnm_is16(StbiContext* s);
#endif

    static const char* stbi_g_failure_reason;

    const char* stbi_failure_reason(void)
    {
        return stbi_g_failure_reason;
    }

#ifndef STBI_NO_FAILURE_STRINGS
    static int stbi__err(const char* str)
    {
        stbi_g_failure_reason = str;
        return 0;
    }
#endif

    static void* stbi_malloc(const size_t size)
    {
        return STBI_MALLOC(size);
    }

    // stb_image uses ints pervasively, including for offset calculations.
    // therefore the largest decoded image size we can support with the
    // current code, even on 64-bit targets, is INT_MAX. this is not a
    // significant limitation for the intended use case.
    //
    // we do, however, need to make sure our size calculations don't
    // overflow. hence a few helper functions for size calculations that
    // multiply integers together, making sure that they're non-negative
    // and no overflow occurs.

    // return 1 if the sum is valid, 0 on overflow.
    // negative terms are considered invalid.
    static int stbi_addsizes_valid(const int a, const int b)
    {
        if (b < 0)
            return 0;
        // now 0 <= b <= INT_MAX, hence also
        // 0 <= INT_MAX - b <= INTMAX.
        // And "a + b <= INT_MAX" (which might overflow) is the
        // same as a <= INT_MAX - b (no overflow)
        return a <= INT_MAX - b;
    }

    // returns 1 if the product is valid, 0 on overflow.
    // negative factors are considered invalid.
    static int stbi_mul2sizes_valid(const int a, const int b)
    {
        if (a < 0 || b < 0)
            return 0;
        if (b == 0)
            return 1;  // mul-by-0 is always safe
        // portable way to check for no overflows in a*b
        return a <= INT_MAX / b;
    }

#if !defined(STBI_NO_JPEG) || !defined(STBI_NO_PNG) || \
    !defined(STBI_NO_TGA) || !defined(STBI_NO_HDR)
    // returns 1 if "a*b + add" has no negative terms/factors and doesn't overflow
    static int stbi_mad2sizes_valid(const int a, const int b, const int add)
    {
        return stbi_mul2sizes_valid(a, b) && stbi_addsizes_valid(a * b, add);
    }
#endif

    // returns 1 if "a*b*c + add" has no negative terms/factors and doesn't overflow
    static int stbi_mad3sizes_valid(const int a, const int b, const int c, const int add)
    {
        return stbi_mul2sizes_valid(a, b) &&
               stbi_mul2sizes_valid(a * b, c) &&
               stbi_addsizes_valid(a * b * c, add);
    }

// returns 1 if "a*b*c*d + add" has no negative terms/factors and doesn't overflow
#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR) || !defined(STBI_NO_PNM)
    static int stbi_mad4sizes_valid(const int a, const int b, const int c, const int d,
                                    const int add)
    {
        return stbi_mul2sizes_valid(a, b) &&
               stbi_mul2sizes_valid(a * b, c) &&
               stbi_mul2sizes_valid(a * b * c, d) &&
               stbi_addsizes_valid(a * b * c * d, add);
    }
#endif

#if !defined(STBI_NO_JPEG) || !defined(STBI_NO_PNG) || \
    !defined(STBI_NO_TGA) || !defined(STBI_NO_HDR)
    // mallocs with size overflow checking
    static void* stbi_malloc_mad2(const int a, const int b, const int add)
    {
        if (!stbi_mad2sizes_valid(a, b, add))
            return nullptr;
        return stbi_malloc(a * b + add);
    }
#endif

    static void* stbi_malloc_mad3(const int a, const int b, const int c, const int add)
    {
        if (!stbi_mad3sizes_valid(a, b, c, add))
            return nullptr;
        return stbi_malloc(a * b * c + add);
    }

#if !defined(STBI_NO_LINEAR) || !defined(STBI_NO_HDR) || !defined(STBI_NO_PNM)
    static void* stbi_malloc_mad4(const int a, const int b, const int c, const int d, const int add)
    {
        if (!stbi_mad4sizes_valid(a, b, c, d, add))
            return nullptr;
        return stbi_malloc(a * b * c * d + add);
    }
#endif

    // returns 1 if the sum of two signed ints is valid (between -2^31 and 2^31-1 inclusive), 0 on
    // overflow.
    static int stbi_addints_valid(const int a, const int b)
    {
        if ((a >= 0) != (b >= 0))
            return 1;  // a and b have different signs, so no overflow
        if (a < 0 && b < 0)
            return a >= INT_MIN - b;  // same as a + b >= INT_MIN; INT_MIN - b cannot overflow since
                                      // b < 0.
        return a <= INT_MAX - b;
    }

    // returns 1 if the product of two ints fits in a signed short, 0 on overflow.
    static int stbi_mul2shorts_valid(const int a, const int b)
    {
        if (b == 0 || b == -1)
            return 1;  // multiplication by 0 is always 0; check for -1 so SHRT_MIN/b doesn't
                       // overflow
        if ((a >= 0) == (b >= 0))
            return a <= SHRT_MAX / b;  // product is positive, so similar to mul2sizes_valid
        if (b < 0)
            return a <= SHRT_MIN / b;  // same as a * b >= SHRT_MIN
        return a >= SHRT_MIN / b;
    }

    // stbi__err - error
    // stbi__errpf - error returning pointer to float
    // stbi__errpuc - error returning pointer to unsigned char

#ifdef STBI_NO_FAILURE_STRINGS
  #define stbi__err(x, y) 0
#elif defined(STBI_FAILURE_USERMSG)
  #define stbi__err(x, y) stbi__err(y)
#else
  #define stbi__err(x, y) stbi__err(x)
#endif

#define stbi_errpf(x, y)  ((float*)(size_t)(stbi__err(x, y) ? nullptr : nullptr))
#define stbi_errpuc(x, y) ((unsigned char*)(size_t)(stbi__err(x, y) ? nullptr : nullptr))

    void stbi_image_free(void* retval_from_stbi_load)
    {
        STBI_FREE(retval_from_stbi_load);
    }

#ifndef STBI_NO_LINEAR
    static float* stbi_ldr_to_hdr(stbi_uc* data, int x, int y, int comp);
#endif

#ifndef STBI_NO_HDR
    static stbi_uc* stbi_hdr_to_ldr(float* data, int x, int y, int comp);
#endif

    static int stbi__vertically_flip_on_load_global = 0;

    void stbi_set_flip_vertically_on_load(const int flag_true_if_should_flip)
    {
        stbi__vertically_flip_on_load_global = flag_true_if_should_flip;
    }

#ifndef STBI_THREAD_LOCAL
  #define stbi__vertically_flip_on_load stbi__vertically_flip_on_load_global
#else
    static STBI_THREAD_LOCAL int stbi__vertically_flip_on_load_local,
        stbi__vertically_flip_on_load_set;

    void stbi_set_flip_vertically_on_load_thread(const int flag_true_if_should_flip)
    {
        stbi__vertically_flip_on_load_local = flag_true_if_should_flip;
        stbi__vertically_flip_on_load_set = 1;
    }

  #define stbi_vertically_flip_on_load                                         \
      (stbi__vertically_flip_on_load_set ? stbi__vertically_flip_on_load_local \
                                         : stbi__vertically_flip_on_load_global)
#endif  // STBI_THREAD_LOCAL

    static void* stbi_load_main(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                                StbiResultInfo* ri, int bpc)
    {
        memset(ri, 0, sizeof(*ri));          // make sure it's initialized if we add new fields
        ri->bits_per_channel = 8;            // default is 8 so most paths don't have to be changed
        ri->channel_order = STBI_ORDER_RGB;  // all current input & output are this, but this is
                                             // here so we can add BGR order
        ri->num_channels = 0;

// test the formats with a very explicit header first (at least a FOURCC
// or distinctive magic number first)
#ifndef STBI_NO_PNG
        if (stbi_png_test(s))
            return stbi_png_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_BMP
        if (stbi_bmp_test(s))
            return stbi_bmp_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_GIF
        if (stbi_gif_test(s))
            return stbi_gif_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_PSD
        if (stbi_psd_test(s))
            return stbi_psd_load(s, x, y, comp, req_comp, ri, bpc);
#else
        STBI_NOTUSED(bpc);
#endif
#ifndef STBI_NO_PIC
        if (stbi_pic_test(s))
            return stbi_pic_load(s, x, y, comp, req_comp, ri);
#endif

// then the formats that can end up attempting to load with just 1 or 2
// bytes matching expectations; these are prone to false positives, so
// try them later
#ifndef STBI_NO_JPEG
        if (stbi_jpeg_test(s))
            return stbi_jpeg_load(s, x, y, comp, req_comp, ri);
#endif
#ifndef STBI_NO_PNM
        if (stbi_pnm_test(s))
            return stbi_pnm_load(s, x, y, comp, req_comp, ri);
#endif

#ifndef STBI_NO_HDR
        if (stbi_hdr_test(s)) {
            float* hdr = stbi_hdr_load(s, x, y, comp, req_comp, ri);
            return stbi_hdr_to_ldr(hdr, *x, *y, req_comp ? req_comp : *comp);
        }
#endif

#ifndef STBI_NO_TGA
        // test tga last because it's a crappy test!
        if (stbi_tga_test(s))
            return stbi_tga_load(s, x, y, comp, req_comp, ri);
#endif

        return stbi_errpuc("unknown image type", "Image not of any known type, or corrupt");
    }

    static stbi_uc* stbi_convert_16_to_8(StbiUint16* orig, const int w, const int h,
                                         const int channels)
    {
        const int img_len = w * h * channels;
        stbi_uc* reduced = static_cast<stbi_uc*>(stbi_malloc(img_len));
        if (reduced == nullptr)
            return stbi_errpuc("outofmem", "Out of memory");

        for (int i = 0; i < img_len; ++i)
            reduced[i] = static_cast<stbi_uc>((orig[i] >> 8) & 0xFF);  // top half of each byte is
                                                                       // sufficient approx of 16->8
                                                                       // bit scaling

        STBI_FREE(orig);
        return reduced;
    }

    static StbiUint16* stbi_convert_8_to_16(stbi_uc* orig, const int w, const int h,
                                            const int channels)
    {
        const int img_len = w * h * channels;

        StbiUint16* enlarged = static_cast<StbiUint16*>(stbi_malloc(img_len * 2));
        if (enlarged == nullptr)
            return reinterpret_cast<StbiUint16*>(stbi_errpuc("outofmem", "Out of memory"));

        for (int i = 0; i < img_len; ++i)
            enlarged[i] = static_cast<StbiUint16>((orig[i] << 8) + orig[i]);  // replicate to high
                                                                              // and low byte,
                                                                              // maps 0->0,
                                                                              // 255->0xffff

        STBI_FREE(orig);
        return enlarged;
    }

    static void stbi_vertical_flip(void* image, int w, int h, int bytes_per_pixel)
    {
        const size_t bytes_per_row = static_cast<size_t>(w) * bytes_per_pixel;
        stbi_uc temp[2048];
        stbi_uc* bytes = static_cast<stbi_uc*>(image);

        for (int row = 0; row < (h >> 1); row++) {
            stbi_uc* row0 = bytes + row * bytes_per_row;
            stbi_uc* row1 = bytes + (h - row - 1) * bytes_per_row;
            // swap row0 with row1
            size_t bytes_left = bytes_per_row;
            while (bytes_left) {
                const size_t bytes_copy = (bytes_left < sizeof(temp)) ? bytes_left : sizeof(temp);
                memcpy(temp, row0, bytes_copy);
                memcpy(row0, row1, bytes_copy);
                memcpy(row1, temp, bytes_copy);
                row0 += bytes_copy;
                row1 += bytes_copy;
                bytes_left -= bytes_copy;
            }
        }
    }

#ifndef STBI_NO_GIF
    static void stbi_vertical_flip_slices(void* image, const int w, const int h, const int z,
                                          const int bytes_per_pixel)
    {
        const int slice_size = w * h * bytes_per_pixel;

        stbi_uc* bytes = static_cast<stbi_uc*>(image);
        for (int slice = 0; slice < z; ++slice) {
            stbi_vertical_flip(bytes, w, h, bytes_per_pixel);
            bytes += slice_size;
        }
    }
#endif

    static unsigned char* stbi_load_and_postprocess_8_bit(StbiContext* s, int* x, int* y, int* comp,
                                                          const int req_comp)
    {
        StbiResultInfo ri;
        void* result = stbi_load_main(s, x, y, comp, req_comp, &ri, 8);

        if (result == nullptr)
            return nullptr;

        // it is the responsibility of the loaders to make sure we get either 8 or 16 bit.
        STBI_ASSERT(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);

        if (ri.bits_per_channel != 8) {
            result = stbi_convert_16_to_8(static_cast<StbiUint16*>(result), *x, *y,
                                          req_comp == 0 ? *comp : req_comp);
            ri.bits_per_channel = 8;
        }

        // @TODO: move stbi__convert_format to here

        if (stbi_vertically_flip_on_load) {
            const int channels = req_comp ? req_comp : *comp;
            stbi_vertical_flip(result, *x, *y, channels * sizeof(stbi_uc));
        }

        return static_cast<unsigned char*>(result);
    }

    static StbiUint16* stbi_load_and_postprocess_16bit(StbiContext* s, int* x, int* y, int* comp,
                                                       const int req_comp)
    {
        StbiResultInfo ri;
        void* result = stbi_load_main(s, x, y, comp, req_comp, &ri, 16);

        if (result == nullptr)
            return nullptr;

        // it is the responsibility of the loaders to make sure we get either 8 or 16 bit.
        STBI_ASSERT(ri.bits_per_channel == 8 || ri.bits_per_channel == 16);

        if (ri.bits_per_channel != 16) {
            result = stbi_convert_8_to_16(static_cast<stbi_uc*>(result), *x, *y,
                                          req_comp == 0 ? *comp : req_comp);
            ri.bits_per_channel = 16;
        }

        // @TODO: move stbi__convert_format16 to here
        // @TODO: special case RGB-to-Y (and RGBA-to-YA) for 8-bit-to-16-bit case to keep more
        // precision

        if (stbi_vertically_flip_on_load) {
            const int channels = req_comp ? req_comp : *comp;
            stbi_vertical_flip(result, *x, *y, channels * sizeof(StbiUint16));
        }

        return static_cast<StbiUint16*>(result);
    }

#if !defined(STBI_NO_HDR) && !defined(STBI_NO_LINEAR)
    static void stbi_float_postprocess(float* result, const int* x, const int* y, const int* comp,
                                       const int req_comp)
    {
        if (stbi_vertically_flip_on_load && result != nullptr) {
            const int channels = req_comp ? req_comp : *comp;
            stbi_vertical_flip(result, *x, *y, channels * sizeof(float));
        }
    }
#endif

#ifndef STBI_NO_STDIO

  #if defined(_WIN32) && defined(STBI_WINDOWS_UTF8)
    STBI_EXTERN __declspec(dllimport) int __stdcall MultiByteToWideChar(
        unsigned int cp, unsigned long flags, const char* str, int cbmb, wchar_t* widestr,
        int cchwide);
    STBI_EXTERN __declspec(dllimport) int __stdcall WideCharToMultiByte(
        unsigned int cp, unsigned long flags, const wchar_t* widestr, int cchwide, char* str,
        int cbmb, const char* defchar, int* used_default);
  #endif

  #if defined(_WIN32) && defined(STBI_WINDOWS_UTF8)
    int stbi_convert_wchar_to_utf8(char* buffer, size_t bufferlen, const wchar_t* input)
    {
        return WideCharToMultiByte(65001 /* UTF8 */, 0, input, -1, buffer, (int)bufferlen, nullptr,
                                   nullptr);
    }
  #endif

    static FILE* stbi_fopen(const char* filename, const char* mode)
    {
        FILE* f;
  #if defined(_WIN32) && defined(STBI_WINDOWS_UTF8)
        wchar_t wMode[64];
        wchar_t wFilename[1024];
        if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, filename, -1, wFilename,
                                     sizeof(wFilename) / sizeof(*wFilename)))
            return 0;

        if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, mode, -1, wMode,
                                     sizeof(wMode) / sizeof(*wMode)))
            return 0;

    #if defined(_MSC_VER) && _MSC_VER >= 1400
        if (0 != _wfopen_s(&f, wFilename, wMode))
            f = 0;
    #else
        f = _wfopen(wFilename, wMode);
    #endif

  #elif defined(_MSC_VER) && _MSC_VER >= 1400
        if (0 != fopen_s(&f, filename, mode))
            f = 0;
  #else
        f = fopen(filename, mode);
  #endif
        return f;
    }

    stbi_uc* stbi_load(const char* filename, int* x, int* y, int* comp, const int req_comp)
    {
        FILE* f = stbi_fopen(filename, "rb");
        if (!f)
            return stbi_errpuc("can't fopen", "Unable to open file");
        unsigned char* result = stbi_load_from_file(f, x, y, comp, req_comp);
        fclose(f);
        return result;
    }

    stbi_uc* stbi_load_from_file(FILE* f, int* x, int* y, int* comp, const int req_comp)
    {
        StbiContext s;
        stbi_start_file(&s, f);
        unsigned char* result = stbi_load_and_postprocess_8_bit(&s, x, y, comp, req_comp);
        if (result) {
            // need to 'unget' all the characters in the IO buffer
            fseek(f, -static_cast<int>(s.img_buffer_end - s.img_buffer), SEEK_CUR);
        }
        return result;
    }

    StbiUint16* stbi_load_from_file_16(FILE* f, int* x, int* y, int* comp, const int req_comp)
    {
        StbiContext s;
        stbi_start_file(&s, f);
        StbiUint16* result = stbi_load_and_postprocess_16bit(&s, x, y, comp, req_comp);
        if (result) {
            // need to 'unget' all the characters in the IO buffer
            std::fseek(f, -static_cast<int>(s.img_buffer_end - s.img_buffer), SEEK_CUR);
        }
        return result;
    }

    stbi_us* stbi_load_16(const char* filename, int* x, int* y, int* comp, const int req_comp)
    {
        FILE* f = stbi_fopen(filename, "rb");
        if (!f)
            return reinterpret_cast<stbi_us*>(stbi_errpuc("can't fopen", "Unable to open file"));
        StbiUint16* result = stbi_load_from_file_16(f, x, y, comp, req_comp);
        std::fclose(f);
        return result;
    }

#endif  //! STBI_NO_STDIO

    stbi_us* stbi_load_16_from_memory(const stbi_uc* buffer, const int len, int* x, int* y,
                                      int* channels_in_file, const int desired_channels)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
    }

    stbi_us* stbi_load_16_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                         int* channels_in_file, const int desired_channels)
    {
        StbiContext s;
        stbi_start_callbacks(&s, const_cast<stbi_io_callbacks*>(clbk), user);
        return stbi_load_and_postprocess_16bit(&s, x, y, channels_in_file, desired_channels);
    }

    stbi_uc* stbi_load_from_memory(const stbi_uc* buffer, const int len, int* x, int* y, int* comp,
                                   const int req_comp)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_load_and_postprocess_8_bit(&s, x, y, comp, req_comp);
    }

    stbi_uc* stbi_load_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                      int* channels_in_file, const int desired_channels)
    {
        StbiContext s;
        stbi_start_callbacks(&s, clbk, user);
        return stbi_load_and_postprocess_8_bit(&s, x, y, channels_in_file, desired_channels);
    }

#ifndef STBI_NO_GIF
    stbi_uc* stbi_load_gif_from_memory(const stbi_uc* buffer, const int len, int** delays, int* x,
                                       int* y, int* z, int* comp, const int req_comp)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);

        unsigned char* result = static_cast<unsigned char*>(
            stbi_load_gif_main(&s, delays, x, y, z, comp, req_comp));
        if (stbi_vertically_flip_on_load)
            stbi_vertical_flip_slices(result, *x, *y, *z, *comp);

        return result;
    }
#endif

#ifndef STBI_NO_LINEAR
    static float* stbi_loadf_main(StbiContext* s, int* x, int* y, int* comp, const int req_comp)
    {
        unsigned char* data;
  #ifndef STBI_NO_HDR
        if (stbi_hdr_test(s)) {
            StbiResultInfo ri;
            float* hdr_data = stbi_hdr_load(s, x, y, comp, req_comp, &ri);
            if (hdr_data)
                stbi_float_postprocess(hdr_data, x, y, comp, req_comp);
            return hdr_data;
        }
  #endif
        data = stbi_load_and_postprocess_8_bit(s, x, y, comp, req_comp);
        if (data)
            return stbi_ldr_to_hdr(data, *x, *y, req_comp ? req_comp : *comp);
        return stbi_errpf("unknown image type", "Image not of any known type, or corrupt");
    }

    float* stbi_loadf_from_memory(const stbi_uc* buffer, const int len, int* x, int* y, int* comp,
                                  const int req_comp)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_loadf_main(&s, x, y, comp, req_comp);
    }

    float* stbi_loadf_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                     int* comp, const int req_comp)
    {
        StbiContext s;
        stbi_start_callbacks(&s, clbk, user);
        return stbi_loadf_main(&s, x, y, comp, req_comp);
    }

  #ifndef STBI_NO_STDIO
    float* stbi_loadf(const char* filename, int* x, int* y, int* comp, const int req_comp)
    {
        FILE* f = stbi_fopen(filename, "rb");
        if (!f)
            return stbi_errpf("can't fopen", "Unable to open file");
        float* result = stbi_loadf_from_file(f, x, y, comp, req_comp);
        std::fclose(f);
        return result;
    }

    float* stbi_loadf_from_file(FILE* f, int* x, int* y, int* comp, const int req_comp)
    {
        StbiContext s;
        stbi_start_file(&s, f);
        return stbi_loadf_main(&s, x, y, comp, req_comp);
    }
  #endif  // !STBI_NO_STDIO

#endif  // !STBI_NO_LINEAR

    // these is-hdr-or-not is defined independent of whether STBI_NO_LINEAR is
    // defined, for API simplicity; if STBI_NO_LINEAR is defined, it always
    // reports false!

    int stbi_is_hdr_from_memory(const stbi_uc* buffer, int len)
    {
#ifndef STBI_NO_HDR
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_hdr_test(&s);
#else
        STBI_NOTUSED(buffer);
        STBI_NOTUSED(len);
        return 0;
#endif
    }

#ifndef STBI_NO_STDIO
    int stbi_is_hdr(const char* filename)
    {
        FILE* f = stbi_fopen(filename, "rb");
        int result = 0;
        if (f) {
            result = stbi_is_hdr_from_file(f);
            std::fclose(f);
        }
        return result;
    }

    int stbi_is_hdr_from_file(FILE* f)
    {
  #ifndef STBI_NO_HDR
        const long pos = ftell(f);
        StbiContext s;
        stbi_start_file(&s, f);
        const int res = stbi_hdr_test(&s);
        std::fseek(f, pos, SEEK_SET);
        return res;
  #else
        STBI_NOTUSED(f);
        return 0;
  #endif
    }
#endif  // !STBI_NO_STDIO

    int stbi_is_hdr_from_callbacks(const stbi_io_callbacks* clbk, void* user)
    {
#ifndef STBI_NO_HDR
        StbiContext s;
        stbi_start_callbacks(&s, clbk, user);
        return stbi_hdr_test(&s);
#else
        STBI_NOTUSED(clbk);
        STBI_NOTUSED(user);
        return 0;
#endif
    }

#ifndef STBI_NO_LINEAR
    static float stbi_l2h_gamma = 2.2f;
    static float stbi_l2h_scale = 1.0f;

    void stbi_ldr_to_hdr_gamma(const float gamma)
    {
        stbi_l2h_gamma = gamma;
    }

    void stbi_ldr_to_hdr_scale(const float scale)
    {
        stbi_l2h_scale = scale;
    }
#endif

    static float stbi_h2l_gamma_i = 1.0f / 2.2f, stbi_h2l_scale_i = 1.0f;

    void stbi_hdr_to_ldr_gamma(const float gamma)
    {
        stbi_h2l_gamma_i = 1 / gamma;
    }

    void stbi_hdr_to_ldr_scale(const float scale)
    {
        stbi_h2l_scale_i = 1 / scale;
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // Common code used by all image loaders
    //

    enum {
        STBI_SCAN_load = 0,
        STBI_SCAN_type,
        STBI_SCAN_header
    };

    static void stbi_refill_buffer(StbiContext* s)
    {
        const int n = (s->io.read)(s->io_user_data, (char*)s->buffer_start, s->buflen);
        s->callback_already_read += static_cast<int>(s->img_buffer - s->img_buffer_original);
        if (n == 0) {
            // at end of file, treat same as if from memory, but need to handle case
            // where s->img_buffer isn't pointing to safe memory, e.g. 0-byte file
            s->read_from_callbacks = 0;
            s->img_buffer = s->buffer_start;
            s->img_buffer_end = s->buffer_start + 1;
            *s->img_buffer = 0;
        }
        else {
            s->img_buffer = s->buffer_start;
            s->img_buffer_end = s->buffer_start + n;
        }
    }

    static stbi_inline stbi_uc stbi_get8(StbiContext* s)
    {
        if (s->img_buffer < s->img_buffer_end)
            return *s->img_buffer++;
        if (s->read_from_callbacks) {
            stbi_refill_buffer(s);
            return *s->img_buffer++;
        }
        return 0;
    }

#if defined(STBI_NO_JPEG) && defined(STBI_NO_HDR) && defined(STBI_NO_PIC) && defined(STBI_NO_PNM)
// nothing
#else
    static stbi_inline int stbi_at_eof(const StbiContext* s)
    {
        if (s->io.read) {
            if (!(s->io.eof)(s->io_user_data))
                return 0;
            // if feof() is true, check if buffer = end
            // special case: we've only got the special 0 character at the end
            if (s->read_from_callbacks == 0)
                return 1;
        }

        return s->img_buffer >= s->img_buffer_end;
    }
#endif

#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && \
    defined(STBI_NO_PSD) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF) && defined(STBI_NO_PIC)
// nothing
#else
    static void stbi_skip(StbiContext* s, const int n)
    {
        if (n == 0)
            return;  // already there!
        if (n < 0) {
            s->img_buffer = s->img_buffer_end;
            return;
        }
        if (s->io.read) {
            const int blen = static_cast<int>(s->img_buffer_end - s->img_buffer);
            if (blen < n) {
                s->img_buffer = s->img_buffer_end;
                (s->io.skip)(s->io_user_data, n - blen);
                return;
            }
        }
        s->img_buffer += n;
    }
#endif

#if defined(STBI_NO_PNG) && defined(STBI_NO_TGA) && defined(STBI_NO_HDR) && defined(STBI_NO_PNM)
// nothing
#else
    static int stbi_getn(StbiContext* s, stbi_uc* buffer, const int n)
    {
        if (s->io.read) {
            const int blen = static_cast<int>(s->img_buffer_end - s->img_buffer);
            if (blen < n) {
                memcpy(buffer, s->img_buffer, blen);

                const int count = (s->io.read)(s->io_user_data, (char*)buffer + blen, n - blen);
                const int res = (count == (n - blen));
                s->img_buffer = s->img_buffer_end;
                return res;
            }
        }

        if (s->img_buffer + n <= s->img_buffer_end) {
            memcpy(buffer, s->img_buffer, n);
            s->img_buffer += n;
            return 1;
        }
        else
            return 0;
    }
#endif

#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_PSD) && defined(STBI_NO_PIC)
// nothing
#else
    static int stbi_get16be(StbiContext* s)
    {
        const int z = stbi_get8(s);
        return (z << 8) + stbi_get8(s);
    }
#endif

#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD) && defined(STBI_NO_PIC)
// nothing
#else
    static StbiUint32 stbi_get32be(StbiContext* s)
    {
        const StbiUint32 z = stbi_get16be(s);
        return (z << 16) + stbi_get16be(s);
    }
#endif

#if defined(STBI_NO_BMP) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF)
// nothing
#else
    static int stbi_get16le(StbiContext* s)
    {
        const int z = stbi_get8(s);
        return z + (stbi_get8(s) << 8);
    }
#endif

#ifndef STBI_NO_BMP
    static StbiUint32 stbi_get32le(StbiContext* s)
    {
        StbiUint32 z = stbi_get16le(s);
        z += static_cast<StbiUint32>(stbi_get16le(s)) << 16;
        return z;
    }
#endif

#define STBI_BYTECAST(x) ((stbi_uc)((x) & 255))  // truncate int to byte without warnings

#if defined(STBI_NO_JPEG) && defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && \
    defined(STBI_NO_PSD) && defined(STBI_NO_TGA) && defined(STBI_NO_GIF) &&  \
    defined(STBI_NO_PIC) && defined(STBI_NO_PNM)
// nothing
#else
    //////////////////////////////////////////////////////////////////////////////
    //
    //  generic converter from built-in img_n to req_comp
    //    individual types do this automatically as much as possible (e.g. jpeg
    //    does all cases internally since it needs to colorspace convert anyway,
    //    and it never has alpha, so very few cases ). png can automatically
    //    interleave an alpha=255 channel, but falls back to this for other cases
    //
    //  assume data buffer is malloced, so malloc a new one and free that one
    //  only failure mode is malloc failing

    static stbi_uc stbi_compute_y(const int r, const int g, const int b)
    {
        return static_cast<stbi_uc>(((r * 77) + (g * 150) + (29 * b)) >> 8);
    }
#endif

#if defined(STBI_NO_PNG) && defined(STBI_NO_BMP) && defined(STBI_NO_PSD) && \
    defined(STBI_NO_TGA) && defined(STBI_NO_GIF) && defined(STBI_NO_PIC) && defined(STBI_NO_PNM)
// nothing
#else
    static unsigned char* stbi_convert_format(unsigned char* data, const int img_n,
                                              const int req_comp, const unsigned int x,
                                              const unsigned int y)
    {
        int i;

        if (req_comp == img_n)
            return data;
        STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

        unsigned char* good = static_cast<unsigned char*>(stbi_malloc_mad3(req_comp, x, y, 0));
        if (good == nullptr) {
            STBI_FREE(data);
            return stbi_errpuc("outofmem", "Out of memory");
        }

        for (int j = 0; j < static_cast<int>(y); ++j) {
            unsigned char* src = data + j * x * img_n;
            unsigned char* dest = good + j * x * req_comp;

  #define STBI__COMBO(a, b) ((a) * 8 + (b))
  #define STBI_CASE(a, b)     \
      case STBI__COMBO(a, b): \
          for (i = x - 1; i >= 0; --i, src += a, dest += b)
            // convert source image with img_n components to one with req_comp components;
            // avoid switch per pixel, so use switch per scanline and massive macros
            switch (STBI__COMBO(img_n, req_comp)) {
                STBI_CASE(1, 2)
                {
                    dest[0] = src[0];
                    dest[1] = 255;
                }
                break;
                STBI_CASE(1, 3)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                }
                break;
                STBI_CASE(1, 4)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                    dest[3] = 255;
                }
                break;
                STBI_CASE(2, 1)
                {
                    dest[0] = src[0];
                }
                break;
                STBI_CASE(2, 3)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                }
                break;
                STBI_CASE(2, 4)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                    dest[3] = src[1];
                }
                break;
                STBI_CASE(3, 4)
                {
                    dest[0] = src[0];
                    dest[1] = src[1];
                    dest[2] = src[2];
                    dest[3] = 255;
                }
                break;
                STBI_CASE(3, 1)
                {
                    dest[0] = stbi_compute_y(src[0], src[1], src[2]);
                }
                break;
                STBI_CASE(3, 2)
                {
                    dest[0] = stbi_compute_y(src[0], src[1], src[2]);
                    dest[1] = 255;
                }
                break;
                STBI_CASE(4, 1)
                {
                    dest[0] = stbi_compute_y(src[0], src[1], src[2]);
                }
                break;
                STBI_CASE(4, 2)
                {
                    dest[0] = stbi_compute_y(src[0], src[1], src[2]);
                    dest[1] = src[3];
                }
                break;
                STBI_CASE(4, 3)
                {
                    dest[0] = src[0];
                    dest[1] = src[1];
                    dest[2] = src[2];
                }
                break;
                default:
                    STBI_ASSERT(0);
                    STBI_FREE(data);
                    STBI_FREE(good);
                    return stbi_errpuc("unsupported", "Unsupported format conversion");
            }
  #undef STBI__CASE
        }

        STBI_FREE(data);
        return good;
    }
#endif

#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD)
// nothing
#else
    static StbiUint16 stbi_compute_y_16(const int r, const int g, const int b)
    {
        return static_cast<StbiUint16>(((r * 77) + (g * 150) + (29 * b)) >> 8);
    }
#endif

#if defined(STBI_NO_PNG) && defined(STBI_NO_PSD)
// nothing
#else
    static StbiUint16* stbi_convert_format16(StbiUint16* data, const int img_n, const int req_comp,
                                             const unsigned int x, const unsigned int y)
    {
        int i;

        if (req_comp == img_n)
            return data;
        STBI_ASSERT(req_comp >= 1 && req_comp <= 4);

        StbiUint16* good = static_cast<StbiUint16*>(stbi_malloc(req_comp * x * y * 2));
        if (good == nullptr) {
            STBI_FREE(data);
            return reinterpret_cast<StbiUint16*>(stbi_errpuc("outofmem", "Out of memory"));
        }

        for (int j = 0; j < static_cast<int>(y); ++j) {
            StbiUint16* src = data + j * x * img_n;
            StbiUint16* dest = good + j * x * req_comp;

  #define STBI__COMBO(a, b) ((a) * 8 + (b))
  #define STBI_CASE(a, b)     \
      case STBI__COMBO(a, b): \
          for (i = x - 1; i >= 0; --i, src += a, dest += b)
            // convert source image with img_n components to one with req_comp components;
            // avoid switch per pixel, so use switch per scanline and massive macros
            switch (STBI__COMBO(img_n, req_comp)) {
                STBI_CASE(1, 2)
                {
                    dest[0] = src[0];
                    dest[1] = 0xffff;
                }
                break;
                STBI_CASE(1, 3)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                }
                break;
                STBI_CASE(1, 4)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                    dest[3] = 0xffff;
                }
                break;
                STBI_CASE(2, 1)
                {
                    dest[0] = src[0];
                }
                break;
                STBI_CASE(2, 3)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                }
                break;
                STBI_CASE(2, 4)
                {
                    dest[0] = dest[1] = dest[2] = src[0];
                    dest[3] = src[1];
                }
                break;
                STBI_CASE(3, 4)
                {
                    dest[0] = src[0];
                    dest[1] = src[1];
                    dest[2] = src[2];
                    dest[3] = 0xffff;
                }
                break;
                STBI_CASE(3, 1)
                {
                    dest[0] = stbi_compute_y_16(src[0], src[1], src[2]);
                }
                break;
                STBI_CASE(3, 2)
                {
                    dest[0] = stbi_compute_y_16(src[0], src[1], src[2]);
                    dest[1] = 0xffff;
                }
                break;
                STBI_CASE(4, 1)
                {
                    dest[0] = stbi_compute_y_16(src[0], src[1], src[2]);
                }
                break;
                STBI_CASE(4, 2)
                {
                    dest[0] = stbi_compute_y_16(src[0], src[1], src[2]);
                    dest[1] = src[3];
                }
                break;
                STBI_CASE(4, 3)
                {
                    dest[0] = src[0];
                    dest[1] = src[1];
                    dest[2] = src[2];
                }
                break;
                default:
                    STBI_ASSERT(0);
                    STBI_FREE(data);
                    STBI_FREE(good);
                    return reinterpret_cast<StbiUint16*>(
                        stbi_errpuc("unsupported", "Unsupported format conversion"));
            }
  #undef STBI__CASE
        }

        STBI_FREE(data);
        return good;
    }
#endif

#ifndef STBI_NO_LINEAR
    static float* stbi_ldr_to_hdr(stbi_uc* data, const int x, const int y, const int comp)
    {
        int i, n;
        if (!data)
            return nullptr;
        float* output = static_cast<float*>(stbi_malloc_mad4(x, y, comp, sizeof(float), 0));
        if (output == nullptr) {
            STBI_FREE(data);
            return stbi_errpf("outofmem", "Out of memory");
        }
        // compute number of non-alpha components
        if (comp & 1)
            n = comp;
        else
            n = comp - 1;
        for (i = 0; i < x * y; ++i) {
            for (int k = 0; k < n; ++k) {
                output[i * comp + k] = static_cast<float>(
                    pow(data[i * comp + k] / 255.0f, stbi_l2h_gamma) * stbi_l2h_scale);
            }
        }
        if (n < comp)
            for (i = 0; i < x * y; ++i)
                output[i * comp + n] = data[i * comp + n] / 255.0f;
        STBI_FREE(data);
        return output;
    }
#endif

#ifndef STBI_NO_HDR
  #define stbi_float2int(x) ((int)(x))

    static stbi_uc* stbi_hdr_to_ldr(float* data, const int x, const int y, const int comp)
    {
        int k, n;
        if (!data)
            return nullptr;
        stbi_uc* output = static_cast<stbi_uc*>(stbi_malloc_mad3(x, y, comp, 0));
        if (output == nullptr) {
            STBI_FREE(data);
            return stbi_errpuc("outofmem", "Out of memory");
        }
        // compute number of non-alpha components
        if (comp & 1)
            n = comp;
        else
            n = comp - 1;
        for (int i = 0; i < x * y; ++i) {
            for (k = 0; k < n; ++k) {
                float z{ (pow(data[i * comp + k] * stbi_h2l_scale_i, stbi_h2l_gamma_i)) *
                             255.0f +
                         0.5f };
                if (z < 0)
                    z = 0;
                if (z > 255)
                    z = 255;
                output[i * comp + k] = static_cast<stbi_uc>(stbi_float2int(z));
            }
            if (k < comp) {
                float z = data[i * comp + k] * 255 + 0.5f;
                if (z < 0)
                    z = 0;
                if (z > 255)
                    z = 255;
                output[i * comp + k] = static_cast<stbi_uc>(stbi_float2int(z));
            }
        }
        STBI_FREE(data);
        return output;
    }
#endif

    //////////////////////////////////////////////////////////////////////////////
    //
    //  "baseline" JPEG/JFIF decoder
    //
    //    simple implementation
    //      - doesn't support delayed output of y-dimension
    //      - simple interface (only one output format: 8-bit interleaved RGB)
    //      - doesn't try to recover corrupt jpegs
    //      - doesn't allow partial loading, loading multiple at once
    //      - still fast on x86 (copying globals into locals doesn't help x86)
    //      - allocates lots of intermediate memory (full size of all components)
    //        - non-interleaved case requires this anyway
    //        - allows good upsampling (see next)
    //    high-quality
    //      - upsampled channels are bilinearly interpolated, even across blocks
    //      - quality integer IDCT derived from IJG's 'slow'
    //    performance
    //      - fast huffman; reasonable integer IDCT
    //      - some SIMD kernels for common paths on targets with SSE2/NEON
    //      - uses a lot of intermediate memory, could cache poorly

#ifndef STBI_NO_JPEG

  // huffman decoding acceleration
  #define FAST_BITS     9  // larger handles more cases; smaller stomps less cache
  #define stbi_div16(x) ((stbi_uc)((x) >> 4))

    struct StbiHuffman
    {
        stbi_uc fast[1 << FAST_BITS];
        // weirdly, repacking this into AoS is a 10% speed loss, instead of a win
        StbiUint16 code[256];
        stbi_uc values[256];
        stbi_uc size[257];
        unsigned int maxcode[18];
        int delta[17];  // old 'firstsymbol' - old 'firstcode'
    };

    typedef struct
    {
        StbiContext* s;
        StbiHuffman huff_dc[4];
        StbiHuffman huff_ac[4];
        StbiUint16 dequant[4][64];
        StbiInt16 fast_ac[4][1 << FAST_BITS];

        // sizes for components, interleaved MCUs
        int img_h_max, img_v_max;
        int img_mcu_x, img_mcu_y;
        int img_mcu_w, img_mcu_h;

        // definition of jpeg image component
        struct
        {
            int id;
            int h, v;
            int tq;
            int hd, ha;
            int dc_pred;

            int x, y, w2, h2;
            stbi_uc* data;
            void *raw_data, *raw_coeff;
            stbi_uc* linebuf;
            short* coeff;          // progressive only
            int coeff_w, coeff_h;  // number of 8x8 coefficient blocks
        } img_comp[4];

        StbiUint32 code_buffer;  // jpeg entropy-coded buffer
        int code_bits;           // number of valid bits
        unsigned char marker;    // marker seen while filling entropy buffer
        int nomore;              // flag if we saw a marker so must stop

        int progressive;
        int spec_start;
        int spec_end;
        int succ_high;
        int succ_low;
        int eob_run;
        int jfif;
        int app14_color_transform;  // Adobe APP14 tag
        int rgb;

        int scan_n, order[4];
        int restart_interval, todo;

        // kernels
        void (*idct_block_kernel)(stbi_uc* out, int out_stride, short data[64]);
        void (*YCbCr_to_RGB_kernel)(stbi_uc* out, stbi_uc* y, stbi_uc* pcb, stbi_uc* pcr, int count,
                                    int step);
        stbi_uc* (*resample_row_hv_2_kernel)(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w,
                                             int hs);
    } stbi_jpeg;

    static int stbi_build_huffman(StbiHuffman* h, const int* count)
    {
        int i, j, k = 0;
        // build size list for each symbol (from JPEG spec)
        for (i = 0; i < 16; ++i) {
            for (j = 0; j < count[i]; ++j) {
                h->size[k++] = static_cast<stbi_uc>(i + 1);
                if (k >= 257)
                    return stbi__err("bad size list", "Corrupt JPEG");
            }
        }
        h->size[k] = 0;

        // compute actual symbols (from jpeg spec)
        unsigned int code = 0;
        k = 0;
        for (j = 1; j <= 16; ++j) {
            // compute delta to add to code to compute symbol id
            h->delta[j] = k - code;
            if (h->size[k] == j) {
                while (h->size[k] == j)
                    h->code[k++] = static_cast<StbiUint16>(code++);
                if (code - 1 >= (1u << j))
                    return stbi__err("bad code lengths", "Corrupt JPEG");
            }
            // compute largest code + 1 for this size, preshifted as needed later
            h->maxcode[j] = code << (16 - j);
            code <<= 1;
        }
        h->maxcode[j] = 0xffffffff;

        // build non-spec acceleration table; 255 is flag for not-accelerated
        memset(h->fast, 255, 1 << FAST_BITS);
        for (i = 0; i < k; ++i) {
            const int s = h->size[i];
            if (s <= FAST_BITS) {
                const int c = h->code[i] << (FAST_BITS - s);
                const int m = 1 << (FAST_BITS - s);
                for (j = 0; j < m; ++j)
                    h->fast[c + j] = static_cast<stbi_uc>(i);
            }
        }
        return 1;
    }

    // build a table that decodes both magnitude and value of small ACs in
    // one go.
    static void stbi_build_fast_ac(StbiInt16* fast_ac, const StbiHuffman* h)
    {
        for (int i = 0; i < (1 << FAST_BITS); ++i) {
            const stbi_uc fast = h->fast[i];
            fast_ac[i] = 0;
            if (fast < 255) {
                const int rs = h->values[fast];
                const int run = (rs >> 4) & 15;
                const int magbits = rs & 15;
                const int len = h->size[fast];

                if (magbits && len + magbits <= FAST_BITS) {
                    // magnitude code followed by receive_extend code
                    int k = ((i << len) & ((1 << FAST_BITS) - 1)) >> (FAST_BITS - magbits);
                    const int m = 1 << (magbits - 1);
                    if (k < m)
                        k += (~0U << magbits) + 1;
                    // if the result is small enough, we can fit it in fast_ac table
                    if (k >= -128 && k <= 127)
                        fast_ac[i] = static_cast<StbiInt16>(
                            (k * 256) + (run * 16) + (len + magbits));
                }
            }
        }
    }

    static void stbi_grow_buffer_unsafe(stbi_jpeg* j)
    {
        do {
            const unsigned int b = j->nomore ? 0 : stbi_get8(j->s);
            if (b == 0xff) {
                int c = stbi_get8(j->s);
                while (c == 0xff)
                    c = stbi_get8(j->s);  // consume fill bytes
                if (c != 0) {
                    j->marker = static_cast<unsigned char>(c);
                    j->nomore = 1;
                    return;
                }
            }
            j->code_buffer |= b << (24 - j->code_bits);
            j->code_bits += 8;
        }
        while (j->code_bits <= 24);
    }

    // (1 << n) - 1
    static const StbiUint32 stbi_bmask[17] = { 0, 1, 3, 7, 15, 31, 63, 127, 255,
                                               511, 1023, 2047, 4095, 8191, 16383,
                                               32767, 65535 };

    // decode a jpeg huffman value from the bitstream
    static stbi_inline int stbi_jpeg_huff_decode(stbi_jpeg* j, const StbiHuffman* h)
    {
        if (j->code_bits < 16)
            stbi_grow_buffer_unsafe(j);

        // look at the top FAST_BITS and determine what symbol ID it is,
        // if the code is <= FAST_BITS
        int c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
        int k = h->fast[c];
        if (k < 255) {
            const int s = h->size[k];
            if (s > j->code_bits)
                return -1;
            j->code_buffer <<= s;
            j->code_bits -= s;
            return h->values[k];
        }

        // naive test is to shift the code_buffer down so k bits are
        // valid, then test against maxcode. To speed this up, we've
        // preshifted maxcode left so that it has (16-k) 0s at the
        // end; in other words, regardless of the number of bits, it
        // wants to be compared against something shifted to have 16;
        // that way we don't need to shift inside the loop.
        const unsigned int temp = j->code_buffer >> 16;
        for (k = FAST_BITS + 1;; ++k)
            if (temp < h->maxcode[k])
                break;
        if (k == 17) {
            // error! code not found
            j->code_bits -= 16;
            return -1;
        }

        if (k > j->code_bits)
            return -1;

        // convert the huffman code to the symbol id
        c = ((j->code_buffer >> (32 - k)) & stbi_bmask[k]) + h->delta[k];
        if (c < 0 || c >= 256)  // symbol id out of bounds!
            return -1;
        STBI_ASSERT(
            (((j->code_buffer) >> (32 - h->size[c])) & stbi_bmask[h->size[c]]) == h->code[c]);

        // convert the id to a symbol
        j->code_bits -= k;
        j->code_buffer <<= k;
        return h->values[c];
    }

    // bias[n] = (-1<<n) + 1
    constexpr static int stbi_jbias[16] = { 0, -1, -3, -7, -15, -31, -63, -127,
                                            -255, -511, -1023, -2047, -4095, -8191, -16383, -32767 };

    // combined JPEG 'receive' and JPEG 'extend', since baseline
    // always extends everything it receives.
    static stbi_inline int stbi_extend_receive(stbi_jpeg* j, const int n)
    {
        if (j->code_bits < n)
            stbi_grow_buffer_unsafe(j);

        // ran out of bits from stream,
        // return 0s intead of continuing
        if (j->code_bits < n)
            return 0;

        // sign bit always in MSB;
        // 0 if MSB clear (positive),
        // 1 if MSB set (negative)
        const int sgn = j->code_buffer >> 31;
        unsigned int k = stbi_lrot(j->code_buffer, n);
        j->code_buffer = k & ~stbi_bmask[n];
        k &= stbi_bmask[n];
        j->code_bits -= n;
        return k + (stbi_jbias[n] & (sgn - 1));
    }

    // get some unsigned bits
    static stbi_inline int stbi_jpeg_get_bits(stbi_jpeg* j, const int n)
    {
        if (j->code_bits < n)
            stbi_grow_buffer_unsafe(j);
        if (j->code_bits < n)
            return 0;  // ran out of bits from stream, return 0s intead of continuing
        unsigned int k = stbi_lrot(j->code_buffer, n);
        j->code_buffer = k & ~stbi_bmask[n];
        k &= stbi_bmask[n];
        j->code_bits -= n;
        return static_cast<int>(k);
    }

    static stbi_inline int stbi_jpeg_get_bit(stbi_jpeg* j)
    {
        if (j->code_bits < 1)
            stbi_grow_buffer_unsafe(j);
        if (j->code_bits < 1)
            return 0;  // ran out of bits from stream, return 0s intead of continuing
        const unsigned int k = j->code_buffer;
        j->code_buffer <<= 1;
        --j->code_bits;
        return static_cast<int>(k & 0x80000000);
    }

    // given a value that's at position X in the zigzag stream,
    // where does it appear in the 8x8 matrix coded as row-major?
    static const stbi_uc stbi_jpeg_dezigzag[64 + 15] = {
        0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5, 12, 19, 26, 33, 40, 48, 41, 34, 27,
        20, 13, 6, 7, 14, 21, 28, 35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
        58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63,
        // let corrupt input sample past end
        63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63
    };

    // decode one 64-entry block--
    static int stbi_jpeg_decode_block(stbi_jpeg* j, short data[64], StbiHuffman* hdc,
                                      StbiHuffman* hac, const StbiInt16* fac, const int b,
                                      const StbiUint16* dequant)
    {
        if (j->code_bits < 16)
            stbi_grow_buffer_unsafe(j);
        const int t = stbi_jpeg_huff_decode(j, hdc);
        if (t < 0 || t > 15)
            return stbi__err("bad huffman code", "Corrupt JPEG");

        // 0 all the ac values now so we can do it 32-bits at a time
        memset(data, 0, 64 * sizeof(data[0]));

        const int diff = t ? stbi_extend_receive(j, t) : 0;
        if (!stbi_addints_valid(j->img_comp[b].dc_pred, diff))
            return stbi__err("bad delta", "Corrupt JPEG");
        const int dc = j->img_comp[b].dc_pred + diff;
        j->img_comp[b].dc_pred = dc;
        if (!stbi_mul2shorts_valid(dc, dequant[0]))
            return stbi__err("can't merge dc and ac", "Corrupt JPEG");
        data[0] = static_cast<short>(dc * dequant[0]);

        // decode AC components, see JPEG spec
        int k = 1;
        do {
            unsigned int zig;
            int s;
            if (j->code_bits < 16)
                stbi_grow_buffer_unsafe(j);
            const int c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
            int r = fac[c];
            if (r) {                 // fast-AC path
                k += (r >> 4) & 15;  // run
                s = r & 15;          // combined length
                if (s > j->code_bits)
                    return stbi__err("bad huffman code",
                                     "Combined length longer than code bits available");
                j->code_buffer <<= s;
                j->code_bits -= s;
                // decode into unzigzag'd location
                zig = stbi_jpeg_dezigzag[k++];
                data[zig] = static_cast<short>((r >> 8) * dequant[zig]);
            }
            else {
                const int rs = stbi_jpeg_huff_decode(j, hac);
                if (rs < 0)
                    return stbi__err("bad huffman code", "Corrupt JPEG");
                s = rs & 15;
                r = rs >> 4;
                if (s == 0) {
                    if (rs != 0xf0)
                        break;  // end block
                    k += 16;
                }
                else {
                    k += r;
                    // decode into unzigzag'd location
                    zig = stbi_jpeg_dezigzag[k++];
                    data[zig] = static_cast<short>(stbi_extend_receive(j, s) * dequant[zig]);
                }
            }
        }
        while (k < 64);
        return 1;
    }

    static int stbi_jpeg_decode_block_prog_dc(stbi_jpeg* j, short data[64], StbiHuffman* hdc,
                                              const int b)
    {
        if (j->spec_end != 0)
            return stbi__err("can't merge dc and ac", "Corrupt JPEG");

        if (j->code_bits < 16)
            stbi_grow_buffer_unsafe(j);

        if (j->succ_high == 0) {
            // first scan for DC coefficient, must be first
            memset(data, 0, 64 * sizeof(data[0]));  // 0 all the ac values now
            const int t = stbi_jpeg_huff_decode(j, hdc);
            if (t < 0 || t > 15)
                return stbi__err("can't merge dc and ac", "Corrupt JPEG");
            const int diff = t ? stbi_extend_receive(j, t) : 0;

            if (!stbi_addints_valid(j->img_comp[b].dc_pred, diff))
                return stbi__err("bad delta", "Corrupt JPEG");
            const int dc = j->img_comp[b].dc_pred + diff;
            j->img_comp[b].dc_pred = dc;
            if (!stbi_mul2shorts_valid(dc, 1 << j->succ_low))
                return stbi__err("can't merge dc and ac", "Corrupt JPEG");
            data[0] = static_cast<short>(dc * (1 << j->succ_low));
        }
        else {
            // refinement scan for DC coefficient
            if (stbi_jpeg_get_bit(j))
                data[0] += static_cast<short>(1 << j->succ_low);
        }
        return 1;
    }

    // @OPTIMIZE: store non-zigzagged during the decode passes,
    // and only de-zigzag when dequantizing
    static int stbi_jpeg_decode_block_prog_ac(stbi_jpeg* j, short data[64], StbiHuffman* hac,
                                              const StbiInt16* fac)
    {
        int k;
        if (j->spec_start == 0)
            return stbi__err("can't merge dc and ac", "Corrupt JPEG");

        if (j->succ_high == 0) {
            const int shift = j->succ_low;

            if (j->eob_run) {
                --j->eob_run;
                return 1;
            }

            k = j->spec_start;
            do {
                unsigned int zig;
                int s;
                if (j->code_bits < 16)
                    stbi_grow_buffer_unsafe(j);
                const int c = (j->code_buffer >> (32 - FAST_BITS)) & ((1 << FAST_BITS) - 1);
                int r = fac[c];
                if (r) {                 // fast-AC path
                    k += (r >> 4) & 15;  // run
                    s = r & 15;          // combined length
                    if (s > j->code_bits)
                        return stbi__err("bad huffman code",
                                         "Combined length longer than code bits available");
                    j->code_buffer <<= s;
                    j->code_bits -= s;
                    zig = stbi_jpeg_dezigzag[k++];
                    data[zig] = static_cast<short>((r >> 8) * (1 << shift));
                }
                else {
                    const int rs = stbi_jpeg_huff_decode(j, hac);
                    if (rs < 0)
                        return stbi__err("bad huffman code", "Corrupt JPEG");
                    s = rs & 15;
                    r = rs >> 4;
                    if (s == 0) {
                        if (r < 15) {
                            j->eob_run = (1 << r);
                            if (r)
                                j->eob_run += stbi_jpeg_get_bits(j, r);
                            --j->eob_run;
                            break;
                        }
                        k += 16;
                    }
                    else {
                        k += r;
                        zig = stbi_jpeg_dezigzag[k++];
                        data[zig] = static_cast<short>(stbi_extend_receive(j, s) * (1 << shift));
                    }
                }
            }
            while (k <= j->spec_end);
        }
        else {
            // refinement scan for these AC coefficients

            const short bit = static_cast<short>(1 << j->succ_low);

            if (j->eob_run) {
                --j->eob_run;
                for (k = j->spec_start; k <= j->spec_end; ++k) {
                    short* p = &data[stbi_jpeg_dezigzag[k]];
                    if (*p != 0)
                        if (stbi_jpeg_get_bit(j))
                            if ((*p & bit) == 0) {
                                if (*p > 0)
                                    *p += bit;
                                else
                                    *p -= bit;
                            }
                }
            }
            else {
                k = j->spec_start;
                do {
                    const int rs = stbi_jpeg_huff_decode(j, hac);  // @OPTIMIZE see if we can use
                                                                   // the fast path here,
                                                                   // advance-by-r is so slow, eh
                    if (rs < 0)
                        return stbi__err("bad huffman code", "Corrupt JPEG");
                    int s = rs & 15;
                    int r = rs >> 4;
                    if (s == 0) {
                        if (r < 15) {
                            j->eob_run = (1 << r) - 1;
                            if (r)
                                j->eob_run += stbi_jpeg_get_bits(j, r);
                            r = 64;  // force end of block
                        }
                        else {
                            // r=15 s=0 should write 16 0s, so we just do
                            // a run of 15 0s and then write s (which is 0),
                            // so we don't have to do anything special here
                        }
                    }
                    else {
                        if (s != 1)
                            return stbi__err("bad huffman code", "Corrupt JPEG");
                        // sign bit
                        if (stbi_jpeg_get_bit(j))
                            s = bit;
                        else
                            s = -bit;
                    }

                    // advance by r
                    while (k <= j->spec_end) {
                        short* p = &data[stbi_jpeg_dezigzag[k++]];
                        if (*p != 0) {
                            if (stbi_jpeg_get_bit(j))
                                if ((*p & bit) == 0) {
                                    if (*p > 0)
                                        *p += bit;
                                    else
                                        *p -= bit;
                                }
                        }
                        else {
                            if (r == 0) {
                                *p = static_cast<short>(s);
                                break;
                            }
                            --r;
                        }
                    }
                }
                while (k <= j->spec_end);
            }
        }
        return 1;
    }

    // take a -128..127 value and stbi__clamp it and convert to 0..255
    static stbi_inline stbi_uc stbi_clamp(const int x)
    {
        // trick to use a single test to catch both cases
        if (static_cast<unsigned int>(x) > 255) {
            if (x < 0)
                return 0;
            if (x > 255)
                return 255;
        }
        return static_cast<stbi_uc>(x);
    }

  #define stbi__f2f(x) ((int)((x) * 4096 + 0.5))
  #define stbi__fsh(x) ((x) * 4096)

  // derived from jidctint -- DCT_ISLOW
  #define STBI_IDCT_1D(s0, s1, s2, s3, s4, s5, s6, s7)        \
      int t0, t1, t2, t3, p1, p2, p3, p4, p5, x0, x1, x2, x3; \
      p2 = s2;                                                \
      p3 = s6;                                                \
      p1 = (p2 + p3) * stbi__f2f(0.5411961f);                 \
      t2 = p1 + p3 * stbi__f2f(-1.847759065f);                \
      t3 = p1 + p2 * stbi__f2f(0.765366865f);                 \
      p2 = s0;                                                \
      p3 = s4;                                                \
      t0 = stbi__fsh(p2 + p3);                                \
      t1 = stbi__fsh(p2 - p3);                                \
      x0 = t0 + t3;                                           \
      x3 = t0 - t3;                                           \
      x1 = t1 + t2;                                           \
      x2 = t1 - t2;                                           \
      t0 = s7;                                                \
      t1 = s5;                                                \
      t2 = s3;                                                \
      t3 = s1;                                                \
      p3 = t0 + t2;                                           \
      p4 = t1 + t3;                                           \
      p1 = t0 + t3;                                           \
      p2 = t1 + t2;                                           \
      p5 = (p3 + p4) * stbi__f2f(1.175875602f);               \
      t0 = t0 * stbi__f2f(0.298631336f);                      \
      t1 = t1 * stbi__f2f(2.053119869f);                      \
      t2 = t2 * stbi__f2f(3.072711026f);                      \
      t3 = t3 * stbi__f2f(1.501321110f);                      \
      p1 = p5 + p1 * stbi__f2f(-0.899976223f);                \
      p2 = p5 + p2 * stbi__f2f(-2.562915447f);                \
      p3 = p3 * stbi__f2f(-1.961570560f);                     \
      p4 = p4 * stbi__f2f(-0.390180644f);                     \
      t3 += p1 + p4;                                          \
      t2 += p2 + p3;                                          \
      t1 += p2 + p4;                                          \
      t0 += p1 + p3;

    static void stbi_idct_block(stbi_uc* out, int out_stride, short data[64])
    {
        int i, val[64], *v = val;
        stbi_uc* o;
        short* d = data;

        // columns
        for (i = 0; i < 8; ++i, ++d, ++v) {
            // if all zeroes, shortcut -- this avoids dequantizing 0s and IDCTing
            if (d[8] == 0 && d[16] == 0 && d[24] == 0 && d[32] == 0 &&
                d[40] == 0 && d[48] == 0 && d[56] == 0) {
                //    no shortcut                 0     seconds
                //    (1|2|3|4|5|6|7)==0          0     seconds
                //    all separate               -0.047 seconds
                //    1 && 2|3 && 4|5 && 6|7:    -0.047 seconds
                int dcterm = d[0] * 4;
                v[0] = v[8] = v[16] = v[24] = v[32] = v[40] = v[48] = v[56] = dcterm;
            }
            else {
                STBI_IDCT_1D(d[0], d[8], d[16], d[24], d[32], d[40], d[48], d[56])
                // constants scaled things up by 1<<12; let's bring them back
                // down, but keep 2 extra bits of precision
                x0 += 512;
                x1 += 512;
                x2 += 512;
                x3 += 512;
                v[0] = (x0 + t3) >> 10;
                v[56] = (x0 - t3) >> 10;
                v[8] = (x1 + t2) >> 10;
                v[48] = (x1 - t2) >> 10;
                v[16] = (x2 + t1) >> 10;
                v[40] = (x2 - t1) >> 10;
                v[24] = (x3 + t0) >> 10;
                v[32] = (x3 - t0) >> 10;
            }
        }

        for (i = 0, v = val, o = out; i < 8; ++i, v += 8, o += out_stride) {
            // no fast case since the first 1D IDCT spread components out
            STBI_IDCT_1D(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7])
            // constants scaled things up by 1<<12, plus we had 1<<2 from first
            // loop, plus horizontal and vertical each scale by sqrt(8) so together
            // we've got an extra 1<<3, so 1<<17 total we need to remove.
            // so we want to round that, which means adding 0.5 * 1<<17,
            // aka 65536. Also, we'll end up with -128 to 127 that we want
            // to encode as 0..255 by adding 128, so we'll add that before the shift
            x0 += 65536 + (128 << 17);
            x1 += 65536 + (128 << 17);
            x2 += 65536 + (128 << 17);
            x3 += 65536 + (128 << 17);
            // tried computing the shifts into temps, or'ing the temps to see
            // if any were out of range, but that was slower
            o[0] = stbi_clamp((x0 + t3) >> 17);
            o[7] = stbi_clamp((x0 - t3) >> 17);
            o[1] = stbi_clamp((x1 + t2) >> 17);
            o[6] = stbi_clamp((x1 - t2) >> 17);
            o[2] = stbi_clamp((x2 + t1) >> 17);
            o[5] = stbi_clamp((x2 - t1) >> 17);
            o[3] = stbi_clamp((x3 + t0) >> 17);
            o[4] = stbi_clamp((x3 - t0) >> 17);
        }
    }

  #ifdef STBI_SSE2
    // sse2 integer IDCT. not the fastest possible implementation but it
    // produces bit-identical results to the generic C version so it's
    // fully "transparent".
    static void stbi_idct_simd(stbi_uc* out, int out_stride, short data[64])
    {
        // This is constructed to match our regular (generic) integer IDCT exactly.
        __m128i row0, row1, row2, row3, row4, row5, row6, row7;
        __m128i tmp;

    // dot product constant: even elems=x, odd elems=y
    #define dct_const(x, y) _mm_setr_epi16((x), (y), (x), (y), (x), (y), (x), (y))

    // out(0) = c0[even]*x + c0[odd]*y   (c0, x, y 16-bit, out 32-bit)
    // out(1) = c1[even]*x + c1[odd]*y
    #define dct_rot(out0, out1, x, y, c0, c1)          \
        __m128i c0##lo = _mm_unpacklo_epi16((x), (y)); \
        __m128i c0##hi = _mm_unpackhi_epi16((x), (y)); \
        __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
        __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
        __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
        __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

    // out = in << 12  (in 16-bit, out 32-bit)
    #define dct_widen(out, in)                                                              \
        __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
        __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

    // wide add
    #define dct_wadd(out, a, b)                        \
        __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
        __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

    // wide sub
    #define dct_wsub(out, a, b)                        \
        __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
        __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

    // butterfly a/b, add bias, then shift by "s" and pack
    #define dct_bfly32o(out0, out1, a, b, bias, s)                                      \
        {                                                                               \
            __m128i abiased_l = _mm_add_epi32(a##_l, bias);                             \
            __m128i abiased_h = _mm_add_epi32(a##_h, bias);                             \
            dct_wadd(sum, abiased, b);                                                  \
            dct_wsub(dif, abiased, b);                                                  \
            out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
            out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
        }

    // 8-bit interleave step (for transposes)
    #define dct_interleave8(a, b)    \
        tmp = a;                     \
        a = _mm_unpacklo_epi8(a, b); \
        b = _mm_unpackhi_epi8(tmp, b)

    // 16-bit interleave step (for transposes)
    #define dct_interleave16(a, b)    \
        tmp = a;                      \
        a = _mm_unpacklo_epi16(a, b); \
        b = _mm_unpackhi_epi16(tmp, b)

    #define dct_pass(bias, shift)                            \
        {                                                    \
            /* even part */                                  \
            dct_rot(t2e, t3e, row2, row6, rot0_0, rot0_1);   \
            __m128i sum04 = _mm_add_epi16(row0, row4);       \
            __m128i dif04 = _mm_sub_epi16(row0, row4);       \
            dct_widen(t0e, sum04);                           \
            dct_widen(t1e, dif04);                           \
            dct_wadd(x0, t0e, t3e);                          \
            dct_wsub(x3, t0e, t3e);                          \
            dct_wadd(x1, t1e, t2e);                          \
            dct_wsub(x2, t1e, t2e);                          \
            /* odd part */                                   \
            dct_rot(y0o, y2o, row7, row3, rot2_0, rot2_1);   \
            dct_rot(y1o, y3o, row5, row1, rot3_0, rot3_1);   \
            __m128i sum17 = _mm_add_epi16(row1, row7);       \
            __m128i sum35 = _mm_add_epi16(row3, row5);       \
            dct_rot(y4o, y5o, sum17, sum35, rot1_0, rot1_1); \
            dct_wadd(x4, y0o, y4o);                          \
            dct_wadd(x5, y1o, y5o);                          \
            dct_wadd(x6, y2o, y5o);                          \
            dct_wadd(x7, y3o, y4o);                          \
            dct_bfly32o(row0, row7, x0, x7, bias, shift);    \
            dct_bfly32o(row1, row6, x1, x6, bias, shift);    \
            dct_bfly32o(row2, row5, x2, x5, bias, shift);    \
            dct_bfly32o(row3, row4, x3, x4, bias, shift);    \
        }

        __m128i rot0_0 = dct_const(stbi__f2f(0.5411961f),
                                   stbi__f2f(0.5411961f) + stbi__f2f(-1.847759065f));
        __m128i rot0_1 = dct_const(stbi__f2f(0.5411961f) + stbi__f2f(0.765366865f),
                                   stbi__f2f(0.5411961f));
        __m128i rot1_0 = dct_const(stbi__f2f(1.175875602f) + stbi__f2f(-0.899976223f),
                                   stbi__f2f(1.175875602f));
        __m128i rot1_1 = dct_const(stbi__f2f(1.175875602f),
                                   stbi__f2f(1.175875602f) + stbi__f2f(-2.562915447f));
        __m128i rot2_0 = dct_const(stbi__f2f(-1.961570560f) + stbi__f2f(0.298631336f),
                                   stbi__f2f(-1.961570560f));
        __m128i rot2_1 = dct_const(stbi__f2f(-1.961570560f),
                                   stbi__f2f(-1.961570560f) + stbi__f2f(3.072711026f));
        __m128i rot3_0 = dct_const(stbi__f2f(-0.390180644f) + stbi__f2f(2.053119869f),
                                   stbi__f2f(-0.390180644f));
        __m128i rot3_1 = dct_const(stbi__f2f(-0.390180644f),
                                   stbi__f2f(-0.390180644f) + stbi__f2f(1.501321110f));

        // rounding biases in column/row passes, see stbi__idct_block for explanation.
        __m128i bias_0 = _mm_set1_epi32(512);
        __m128i bias_1 = _mm_set1_epi32(65536 + (128 << 17));

        // load
        row0 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 0 * 8));
        row1 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 1 * 8));
        row2 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 2 * 8));
        row3 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 3 * 8));
        row4 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 4 * 8));
        row5 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 5 * 8));
        row6 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 6 * 8));
        row7 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 7 * 8));

        // column pass
        dct_pass(bias_0, 10)

        {
            // 16bit 8x8 transpose pass 1
            dct_interleave16(row0, row4);
            dct_interleave16(row1, row5);
            dct_interleave16(row2, row6);
            dct_interleave16(row3, row7);

            // transpose pass 2
            dct_interleave16(row0, row2);
            dct_interleave16(row1, row3);
            dct_interleave16(row4, row6);
            dct_interleave16(row5, row7);

            // transpose pass 3
            dct_interleave16(row0, row1);
            dct_interleave16(row2, row3);
            dct_interleave16(row4, row5);
            dct_interleave16(row6, row7);
        }

        // row pass
        dct_pass(bias_1, 17)

        {
            // pack
            __m128i p0 = _mm_packus_epi16(row0, row1);  // a0a1a2a3...a7b0b1b2b3...b7
            __m128i p1 = _mm_packus_epi16(row2, row3);
            __m128i p2 = _mm_packus_epi16(row4, row5);
            __m128i p3 = _mm_packus_epi16(row6, row7);

            // 8bit 8x8 transpose pass 1
            dct_interleave8(p0, p2);  // a0e0a1e1...
            dct_interleave8(p1, p3);  // c0g0c1g1...

            // transpose pass 2
            dct_interleave8(p0, p1);  // a0c0e0g0...
            dct_interleave8(p2, p3);  // b0d0f0h0...

            // transpose pass 3
            dct_interleave8(p0, p2);  // a0b0c0d0...
            dct_interleave8(p1, p3);  // a4b4c4d4...

            // store
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), p0);
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), _mm_shuffle_epi32(p0, 0x4e));
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), p2);
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), _mm_shuffle_epi32(p2, 0x4e));
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), p1);
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), _mm_shuffle_epi32(p1, 0x4e));
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), p3);
            out += out_stride;
            _mm_storel_epi64(reinterpret_cast<__m128i*>(out), _mm_shuffle_epi32(p3, 0x4e));
        }

    #undef dct_const
    #undef dct_rot
    #undef dct_widen
    #undef dct_wadd
    #undef dct_wsub
    #undef dct_bfly32o
    #undef dct_interleave8
    #undef dct_interleave16
    #undef dct_pass
    }

  #endif  // STBI_SSE2

  #ifdef STBI_NEON

    // NEON integer IDCT. should produce bit-identical
    // results to the generic C version.
    static void stbi__idct_simd(stbi_uc* out, int out_stride, short data[64])
    {
        int16x8_t row0, row1, row2, row3, row4, row5, row6, row7;

        int16x4_t rot0_0 = vdup_n_s16(stbi__f2f(0.5411961f));
        int16x4_t rot0_1 = vdup_n_s16(stbi__f2f(-1.847759065f));
        int16x4_t rot0_2 = vdup_n_s16(stbi__f2f(0.765366865f));
        int16x4_t rot1_0 = vdup_n_s16(stbi__f2f(1.175875602f));
        int16x4_t rot1_1 = vdup_n_s16(stbi__f2f(-0.899976223f));
        int16x4_t rot1_2 = vdup_n_s16(stbi__f2f(-2.562915447f));
        int16x4_t rot2_0 = vdup_n_s16(stbi__f2f(-1.961570560f));
        int16x4_t rot2_1 = vdup_n_s16(stbi__f2f(-0.390180644f));
        int16x4_t rot3_0 = vdup_n_s16(stbi__f2f(0.298631336f));
        int16x4_t rot3_1 = vdup_n_s16(stbi__f2f(2.053119869f));
        int16x4_t rot3_2 = vdup_n_s16(stbi__f2f(3.072711026f));
        int16x4_t rot3_3 = vdup_n_s16(stbi__f2f(1.501321110f));

    #define dct_long_mul(out, inq, coeff)                        \
        int32x4_t out##_l = vmull_s16(vget_low_s16(inq), coeff); \
        int32x4_t out##_h = vmull_s16(vget_high_s16(inq), coeff)

    #define dct_long_mac(out, acc, inq, coeff)                            \
        int32x4_t out##_l = vmlal_s16(acc##_l, vget_low_s16(inq), coeff); \
        int32x4_t out##_h = vmlal_s16(acc##_h, vget_high_s16(inq), coeff)

    #define dct_widen(out, inq)                                 \
        int32x4_t out##_l = vshll_n_s16(vget_low_s16(inq), 12); \
        int32x4_t out##_h = vshll_n_s16(vget_high_s16(inq), 12)

    // wide add
    #define dct_wadd(out, a, b)                      \
        int32x4_t out##_l = vaddq_s32(a##_l, b##_l); \
        int32x4_t out##_h = vaddq_s32(a##_h, b##_h)

    // wide sub
    #define dct_wsub(out, a, b)                      \
        int32x4_t out##_l = vsubq_s32(a##_l, b##_l); \
        int32x4_t out##_h = vsubq_s32(a##_h, b##_h)

    // butterfly a/b, then shift using "shiftop" by "s" and pack
    #define dct_bfly32o(out0, out1, a, b, shiftop, s)                  \
        {                                                              \
            dct_wadd(sum, a, b);                                       \
            dct_wsub(dif, a, b);                                       \
            out0 = vcombine_s16(shiftop(sum_l, s), shiftop(sum_h, s)); \
            out1 = vcombine_s16(shiftop(dif_l, s), shiftop(dif_h, s)); \
        }

    #define dct_pass(shiftop, shift)                         \
        {                                                    \
            /* even part */                                  \
            int16x8_t sum26 = vaddq_s16(row2, row6);         \
            dct_long_mul(p1e, sum26, rot0_0);                \
            dct_long_mac(t2e, p1e, row6, rot0_1);            \
            dct_long_mac(t3e, p1e, row2, rot0_2);            \
            int16x8_t sum04 = vaddq_s16(row0, row4);         \
            int16x8_t dif04 = vsubq_s16(row0, row4);         \
            dct_widen(t0e, sum04);                           \
            dct_widen(t1e, dif04);                           \
            dct_wadd(x0, t0e, t3e);                          \
            dct_wsub(x3, t0e, t3e);                          \
            dct_wadd(x1, t1e, t2e);                          \
            dct_wsub(x2, t1e, t2e);                          \
            /* odd part */                                   \
            int16x8_t sum15 = vaddq_s16(row1, row5);         \
            int16x8_t sum17 = vaddq_s16(row1, row7);         \
            int16x8_t sum35 = vaddq_s16(row3, row5);         \
            int16x8_t sum37 = vaddq_s16(row3, row7);         \
            int16x8_t sumodd = vaddq_s16(sum17, sum35);      \
            dct_long_mul(p5o, sumodd, rot1_0);               \
            dct_long_mac(p1o, p5o, sum17, rot1_1);           \
            dct_long_mac(p2o, p5o, sum35, rot1_2);           \
            dct_long_mul(p3o, sum37, rot2_0);                \
            dct_long_mul(p4o, sum15, rot2_1);                \
            dct_wadd(sump13o, p1o, p3o);                     \
            dct_wadd(sump24o, p2o, p4o);                     \
            dct_wadd(sump23o, p2o, p3o);                     \
            dct_wadd(sump14o, p1o, p4o);                     \
            dct_long_mac(x4, sump13o, row7, rot3_0);         \
            dct_long_mac(x5, sump24o, row5, rot3_1);         \
            dct_long_mac(x6, sump23o, row3, rot3_2);         \
            dct_long_mac(x7, sump14o, row1, rot3_3);         \
            dct_bfly32o(row0, row7, x0, x7, shiftop, shift); \
            dct_bfly32o(row1, row6, x1, x6, shiftop, shift); \
            dct_bfly32o(row2, row5, x2, x5, shiftop, shift); \
            dct_bfly32o(row3, row4, x3, x4, shiftop, shift); \
        }

        // load
        row0 = vld1q_s16(data + 0 * 8);
        row1 = vld1q_s16(data + 1 * 8);
        row2 = vld1q_s16(data + 2 * 8);
        row3 = vld1q_s16(data + 3 * 8);
        row4 = vld1q_s16(data + 4 * 8);
        row5 = vld1q_s16(data + 5 * 8);
        row6 = vld1q_s16(data + 6 * 8);
        row7 = vld1q_s16(data + 7 * 8);

        // add DC bias
        row0 = vaddq_s16(row0, vsetq_lane_s16(1024, vdupq_n_s16(0), 0));

        // column pass
        dct_pass(vrshrn_n_s32, 10);

        // 16bit 8x8 transpose
        {
    // these three map to a single VTRN.16, VTRN.32, and VSWP, respectively.
    // whether compilers actually get this is another story, sadly.
    #define dct_trn16(x, y)                  \
        {                                    \
            int16x8x2_t t = vtrnq_s16(x, y); \
            x = t.val[0];                    \
            y = t.val[1];                    \
        }
    #define dct_trn32(x, y)                                                                \
        {                                                                                  \
            int32x4x2_t t = vtrnq_s32(vreinterpretq_s32_s16(x), vreinterpretq_s32_s16(y)); \
            x = vreinterpretq_s16_s32(t.val[0]);                                           \
            y = vreinterpretq_s16_s32(t.val[1]);                                           \
        }
    #define dct_trn64(x, y)                                         \
        {                                                           \
            int16x8_t x0 = x;                                       \
            int16x8_t y0 = y;                                       \
            x = vcombine_s16(vget_low_s16(x0), vget_low_s16(y0));   \
            y = vcombine_s16(vget_high_s16(x0), vget_high_s16(y0)); \
        }

            // pass 1
            dct_trn16(row0, row1);  // a0b0a2b2a4b4a6b6
            dct_trn16(row2, row3);
            dct_trn16(row4, row5);
            dct_trn16(row6, row7);

            // pass 2
            dct_trn32(row0, row2);  // a0b0c0d0a4b4c4d4
            dct_trn32(row1, row3);
            dct_trn32(row4, row6);
            dct_trn32(row5, row7);

            // pass 3
            dct_trn64(row0, row4);  // a0b0c0d0e0f0g0h0
            dct_trn64(row1, row5);
            dct_trn64(row2, row6);
            dct_trn64(row3, row7);

    #undef dct_trn16
    #undef dct_trn32
    #undef dct_trn64
        }

        // row pass
        // vrshrn_n_s32 only supports shifts up to 16, we need
        // 17. so do a non-rounding shift of 16 first then follow
        // up with a rounding shift by 1.
        dct_pass(vshrn_n_s32, 16);

        {
            // pack and round
            uint8x8_t p0 = vqrshrun_n_s16(row0, 1);
            uint8x8_t p1 = vqrshrun_n_s16(row1, 1);
            uint8x8_t p2 = vqrshrun_n_s16(row2, 1);
            uint8x8_t p3 = vqrshrun_n_s16(row3, 1);
            uint8x8_t p4 = vqrshrun_n_s16(row4, 1);
            uint8x8_t p5 = vqrshrun_n_s16(row5, 1);
            uint8x8_t p6 = vqrshrun_n_s16(row6, 1);
            uint8x8_t p7 = vqrshrun_n_s16(row7, 1);

                // again, these can translate into one instruction, but often don't.
    #define dct_trn8_8(x, y)               \
        {                                  \
            uint8x8x2_t t = vtrn_u8(x, y); \
            x = t.val[0];                  \
            y = t.val[1];                  \
        }
    #define dct_trn8_16(x, y)                                                          \
        {                                                                              \
            uint16x4x2_t t = vtrn_u16(vreinterpret_u16_u8(x), vreinterpret_u16_u8(y)); \
            x = vreinterpret_u8_u16(t.val[0]);                                         \
            y = vreinterpret_u8_u16(t.val[1]);                                         \
        }
    #define dct_trn8_32(x, y)                                                          \
        {                                                                              \
            uint32x2x2_t t = vtrn_u32(vreinterpret_u32_u8(x), vreinterpret_u32_u8(y)); \
            x = vreinterpret_u8_u32(t.val[0]);                                         \
            y = vreinterpret_u8_u32(t.val[1]);                                         \
        }

            // sadly can't use interleaved stores here since we only write
            // 8 bytes to each scan line!

            // 8x8 8-bit transpose pass 1
            dct_trn8_8(p0, p1);
            dct_trn8_8(p2, p3);
            dct_trn8_8(p4, p5);
            dct_trn8_8(p6, p7);

            // pass 2
            dct_trn8_16(p0, p2);
            dct_trn8_16(p1, p3);
            dct_trn8_16(p4, p6);
            dct_trn8_16(p5, p7);

            // pass 3
            dct_trn8_32(p0, p4);
            dct_trn8_32(p1, p5);
            dct_trn8_32(p2, p6);
            dct_trn8_32(p3, p7);

            // store
            vst1_u8(out, p0);
            out += out_stride;
            vst1_u8(out, p1);
            out += out_stride;
            vst1_u8(out, p2);
            out += out_stride;
            vst1_u8(out, p3);
            out += out_stride;
            vst1_u8(out, p4);
            out += out_stride;
            vst1_u8(out, p5);
            out += out_stride;
            vst1_u8(out, p6);
            out += out_stride;
            vst1_u8(out, p7);

    #undef dct_trn8_8
    #undef dct_trn8_16
    #undef dct_trn8_32
        }

    #undef dct_long_mul
    #undef dct_long_mac
    #undef dct_widen
    #undef dct_wadd
    #undef dct_wsub
    #undef dct_bfly32o
    #undef dct_pass
    }

  #endif  // STBI_NEON

  #define STBI_MARKER_none 0xff

    // if there's a pending marker from the entropy stream, return that
    // otherwise, fetch from the stream and get a marker. if there's no
    // marker, return 0xff, which is never a valid marker value
    static stbi_uc stbi_get_marker(stbi_jpeg* j)
    {
        stbi_uc x;
        if (j->marker != STBI_MARKER_none) {
            x = j->marker;
            j->marker = STBI_MARKER_none;
            return x;
        }
        x = stbi_get8(j->s);
        if (x != 0xff)
            return STBI_MARKER_none;
        while (x == 0xff)
            x = stbi_get8(j->s);  // consume repeated 0xff fill bytes
        return x;
    }

  // in each scan, we'll have scan_n components, and the order
  // of the components is specified by order[]
  #define STBI_RESTART(x) ((x) >= 0xd0 && (x) <= 0xd7)

    // after a restart interval, stbi__jpeg_reset the entropy decoder and
    // the dc prediction
    static void stbi_jpeg_reset(stbi_jpeg* j)
    {
        j->code_bits = 0;
        j->code_buffer = 0;
        j->nomore = 0;
        j->img_comp[0].dc_pred =
            j->img_comp[1].dc_pred =
                j->img_comp[2].dc_pred =
                    j->img_comp[3].dc_pred = 0;
        j->marker = STBI_MARKER_none;
        j->todo = j->restart_interval ? j->restart_interval : 0x7fffffff;
        j->eob_run = 0;
        // no more than 1<<31 MCUs if no restart_interal? that's plenty safe,
        // since we don't even allow 1<<30 pixels
    }

    static int stbi_parse_entropy_coded_data(stbi_jpeg* z)
    {
        stbi_jpeg_reset(z);
        if (!z->progressive) {
            if (z->scan_n == 1) {
                STBI_SIMD_ALIGN(short, data[64]);
                const int n = z->order[0];
                // non-interleaved data, we just need to process one block at a time,
                // in trivial scanline order
                // number of blocks to do just depends on how many actual "pixels" this
                // component has, independent of interleaved MCU blocking and such
                const int w = (z->img_comp[n].x + 7) >> 3;
                const int h = (z->img_comp[n].y + 7) >> 3;
                for (int j = 0; j < h; ++j) {
                    for (int i = 0; i < w; ++i) {
                        const int ha = z->img_comp[n].ha;
                        if (!stbi_jpeg_decode_block(z, data, z->huff_dc + z->img_comp[n].hd,
                                                    z->huff_ac + ha, z->fast_ac[ha], n,
                                                    z->dequant[z->img_comp[n].tq]))
                            return 0;
                        z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2 * j * 8 + i * 8,
                                             z->img_comp[n].w2, data);
                        // every data block is an MCU, so countdown the restart interval
                        if (--z->todo <= 0) {
                            if (z->code_bits < 24)
                                stbi_grow_buffer_unsafe(z);
                            // if it's NOT a restart, then just bail, so we get corrupt data
                            // rather than no data
                            if (!STBI_RESTART(z->marker))
                                return 1;
                            stbi_jpeg_reset(z);
                        }
                    }
                }
                return 1;
            }
            else {  // interleaved
                STBI_SIMD_ALIGN(short, data[64]);
                for (int j = 0; j < z->img_mcu_y; ++j) {
                    for (int i = 0; i < z->img_mcu_x; ++i) {
                        // scan an interleaved mcu... process scan_n components in order
                        for (int k = 0; k < z->scan_n; ++k) {
                            const int n = z->order[k];
                            // scan out an mcu's worth of this component; that's just determined
                            // by the basic H and V specified for the component
                            for (int y = 0; y < z->img_comp[n].v; ++y) {
                                for (int x = 0; x < z->img_comp[n].h; ++x) {
                                    const int x2 = (i * z->img_comp[n].h + x) * 8;
                                    const int y2 = (j * z->img_comp[n].v + y) * 8;
                                    const int ha = z->img_comp[n].ha;
                                    if (!stbi_jpeg_decode_block(z, data,
                                                                z->huff_dc + z->img_comp[n].hd,
                                                                z->huff_ac + ha, z->fast_ac[ha], n,
                                                                z->dequant[z->img_comp[n].tq]))
                                        return 0;
                                    z->idct_block_kernel(
                                        z->img_comp[n].data + z->img_comp[n].w2 * y2 + x2,
                                        z->img_comp[n].w2, data);
                                }
                            }
                        }
                        // after all interleaved components, that's an interleaved MCU,
                        // so now count down the restart interval
                        if (--z->todo <= 0) {
                            if (z->code_bits < 24)
                                stbi_grow_buffer_unsafe(z);
                            if (!STBI_RESTART(z->marker))
                                return 1;
                            stbi_jpeg_reset(z);
                        }
                    }
                }
                return 1;
            }
        }
        else if (z->scan_n == 1) {
            const int n = z->order[0];
            // non-interleaved data, we just need to process one block at a time,
            // in trivial scanline order
            // number of blocks to do just depends on how many actual "pixels" this
            // component has, independent of interleaved MCU blocking and such
            const int w = (z->img_comp[n].x + 7) >> 3;
            const int h = (z->img_comp[n].y + 7) >> 3;
            for (int j = 0; j < h; ++j) {
                for (int i = 0; i < w; ++i) {
                    short* data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
                    if (z->spec_start == 0) {
                        if (!stbi_jpeg_decode_block_prog_dc(z, data, &z->huff_dc[z->img_comp[n].hd],
                                                            n))
                            return 0;
                    }
                    else {
                        const int ha = z->img_comp[n].ha;
                        if (!stbi_jpeg_decode_block_prog_ac(z, data, &z->huff_ac[ha],
                                                            z->fast_ac[ha]))
                            return 0;
                    }
                    // every data block is an MCU, so countdown the restart interval
                    if (--z->todo <= 0) {
                        if (z->code_bits < 24)
                            stbi_grow_buffer_unsafe(z);
                        if (!STBI_RESTART(z->marker))
                            return 1;
                        stbi_jpeg_reset(z);
                    }
                }
            }
            return 1;
        }
        else {  // interleaved
            for (int j = 0; j < z->img_mcu_y; ++j) {
                for (int i = 0; i < z->img_mcu_x; ++i) {
                    // scan an interleaved mcu... process scan_n components in order
                    for (int k = 0; k < z->scan_n; ++k) {
                        const int n = z->order[k];
                        // scan out an mcu's worth of this component; that's just determined
                        // by the basic H and V specified for the component
                        for (int y = 0; y < z->img_comp[n].v; ++y) {
                            for (int x = 0; x < z->img_comp[n].h; ++x) {
                                const int x2 = (i * z->img_comp[n].h + x);
                                const int y2 = (j * z->img_comp[n].v + y);
                                short* data = z->img_comp[n].coeff +
                                              64 * (x2 + y2 * z->img_comp[n].coeff_w);
                                if (!stbi_jpeg_decode_block_prog_dc(
                                        z, data, &z->huff_dc[z->img_comp[n].hd], n))
                                    return 0;
                            }
                        }
                    }
                    // after all interleaved components, that's an interleaved MCU,
                    // so now count down the restart interval
                    if (--z->todo <= 0) {
                        if (z->code_bits < 24)
                            stbi_grow_buffer_unsafe(z);
                        if (!STBI_RESTART(z->marker))
                            return 1;
                        stbi_jpeg_reset(z);
                    }
                }
            }
            return 1;
        }
    }

    static void stbi_jpeg_dequantize(short* data, const StbiUint16* dequant)
    {
        for (int i = 0; i < 64; ++i)
            data[i] *= dequant[i];
    }

    static void stbi_jpeg_finish(stbi_jpeg* z)
    {
        if (z->progressive) {
            // dequantize and idct the data
            for (int n = 0; n < z->s->img_n; ++n) {
                const int w = (z->img_comp[n].x + 7) >> 3;
                const int h = (z->img_comp[n].y + 7) >> 3;
                for (int j = 0; j < h; ++j) {
                    for (int i = 0; i < w; ++i) {
                        short* data = z->img_comp[n].coeff + 64 * (i + j * z->img_comp[n].coeff_w);
                        stbi_jpeg_dequantize(data, z->dequant[z->img_comp[n].tq]);
                        z->idct_block_kernel(z->img_comp[n].data + z->img_comp[n].w2 * j * 8 + i * 8,
                                             z->img_comp[n].w2, data);
                    }
                }
            }
        }
    }

    static int stbi_process_marker(stbi_jpeg* z, const int m)
    {
        int L;
        switch (m) {
            case STBI_MARKER_none:  // no marker found
                return stbi__err("expected marker", "Corrupt JPEG");

            case 0xDD:  // DRI - specify restart interval
                if (stbi_get16be(z->s) != 4)
                    return stbi__err("bad DRI len", "Corrupt JPEG");
                z->restart_interval = stbi_get16be(z->s);
                return 1;

            case 0xDB:  // DQT - define quantization table
                L = stbi_get16be(z->s) - 2;
                while (L > 0) {
                    const int q = stbi_get8(z->s);
                    int p = q >> 4, sixteen = (p != 0);
                    const int t = q & 15;
                    if (p != 0 && p != 1)
                        return stbi__err("bad DQT type", "Corrupt JPEG");
                    if (t > 3)
                        return stbi__err("bad DQT table", "Corrupt JPEG");

                    for (int i = 0; i < 64; ++i)
                        z->dequant[t][stbi_jpeg_dezigzag[i]] = static_cast<StbiUint16>(
                            sixteen ? stbi_get16be(z->s) : stbi_get8(z->s));
                    L -= (sixteen ? 129 : 65);
                }
                return L == 0;

            case 0xC4:  // DHT - define huffman table
                L = stbi_get16be(z->s) - 2;
                while (L > 0) {
                    stbi_uc* v;
                    int sizes[16], i, n = 0;
                    const int q = stbi_get8(z->s);
                    const int tc = q >> 4;
                    const int th = q & 15;
                    if (tc > 1 || th > 3)
                        return stbi__err("bad DHT header", "Corrupt JPEG");
                    for (i = 0; i < 16; ++i) {
                        sizes[i] = stbi_get8(z->s);
                        n += sizes[i];
                    }
                    if (n > 256)
                        return stbi__err("bad DHT header", "Corrupt JPEG");  // Loop over i < n
                                                                             // would write past end
                                                                             // of values!
                    L -= 17;
                    if (tc == 0) {
                        if (!stbi_build_huffman(z->huff_dc + th, sizes))
                            return 0;
                        v = z->huff_dc[th].values;
                    }
                    else {
                        if (!stbi_build_huffman(z->huff_ac + th, sizes))
                            return 0;
                        v = z->huff_ac[th].values;
                    }
                    for (i = 0; i < n; ++i)
                        v[i] = stbi_get8(z->s);
                    if (tc != 0)
                        stbi_build_fast_ac(z->fast_ac[th], z->huff_ac + th);
                    L -= n;
                }
                return L == 0;
        }

        // check for comment block or APP blocks
        if ((m >= 0xE0 && m <= 0xEF) || m == 0xFE) {
            L = stbi_get16be(z->s);
            if (L < 2) {
                if (m == 0xFE)
                    return stbi__err("bad COM len", "Corrupt JPEG");
                else
                    return stbi__err("bad APP len", "Corrupt JPEG");
            }
            L -= 2;

            if (m == 0xE0 && L >= 5) {  // JFIF APP0 segment
                static const unsigned char tag[5] = { 'J', 'F', 'I', 'F', '\0' };
                int ok = 1;
                for (int i = 0; i < 5; ++i)
                    if (stbi_get8(z->s) != tag[i])
                        ok = 0;
                L -= 5;
                if (ok)
                    z->jfif = 1;
            }
            else if (m == 0xEE && L >= 12) {  // Adobe APP14 segment
                static const unsigned char tag[6] = { 'A', 'd', 'o', 'b', 'e', '\0' };
                int ok = 1;
                for (int i = 0; i < 6; ++i)
                    if (stbi_get8(z->s) != tag[i])
                        ok = 0;
                L -= 6;
                if (ok) {
                    stbi_get8(z->s);                             // version
                    stbi_get16be(z->s);                          // flags0
                    stbi_get16be(z->s);                          // flags1
                    z->app14_color_transform = stbi_get8(z->s);  // color transform
                    L -= 6;
                }
            }

            stbi_skip(z->s, L);
            return 1;
        }

        return stbi__err("unknown marker", "Corrupt JPEG");
    }

    // after we see SOS
    static int stbi_process_scan_header(stbi_jpeg* z)
    {
        const int Ls = stbi_get16be(z->s);
        z->scan_n = stbi_get8(z->s);
        if (z->scan_n < 1 || z->scan_n > 4 || z->scan_n > (int)z->s->img_n)
            return stbi__err("bad SOS component count", "Corrupt JPEG");
        if (Ls != 6 + 2 * z->scan_n)
            return stbi__err("bad SOS len", "Corrupt JPEG");
        for (int i = 0; i < z->scan_n; ++i) {
            int id = stbi_get8(z->s), which;
            const int q = stbi_get8(z->s);
            for (which = 0; which < z->s->img_n; ++which)
                if (z->img_comp[which].id == id)
                    break;
            if (which == z->s->img_n)
                return 0;  // no match
            z->img_comp[which].hd = q >> 4;
            if (z->img_comp[which].hd > 3)
                return stbi__err("bad DC huff", "Corrupt JPEG");
            z->img_comp[which].ha = q & 15;
            if (z->img_comp[which].ha > 3)
                return stbi__err("bad AC huff", "Corrupt JPEG");
            z->order[i] = which;
        }

        {
            z->spec_start = stbi_get8(z->s);
            z->spec_end = stbi_get8(z->s);  // should be 63, but might be 0
            const int aa = stbi_get8(z->s);
            z->succ_high = (aa >> 4);
            z->succ_low = (aa & 15);
            if (z->progressive) {
                if (z->spec_start > 63 || z->spec_end > 63 || z->spec_start > z->spec_end ||
                    z->succ_high > 13 || z->succ_low > 13)
                    return stbi__err("bad SOS", "Corrupt JPEG");
            }
            else {
                if (z->spec_start != 0)
                    return stbi__err("bad SOS", "Corrupt JPEG");
                if (z->succ_high != 0 || z->succ_low != 0)
                    return stbi__err("bad SOS", "Corrupt JPEG");
                z->spec_end = 63;
            }
        }

        return 1;
    }

    static int stbi_free_jpeg_components(stbi_jpeg* z, const int ncomp, const int why)
    {
        for (int i = 0; i < ncomp; ++i) {
            if (z->img_comp[i].raw_data) {
                STBI_FREE(z->img_comp[i].raw_data);
                z->img_comp[i].raw_data = nullptr;
                z->img_comp[i].data = nullptr;
            }
            if (z->img_comp[i].raw_coeff) {
                STBI_FREE(z->img_comp[i].raw_coeff);
                z->img_comp[i].raw_coeff = 0;
                z->img_comp[i].coeff = 0;
            }
            if (z->img_comp[i].linebuf) {
                STBI_FREE(z->img_comp[i].linebuf);
                z->img_comp[i].linebuf = nullptr;
            }
        }
        return why;
    }

    static int stbi_process_frame_header(stbi_jpeg* z, const int scan)
    {
        StbiContext* s = z->s;
        int i, h_max = 1, v_max = 1;
        const int Lf = stbi_get16be(s);
        if (Lf < 11)
            return stbi__err("bad SOF len", "Corrupt JPEG");  // JPEG
        const int p = stbi_get8(s);
        if (p != 8)
            return stbi__err("only 8-bit", "JPEG format not supported: 8-bit only");  // JPEG
                                                                                      // baseline
        s->img_y = stbi_get16be(s);
        if (s->img_y == 0)
            return stbi__err("no header height",
                             "JPEG format not supported: delayed height");  // Legal, but we don't
                                                                            // handle it--but
                                                                            // neither does IJG
        s->img_x = stbi_get16be(s);
        if (s->img_x == 0)
            return stbi__err("0 width", "Corrupt JPEG");  // JPEG requires
        if (s->img_y > STBI_MAX_DIMENSIONS)
            return stbi__err("too large", "Very large image (corrupt?)");
        if (s->img_x > STBI_MAX_DIMENSIONS)
            return stbi__err("too large", "Very large image (corrupt?)");
        const int c = stbi_get8(s);
        if (c != 3 && c != 1 && c != 4)
            return stbi__err("bad component count", "Corrupt JPEG");
        s->img_n = c;
        for (i = 0; i < c; ++i) {
            z->img_comp[i].data = nullptr;
            z->img_comp[i].linebuf = nullptr;
        }

        if (Lf != 8 + 3 * s->img_n)
            return stbi__err("bad SOF len", "Corrupt JPEG");

        z->rgb = 0;
        for (i = 0; i < s->img_n; ++i) {
            static const unsigned char rgb[3] = { 'R', 'G', 'B' };
            z->img_comp[i].id = stbi_get8(s);
            if (s->img_n == 3 && z->img_comp[i].id == rgb[i])
                ++z->rgb;
            const int q = stbi_get8(s);
            z->img_comp[i].h = (q >> 4);
            if (!z->img_comp[i].h || z->img_comp[i].h > 4)
                return stbi__err("bad H", "Corrupt JPEG");
            z->img_comp[i].v = q & 15;
            if (!z->img_comp[i].v || z->img_comp[i].v > 4)
                return stbi__err("bad V", "Corrupt JPEG");
            z->img_comp[i].tq = stbi_get8(s);
            if (z->img_comp[i].tq > 3)
                return stbi__err("bad TQ", "Corrupt JPEG");
        }

        if (scan != STBI_SCAN_load)
            return 1;

        if (!stbi_mad3sizes_valid(s->img_x, s->img_y, s->img_n, 0))
            return stbi__err("too large", "Image too large to decode");

        for (i = 0; i < s->img_n; ++i) {
            if (z->img_comp[i].h > h_max)
                h_max = z->img_comp[i].h;
            if (z->img_comp[i].v > v_max)
                v_max = z->img_comp[i].v;
        }

        // check that plane subsampling factors are integer ratios; our resamplers can't deal with
        // fractional ratios and I've never seen a non-corrupted JPEG file actually use them
        for (i = 0; i < s->img_n; ++i) {
            if (h_max % z->img_comp[i].h != 0)
                return stbi__err("bad H", "Corrupt JPEG");
            if (v_max % z->img_comp[i].v != 0)
                return stbi__err("bad V", "Corrupt JPEG");
        }

        // compute interleaved mcu info
        z->img_h_max = h_max;
        z->img_v_max = v_max;
        z->img_mcu_w = h_max * 8;
        z->img_mcu_h = v_max * 8;
        // these sizes can't be more than 17 bits
        z->img_mcu_x = (s->img_x + z->img_mcu_w - 1) / z->img_mcu_w;
        z->img_mcu_y = (s->img_y + z->img_mcu_h - 1) / z->img_mcu_h;

        for (i = 0; i < s->img_n; ++i) {
            // number of effective pixels (e.g. for non-interleaved MCU)
            z->img_comp[i].x = (s->img_x * z->img_comp[i].h + h_max - 1) / h_max;
            z->img_comp[i].y = (s->img_y * z->img_comp[i].v + v_max - 1) / v_max;
            // to simplify generation, we'll allocate enough memory to decode
            // the bogus oversized data from using interleaved MCUs and their
            // big blocks (e.g. a 16x16 iMCU on an image of width 33); we won't
            // discard the extra data until colorspace conversion
            //
            // img_mcu_x, img_mcu_y: <=17 bits; comp[i].h and .v are <=4 (checked earlier)
            // so these muls can't overflow with 32-bit ints (which we require)
            z->img_comp[i].w2 = z->img_mcu_x * z->img_comp[i].h * 8;
            z->img_comp[i].h2 = z->img_mcu_y * z->img_comp[i].v * 8;
            z->img_comp[i].coeff = 0;
            z->img_comp[i].raw_coeff = 0;
            z->img_comp[i].linebuf = nullptr;
            z->img_comp[i].raw_data = stbi_malloc_mad2(z->img_comp[i].w2, z->img_comp[i].h2, 15);
            if (z->img_comp[i].raw_data == nullptr)
                return stbi_free_jpeg_components(z, i + 1, stbi__err("outofmem", "Out of memory"));
            // align blocks for idct using mmx/sse
            z->img_comp[i].data = (stbi_uc*)(((size_t)z->img_comp[i].raw_data + 15) & ~15);
            if (z->progressive) {
                // w2, h2 are multiples of 8 (see above)
                z->img_comp[i].coeff_w = z->img_comp[i].w2 / 8;
                z->img_comp[i].coeff_h = z->img_comp[i].h2 / 8;
                z->img_comp[i].raw_coeff = stbi_malloc_mad3(z->img_comp[i].w2, z->img_comp[i].h2,
                                                            sizeof(short), 15);
                if (z->img_comp[i].raw_coeff == nullptr)
                    return stbi_free_jpeg_components(z, i + 1,
                                                     stbi__err("outofmem", "Out of memory"));
                z->img_comp[i].coeff = (short*)(((size_t)z->img_comp[i].raw_coeff + 15) & ~15);
            }
        }

        return 1;
    }

  // use comparisons since in some cases we handle more than one case (e.g. SOF)
  #define stbi_DNL(x) ((x) == 0xdc)
  #define stbi_SOI(x) ((x) == 0xd8)
  #define stbi_EOI(x) ((x) == 0xd9)
  #define stbi_SOF(x) ((x) == 0xc0 || (x) == 0xc1 || (x) == 0xc2)
  #define stbi_SOS(x) ((x) == 0xda)

  #define stbi_SOF_progressive(x) ((x) == 0xc2)

    static int stbi_decode_jpeg_header(stbi_jpeg* z, const int scan)
    {
        z->jfif = 0;
        z->app14_color_transform = -1;  // valid values are 0,1,2
        z->marker = STBI_MARKER_none;   // initialize cached marker to empty
        int m = stbi_get_marker(z);
        if (!stbi_SOI(m))
            return stbi__err("no SOI", "Corrupt JPEG");
        if (scan == STBI_SCAN_type)
            return 1;
        m = stbi_get_marker(z);
        while (!stbi_SOF(m)) {
            if (!stbi_process_marker(z, m))
                return 0;
            m = stbi_get_marker(z);
            while (m == STBI_MARKER_none) {
                // some files have extra padding after their blocks, so ok, we'll scan
                if (stbi_at_eof(z->s))
                    return stbi__err("no SOF", "Corrupt JPEG");
                m = stbi_get_marker(z);
            }
        }
        z->progressive = stbi_SOF_progressive(m);
        if (!stbi_process_frame_header(z, scan))
            return 0;
        return 1;
    }

    static stbi_uc stbi_skip_jpeg_junk_at_end(const stbi_jpeg* j)
    {
        // some JPEGs have junk at end, skip over it but if we find what looks
        // like a valid marker, resume there
        while (!stbi_at_eof(j->s)) {
            stbi_uc x = stbi_get8(j->s);
            while (x == 0xff) {  // might be a marker
                if (stbi_at_eof(j->s))
                    return STBI_MARKER_none;
                x = stbi_get8(j->s);
                if (x != 0x00 && x != 0xff) {
                    // not a stuffed zero or lead-in to another marker, looks
                    // like an actual marker, return it
                    return x;
                }
                // stuffed zero has x=0 now which ends the loop, meaning we go
                // back to regular scan loop.
                // repeated 0xff keeps trying to read the next byte of the marker.
            }
        }
        return STBI_MARKER_none;
    }

    // decode image to YCbCr format
    static int stbi_decode_jpeg_image(stbi_jpeg* j)
    {
        int m;
        for (m = 0; m < 4; m++) {
            j->img_comp[m].raw_data = nullptr;
            j->img_comp[m].raw_coeff = nullptr;
        }
        j->restart_interval = 0;
        if (!stbi_decode_jpeg_header(j, STBI_SCAN_load))
            return 0;
        m = stbi_get_marker(j);
        while (!stbi_EOI(m)) {
            if (stbi_SOS(m)) {
                if (!stbi_process_scan_header(j))
                    return 0;
                if (!stbi_parse_entropy_coded_data(j))
                    return 0;
                if (j->marker == STBI_MARKER_none) {
                    j->marker = stbi_skip_jpeg_junk_at_end(j);
                    // if we reach eof without hitting a marker, stbi__get_marker() below will fail
                    // and we'll eventually return 0
                }
                m = stbi_get_marker(j);
                if (STBI_RESTART(m))
                    m = stbi_get_marker(j);
            }
            else if (stbi_DNL(m)) {
                const int Ld = stbi_get16be(j->s);
                const StbiUint32 NL = stbi_get16be(j->s);
                if (Ld != 4)
                    return stbi__err("bad DNL len", "Corrupt JPEG");
                if (NL != j->s->img_y)
                    return stbi__err("bad DNL height", "Corrupt JPEG");
                m = stbi_get_marker(j);
            }
            else {
                if (!stbi_process_marker(j, m))
                    return 1;
                m = stbi_get_marker(j);
            }
        }
        if (j->progressive)
            stbi_jpeg_finish(j);
        return 1;
    }

    // static jfif-centered resampling (across block boundaries)

    typedef stbi_uc* (*resample_row_func)(stbi_uc* out, stbi_uc* in0, stbi_uc* in1, int w, int hs);

  #define stbi_div4(x) ((stbi_uc)((x) >> 2))

    static stbi_uc* resample_row_1(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w, int hs)
    {
        STBI_NOTUSED(out);
        STBI_NOTUSED(in_far);
        STBI_NOTUSED(w);
        STBI_NOTUSED(hs);
        return in_near;
    }

    static stbi_uc* stbi_resample_row_v_2(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w,
                                          int hs)
    {
        // need to generate two samples vertically for every one in input
        STBI_NOTUSED(hs);
        for (int i = 0; i < w; ++i)
            out[i] = stbi_div4(3 * in_near[i] + in_far[i] + 2);
        return out;
    }

    static stbi_uc* stbi_resample_row_h_2(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w,
                                          int hs)
    {
        // need to generate two samples horizontally for every one in input
        int i;
        const stbi_uc* input = in_near;

        if (w == 1) {
            // if only one sample, can't do any interpolation
            out[0] = out[1] = input[0];
            return out;
        }

        out[0] = input[0];
        out[1] = stbi_div4(input[0] * 3 + input[1] + 2);
        for (i = 1; i < w - 1; ++i) {
            const int n = 3 * input[i] + 2;
            out[i * 2 + 0] = stbi_div4(n + input[i - 1]);
            out[i * 2 + 1] = stbi_div4(n + input[i + 1]);
        }
        out[i * 2 + 0] = stbi_div4(input[w - 2] * 3 + input[w - 1] + 2);
        out[i * 2 + 1] = input[w - 1];

        STBI_NOTUSED(in_far);
        STBI_NOTUSED(hs);

        return out;
    }

    static stbi_uc* stbi_resample_row_hv_2(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w,
                                           int hs)
    {
        // need to generate 2x2 samples for every one in input
        if (w == 1) {
            out[0] = out[1] = stbi_div4(3 * in_near[0] + in_far[0] + 2);
            return out;
        }

        int t1 = 3 * in_near[0] + in_far[0];
        out[0] = stbi_div4(t1 + 2);
        for (int i = 1; i < w; ++i) {
            const int t0 = t1;
            t1 = 3 * in_near[i] + in_far[i];
            out[i * 2 - 1] = stbi_div16(3 * t0 + t1 + 8);
            out[i * 2] = stbi_div16(3 * t1 + t0 + 8);
        }
        out[w * 2 - 1] = stbi_div4(t1 + 2);

        STBI_NOTUSED(hs);

        return out;
    }

  #if defined(STBI_SSE2) || defined(STBI_NEON)
    static stbi_uc* stbi_resample_row_hv_2_simd(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far,
                                                int w, int hs)
    {
        // need to generate 2x2 samples for every one in input
        int i = 0, t0, t1;

        if (w == 1) {
            out[0] = out[1] = stbi_div4(3 * in_near[0] + in_far[0] + 2);
            return out;
        }

        t1 = 3 * in_near[0] + in_far[0];
        // process groups of 8 pixels for as long as we can.
        // note we can't handle the last pixel in a row in this loop
        // because we need to handle the filter boundary conditions.
        for (; i < ((w - 1) & ~7); i += 8) {
    #if defined(STBI_SSE2)
            // load and perform the vertical filtering pass
            // this uses 3*x + y = 4*x + (y - x)
            __m128i zero = _mm_setzero_si128();
            __m128i farb = _mm_loadl_epi64(reinterpret_cast<__m128i*>(in_far + i));
            __m128i nearb = _mm_loadl_epi64(reinterpret_cast<__m128i*>(in_near + i));
            __m128i farw = _mm_unpacklo_epi8(farb, zero);
            __m128i nearw = _mm_unpacklo_epi8(nearb, zero);
            __m128i diff = _mm_sub_epi16(farw, nearw);
            __m128i nears = _mm_slli_epi16(nearw, 2);
            __m128i curr = _mm_add_epi16(nears, diff);  // current row

            // horizontal filter works the same based on shifted vers of current
            // row. "prev" is current row shifted right by 1 pixel; we need to
            // insert the previous pixel value (from t1).
            // "next" is current row shifted left by 1 pixel, with first pixel
            // of next block of 8 pixels added in.
            __m128i prv0 = _mm_slli_si128(curr, 2);
            __m128i nxt0 = _mm_srli_si128(curr, 2);
            __m128i prev = _mm_insert_epi16(prv0, t1, 0);
            __m128i next = _mm_insert_epi16(nxt0, 3 * in_near[i + 8] + in_far[i + 8], 7);

            // horizontal filter, polyphase implementation since it's convenient:
            // even pixels = 3*cur + prev = cur*4 + (prev - cur)
            // odd  pixels = 3*cur + next = cur*4 + (next - cur)
            // note the shared term.
            __m128i bias = _mm_set1_epi16(8);
            __m128i curs = _mm_slli_epi16(curr, 2);
            __m128i prvd = _mm_sub_epi16(prev, curr);
            __m128i nxtd = _mm_sub_epi16(next, curr);
            __m128i curb = _mm_add_epi16(curs, bias);
            __m128i even = _mm_add_epi16(prvd, curb);
            __m128i odd = _mm_add_epi16(nxtd, curb);

            // interleave even and odd pixels, then undo scaling.
            __m128i int0 = _mm_unpacklo_epi16(even, odd);
            __m128i int1 = _mm_unpackhi_epi16(even, odd);
            __m128i de0 = _mm_srli_epi16(int0, 4);
            __m128i de1 = _mm_srli_epi16(int1, 4);

            // pack and write output
            __m128i outv = _mm_packus_epi16(de0, de1);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(out + i * 2), outv);
    #elif defined(STBI_NEON)
            // load and perform the vertical filtering pass
            // this uses 3*x + y = 4*x + (y - x)
            uint8x8_t farb = vld1_u8(in_far + i);
            uint8x8_t nearb = vld1_u8(in_near + i);
            int16x8_t diff = vreinterpretq_s16_u16(vsubl_u8(farb, nearb));
            int16x8_t nears = vreinterpretq_s16_u16(vshll_n_u8(nearb, 2));
            int16x8_t curr = vaddq_s16(nears, diff);  // current row

            // horizontal filter works the same based on shifted vers of current
            // row. "prev" is current row shifted right by 1 pixel; we need to
            // insert the previous pixel value (from t1).
            // "next" is current row shifted left by 1 pixel, with first pixel
            // of next block of 8 pixels added in.
            int16x8_t prv0 = vextq_s16(curr, curr, 7);
            int16x8_t nxt0 = vextq_s16(curr, curr, 1);
            int16x8_t prev = vsetq_lane_s16(t1, prv0, 0);
            int16x8_t next = vsetq_lane_s16(3 * in_near[i + 8] + in_far[i + 8], nxt0, 7);

            // horizontal filter, polyphase implementation since it's convenient:
            // even pixels = 3*cur + prev = cur*4 + (prev - cur)
            // odd  pixels = 3*cur + next = cur*4 + (next - cur)
            // note the shared term.
            int16x8_t curs = vshlq_n_s16(curr, 2);
            int16x8_t prvd = vsubq_s16(prev, curr);
            int16x8_t nxtd = vsubq_s16(next, curr);
            int16x8_t even = vaddq_s16(curs, prvd);
            int16x8_t odd = vaddq_s16(curs, nxtd);

            // undo scaling and round, then store with even/odd phases interleaved
            uint8x8x2_t o;
            o.val[0] = vqrshrun_n_s16(even, 4);
            o.val[1] = vqrshrun_n_s16(odd, 4);
            vst2_u8(out + i * 2, o);
    #endif

            // "previous" value for next iter
            t1 = 3 * in_near[i + 7] + in_far[i + 7];
        }

        t0 = t1;
        t1 = 3 * in_near[i] + in_far[i];
        out[i * 2] = stbi_div16(3 * t1 + t0 + 8);

        for (++i; i < w; ++i) {
            t0 = t1;
            t1 = 3 * in_near[i] + in_far[i];
            out[i * 2 - 1] = stbi_div16(3 * t0 + t1 + 8);
            out[i * 2] = stbi_div16(3 * t1 + t0 + 8);
        }
        out[w * 2 - 1] = stbi_div4(t1 + 2);

        STBI_NOTUSED(hs);

        return out;
    }
  #endif

    static stbi_uc* stbi_resample_row_generic(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far,
                                              int w, int hs)
    {
        // resample with nearest-neighbor
        STBI_NOTUSED(in_far);
        for (int i = 0; i < w; ++i)
            for (int j = 0; j < hs; ++j)
                out[i * hs + j] = in_near[i];
        return out;
    }

  // this is a reduced-precision calculation of YCbCr-to-RGB introduced
  // to make sure the code produces the same results in both SIMD and scalar
  #define stbi_float2fixed(x) (((int)((x) * 4096.0f + 0.5f)) << 8)

    static void stbi_YCbCr_to_RGB_row(stbi_uc* out, stbi_uc* y, stbi_uc* pcb, stbi_uc* pcr,
                                      int count, int step)
    {
        for (int i = 0; i < count; ++i) {
            const int y_fixed = (y[i] << 20) + (1 << 19);  // rounding
            const int cr = pcr[i] - 128;
            const int cb = pcb[i] - 128;
            int r = y_fixed + cr * stbi_float2fixed(1.40200f);
            int g = y_fixed + (cr * -stbi_float2fixed(0.71414f)) +
                    ((cb * -stbi_float2fixed(0.34414f)) & 0xffff0000);
            int b = y_fixed + cb * stbi_float2fixed(1.77200f);
            r >>= 20;
            g >>= 20;
            b >>= 20;
            if (static_cast<unsigned>(r) > 255) {
                if (r < 0)
                    r = 0;
                else
                    r = 255;
            }
            if (static_cast<unsigned>(g) > 255) {
                if (g < 0)
                    g = 0;
                else
                    g = 255;
            }
            if (static_cast<unsigned>(b) > 255) {
                if (b < 0)
                    b = 0;
                else
                    b = 255;
            }
            out[0] = static_cast<stbi_uc>(r);
            out[1] = static_cast<stbi_uc>(g);
            out[2] = static_cast<stbi_uc>(b);
            out[3] = 255;
            out += step;
        }
    }

  #if defined(STBI_SSE2) || defined(STBI_NEON)
    static void stbi_YCbCr_to_RGB_simd(stbi_uc* out, stbi_uc* y, stbi_uc* pcb, stbi_uc* pcr,
                                       int count, int step)
    {
        int i = 0;

    #ifdef STBI_SSE2
        // step == 3 is pretty ugly on the final interleave, and i'm not convinced
        // it's useful in practice (you wouldn't use it for textures, for example).
        // so just accelerate step == 4 case.
        if (step == 4) {
            // this is a fairly straightforward implementation and not super-optimized.
            __m128i signflip = _mm_set1_epi8(-0x80);
            __m128i cr_const0 = _mm_set1_epi16(static_cast<short>(std::lround(1.40200f * 4096.0f)));
            __m128i cr_const1 = _mm_set1_epi16(-static_cast<short>(std::lround(0.71414f * 4096.0f)));
            __m128i cb_const0 = _mm_set1_epi16(-static_cast<short>(std::lround(0.34414f * 4096.0f)));
            __m128i cb_const1 = _mm_set1_epi16(static_cast<short>(std::lround(1.77200f * 4096.0f)));
            __m128i y_bias = _mm_set1_epi8(static_cast<char>(static_cast<unsigned char>(128)));
            __m128i xw = _mm_set1_epi16(255);  // alpha channel

            for (; i + 7 < count; i += 8) {
                // load
                __m128i y_bytes = _mm_loadl_epi64(reinterpret_cast<__m128i*>(y + i));
                __m128i cr_bytes = _mm_loadl_epi64(reinterpret_cast<__m128i*>(pcr + i));
                __m128i cb_bytes = _mm_loadl_epi64(reinterpret_cast<__m128i*>(pcb + i));
                __m128i cr_biased = _mm_xor_si128(cr_bytes, signflip);  // -128
                __m128i cb_biased = _mm_xor_si128(cb_bytes, signflip);  // -128

                // unpack to short (and left-shift cr, cb by 8)
                __m128i yw = _mm_unpacklo_epi8(y_bias, y_bytes);
                __m128i crw = _mm_unpacklo_epi8(_mm_setzero_si128(), cr_biased);
                __m128i cbw = _mm_unpacklo_epi8(_mm_setzero_si128(), cb_biased);

                // color transform
                __m128i yws = _mm_srli_epi16(yw, 4);
                __m128i cr0 = _mm_mulhi_epi16(cr_const0, crw);
                __m128i cb0 = _mm_mulhi_epi16(cb_const0, cbw);
                __m128i cb1 = _mm_mulhi_epi16(cbw, cb_const1);
                __m128i cr1 = _mm_mulhi_epi16(crw, cr_const1);
                __m128i rws = _mm_add_epi16(cr0, yws);
                __m128i gwt = _mm_add_epi16(cb0, yws);
                __m128i bws = _mm_add_epi16(yws, cb1);
                __m128i gws = _mm_add_epi16(gwt, cr1);

                // descale
                __m128i rw = _mm_srai_epi16(rws, 4);
                __m128i bw = _mm_srai_epi16(bws, 4);
                __m128i gw = _mm_srai_epi16(gws, 4);

                // back to byte, set up for transpose
                __m128i brb = _mm_packus_epi16(rw, bw);
                __m128i gxb = _mm_packus_epi16(gw, xw);

                // transpose to interleave channels
                __m128i t0 = _mm_unpacklo_epi8(brb, gxb);
                __m128i t1 = _mm_unpackhi_epi8(brb, gxb);
                __m128i o0 = _mm_unpacklo_epi16(t0, t1);
                __m128i o1 = _mm_unpackhi_epi16(t0, t1);

                // store
                _mm_storeu_si128(reinterpret_cast<__m128i*>(out + 0), o0);
                _mm_storeu_si128(reinterpret_cast<__m128i*>(out + 16), o1);
                out += 32;
            }
        }
    #endif

    #ifdef STBI_NEON
        // in this version, step=3 support would be easy to add. but is there demand?
        if (step == 4) {
            // this is a fairly straightforward implementation and not super-optimized.
            uint8x8_t signflip = vdup_n_u8(0x80);
            int16x8_t cr_const0 = vdupq_n_s16((short)(1.40200f * 4096.0f + 0.5f));
            int16x8_t cr_const1 = vdupq_n_s16(-(short)(0.71414f * 4096.0f + 0.5f));
            int16x8_t cb_const0 = vdupq_n_s16(-(short)(0.34414f * 4096.0f + 0.5f));
            int16x8_t cb_const1 = vdupq_n_s16((short)(1.77200f * 4096.0f + 0.5f));

            for (; i + 7 < count; i += 8) {
                // load
                uint8x8_t y_bytes = vld1_u8(y + i);
                uint8x8_t cr_bytes = vld1_u8(pcr + i);
                uint8x8_t cb_bytes = vld1_u8(pcb + i);
                int8x8_t cr_biased = vreinterpret_s8_u8(vsub_u8(cr_bytes, signflip));
                int8x8_t cb_biased = vreinterpret_s8_u8(vsub_u8(cb_bytes, signflip));

                // expand to s16
                int16x8_t yws = vreinterpretq_s16_u16(vshll_n_u8(y_bytes, 4));
                int16x8_t crw = vshll_n_s8(cr_biased, 7);
                int16x8_t cbw = vshll_n_s8(cb_biased, 7);

                // color transform
                int16x8_t cr0 = vqdmulhq_s16(crw, cr_const0);
                int16x8_t cb0 = vqdmulhq_s16(cbw, cb_const0);
                int16x8_t cr1 = vqdmulhq_s16(crw, cr_const1);
                int16x8_t cb1 = vqdmulhq_s16(cbw, cb_const1);
                int16x8_t rws = vaddq_s16(yws, cr0);
                int16x8_t gws = vaddq_s16(vaddq_s16(yws, cb0), cr1);
                int16x8_t bws = vaddq_s16(yws, cb1);

                // undo scaling, round, convert to byte
                uint8x8x4_t o;
                o.val[0] = vqrshrun_n_s16(rws, 4);
                o.val[1] = vqrshrun_n_s16(gws, 4);
                o.val[2] = vqrshrun_n_s16(bws, 4);
                o.val[3] = vdup_n_u8(255);

                // store, interleaving r/g/b/a
                vst4_u8(out, o);
                out += 8 * 4;
            }
        }
    #endif

        for (; i < count; ++i) {
            int y_fixed = (y[i] << 20) + (1 << 19);  // rounding
            int r, g, b;
            int cr = pcr[i] - 128;
            int cb = pcb[i] - 128;
            r = y_fixed + cr * stbi_float2fixed(1.40200f);
            g = y_fixed + cr * -stbi_float2fixed(0.71414f) +
                ((cb * -stbi_float2fixed(0.34414f)) & 0xffff0000);
            b = y_fixed + cb * stbi_float2fixed(1.77200f);
            r >>= 20;
            g >>= 20;
            b >>= 20;
            if (static_cast<unsigned>(r) > 255) {
                if (r < 0)
                    r = 0;
                else
                    r = 255;
            }
            if (static_cast<unsigned>(g) > 255) {
                if (g < 0)
                    g = 0;
                else
                    g = 255;
            }
            if (static_cast<unsigned>(b) > 255) {
                if (b < 0)
                    b = 0;
                else
                    b = 255;
            }
            out[0] = static_cast<stbi_uc>(r);
            out[1] = static_cast<stbi_uc>(g);
            out[2] = static_cast<stbi_uc>(b);
            out[3] = 255;
            out += step;
        }
    }
  #endif

    // set up the kernels
    static void stbi_setup_jpeg(stbi_jpeg* j)
    {
        j->idct_block_kernel = stbi_idct_block;
        j->YCbCr_to_RGB_kernel = stbi_YCbCr_to_RGB_row;
        j->resample_row_hv_2_kernel = stbi_resample_row_hv_2;

  #ifdef STBI_SSE2
        if (stbi_sse2_available()) {
            j->idct_block_kernel = stbi_idct_simd;
            j->YCbCr_to_RGB_kernel = stbi_YCbCr_to_RGB_simd;
            j->resample_row_hv_2_kernel = stbi_resample_row_hv_2_simd;
        }
  #endif

  #ifdef STBI_NEON
        j->idct_block_kernel = stbi__idct_simd;
        j->YCbCr_to_RGB_kernel = stbi__YCbCr_to_RGB_simd;
        j->resample_row_hv_2_kernel = stbi__resample_row_hv_2_simd;
  #endif
    }

    // clean up the temporary component buffers
    static void stbi_cleanup_jpeg(stbi_jpeg* j)
    {
        stbi_free_jpeg_components(j, j->s->img_n, 0);
    }

    typedef struct
    {
        stbi_uc* (*resample)(stbi_uc* out, stbi_uc* in_near, stbi_uc* in_far, int w, int hs);
        stbi_uc *line0, *line1;
        int hs, vs;   // expansion factor in each axis
        int w_lores;  // horizontal pixels pre-expansion
        int ystep;    // how far through vertical expansion we are
        int ypos;     // which pre-expansion row we're on
    } stbi_resample;

    // fast 0..255 * 0..255 => 0..255 rounded multiplication
    static stbi_uc stbi_blinn_8x8(const stbi_uc x, const stbi_uc y)
    {
        const unsigned int t = x * y + 128;
        return static_cast<stbi_uc>((t + (t >> 8)) >> 8);
    }

    static stbi_uc* load_jpeg_image(stbi_jpeg* z, int* out_x, int* out_y, int* comp,
                                    const int req_comp)
    {
        int decode_n;
        z->s->img_n = 0;  // make stbi__cleanup_jpeg safe

        // validate req_comp
        if (req_comp < 0 || req_comp > 4)
            return stbi_errpuc("bad req_comp", "Internal error");

        // load a jpeg image from whichever source, but leave in YCbCr format
        if (!stbi_decode_jpeg_image(z)) {
            stbi_cleanup_jpeg(z);
            return nullptr;
        }

        // determine actual number of components to generate
        const int n = req_comp ? req_comp : z->s->img_n >= 3 ? 3
                                                             : 1;

        const int is_rgb = z->s->img_n == 3 &&
                           (z->rgb == 3 || (z->app14_color_transform == 0 && !z->jfif));

        if (z->s->img_n == 3 && n < 3 && !is_rgb)
            decode_n = 1;
        else
            decode_n = z->s->img_n;

        // nothing to do if no components requested; check this now to avoid
        // accessing uninitialized coutput[0] later
        if (decode_n <= 0) {
            stbi_cleanup_jpeg(z);
            return nullptr;
        }

        // resample and color-convert
        {
            int k;
            unsigned int i;
            stbi_uc* coutput[4] = { nullptr, nullptr, nullptr, nullptr };

            stbi_resample res_comp[4];

            for (k = 0; k < decode_n; ++k) {
                stbi_resample* r = &res_comp[k];

                // allocate line buffer big enough for upsampling off the edges
                // with upsample factor of 4
                z->img_comp[k].linebuf = static_cast<stbi_uc*>(stbi_malloc(z->s->img_x + 3));
                if (!z->img_comp[k].linebuf) {
                    stbi_cleanup_jpeg(z);
                    return stbi_errpuc("outofmem", "Out of memory");
                }

                r->hs = z->img_h_max / z->img_comp[k].h;
                r->vs = z->img_v_max / z->img_comp[k].v;
                r->ystep = r->vs >> 1;
                r->w_lores = (static_cast<int>(z->s->img_x) + r->hs - 1) / r->hs;
                r->ypos = 0;
                r->line0 = r->line1 = z->img_comp[k].data;

                if (r->hs == 1 && r->vs == 1)
                    r->resample = resample_row_1;
                else if (r->hs == 1 && r->vs == 2)
                    r->resample = stbi_resample_row_v_2;
                else if (r->hs == 2 && r->vs == 1)
                    r->resample = stbi_resample_row_h_2;
                else if (r->hs == 2 && r->vs == 2)
                    r->resample = z->resample_row_hv_2_kernel;
                else
                    r->resample = stbi_resample_row_generic;
            }

            // can't error after this so, this is safe
            stbi_uc* output = static_cast<stbi_uc*>(stbi_malloc_mad3(
                n, static_cast<int>(z->s->img_x), static_cast<int>(z->s->img_y), 1));
            if (!output) {
                stbi_cleanup_jpeg(z);
                return stbi_errpuc("outofmem", "Out of memory");
            }

            // now go ahead and resample
            for (unsigned int j = 0; j < z->s->img_y; ++j) {
                stbi_uc* out = output + n * z->s->img_x * j;
                for (k = 0; k < decode_n; ++k) {
                    stbi_resample* r = &res_comp[k];
                    const int y_bot = r->ystep >= (r->vs >> 1);
                    coutput[k] = r->resample(z->img_comp[k].linebuf, y_bot ? r->line1 : r->line0,
                                             y_bot ? r->line0 : r->line1, r->w_lores, r->hs);
                    if (++r->ystep >= r->vs) {
                        r->ystep = 0;
                        r->line0 = r->line1;
                        if (++r->ypos < z->img_comp[k].y)
                            r->line1 += z->img_comp[k].w2;
                    }
                }
                if (n >= 3) {
                    stbi_uc* y = coutput[0];
                    if (z->s->img_n == 3) {
                        if (is_rgb) {
                            for (i = 0; i < z->s->img_x; ++i) {
                                out[0] = y[i];
                                out[1] = coutput[1][i];
                                out[2] = coutput[2][i];
                                out[3] = 255;
                                out += n;
                            }
                        }
                        else {
                            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
                        }
                    }
                    else if (z->s->img_n == 4) {
                        if (z->app14_color_transform == 0) {  // CMYK
                            for (i = 0; i < z->s->img_x; ++i) {
                                const stbi_uc m = coutput[3][i];
                                out[0] = stbi_blinn_8x8(coutput[0][i], m);
                                out[1] = stbi_blinn_8x8(coutput[1][i], m);
                                out[2] = stbi_blinn_8x8(coutput[2][i], m);
                                out[3] = 255;
                                out += n;
                            }
                        }
                        else if (z->app14_color_transform == 2) {  // YCCK
                            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
                            for (i = 0; i < z->s->img_x; ++i) {
                                const stbi_uc m = coutput[3][i];
                                out[0] = stbi_blinn_8x8(255 - out[0], m);
                                out[1] = stbi_blinn_8x8(255 - out[1], m);
                                out[2] = stbi_blinn_8x8(255 - out[2], m);
                                out += n;
                            }
                        }
                        else {  // YCbCr + alpha?  Ignore the fourth channel for now
                            z->YCbCr_to_RGB_kernel(out, y, coutput[1], coutput[2], z->s->img_x, n);
                        }
                    }
                    else
                        for (i = 0; i < z->s->img_x; ++i) {
                            out[0] = out[1] = out[2] = y[i];
                            out[3] = 255;  // not used if n==3
                            out += n;
                        }
                }
                else if (is_rgb) {
                    if (n == 1)
                        for (i = 0; i < z->s->img_x; ++i)
                            *out++ = stbi_compute_y(coutput[0][i], coutput[1][i], coutput[2][i]);
                    else {
                        for (i = 0; i < z->s->img_x; ++i, out += 2) {
                            out[0] = stbi_compute_y(coutput[0][i], coutput[1][i], coutput[2][i]);
                            out[1] = 255;
                        }
                    }
                }
                else if (z->s->img_n == 4 && z->app14_color_transform == 0) {
                    for (i = 0; i < z->s->img_x; ++i) {
                        const stbi_uc m = coutput[3][i];
                        const stbi_uc r = stbi_blinn_8x8(coutput[0][i], m);
                        const stbi_uc g = stbi_blinn_8x8(coutput[1][i], m);
                        const stbi_uc b = stbi_blinn_8x8(coutput[2][i], m);
                        out[0] = stbi_compute_y(r, g, b);
                        out[1] = 255;
                        out += n;
                    }
                }
                else if (z->s->img_n == 4 && z->app14_color_transform == 2) {
                    for (i = 0; i < z->s->img_x; ++i) {
                        out[0] = stbi_blinn_8x8(255 - coutput[0][i], coutput[3][i]);
                        out[1] = 255;
                        out += n;
                    }
                }
                else {
                    const stbi_uc* y = coutput[0];
                    if (n == 1)
                        for (i = 0; i < z->s->img_x; ++i)
                            out[i] = y[i];
                    else
                        for (i = 0; i < z->s->img_x; ++i) {
                            *out++ = y[i];
                            *out++ = 255;
                        }
                }
            }
            stbi_cleanup_jpeg(z);
            *out_x = z->s->img_x;
            *out_y = z->s->img_y;
            if (comp)
                *comp = z->s->img_n >= 3 ? 3 : 1;  // report original components, not output
            return output;
        }
    }

    static void* stbi_jpeg_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                                const StbiResultInfo* ri)
    {
        stbi_jpeg* j = static_cast<stbi_jpeg*>(stbi_malloc(sizeof(stbi_jpeg)));
        if (!j)
            return stbi_errpuc("outofmem", "Out of memory");
        memset(j, 0, sizeof(stbi_jpeg));
        STBI_NOTUSED(ri);
        j->s = s;
        stbi_setup_jpeg(j);
        unsigned char* result = load_jpeg_image(j, x, y, comp, req_comp);
        STBI_FREE(j);
        return result;
    }

    static int stbi_jpeg_test(StbiContext* s)
    {
        stbi_jpeg* j = static_cast<stbi_jpeg*>(stbi_malloc(sizeof(stbi_jpeg)));
        if (!j)
            return stbi__err("outofmem", "Out of memory");
        memset(j, 0, sizeof(stbi_jpeg));
        j->s = s;
        stbi_setup_jpeg(j);
        const int r = stbi_decode_jpeg_header(j, STBI_SCAN_type);
        stbi_rewind(s);
        STBI_FREE(j);
        return r;
    }

    static int stbi_jpeg_info_raw(stbi_jpeg* j, int* x, int* y, int* comp)
    {
        if (!stbi_decode_jpeg_header(j, STBI_SCAN_header)) {
            stbi_rewind(j->s);
            return 0;
        }
        if (x)
            *x = j->s->img_x;
        if (y)
            *y = j->s->img_y;
        if (comp)
            *comp = j->s->img_n >= 3 ? 3 : 1;
        return 1;
    }

    static int stbi_jpeg_info(StbiContext* s, int* x, int* y, int* comp)
    {
        stbi_jpeg* j = static_cast<stbi_jpeg*>(stbi_malloc(sizeof(stbi_jpeg)));
        if (!j)
            return stbi__err("outofmem", "Out of memory");
        memset(j, 0, sizeof(stbi_jpeg));
        j->s = s;
        const int result = stbi_jpeg_info_raw(j, x, y, comp);
        STBI_FREE(j);
        return result;
    }
#endif

    // public domain zlib decode    v0.2  Sean Barrett 2006-11-18
    //    simple implementation
    //      - all input must be provided in an upfront buffer
    //      - all output is written to a single output buffer (can malloc/realloc)
    //    performance
    //      - fast huffman

#ifndef STBI_NO_ZLIB

  // fast-way is faster to check than jpeg huffman, but slow way is slower
  #define STBI__ZFAST_BITS 9  // accelerate all cases in default tables
  #define STBI_ZFAST_MASK  ((1 << STBI__ZFAST_BITS) - 1)
  #define STBI_ZNSYMS      288  // number of symbols in literal/length alphabet

    // zlib-style huffman encoding
    // (jpegs packs from left, zlib from right, so can't share code)
    typedef struct
    {
        StbiUint16 fast[1 << STBI__ZFAST_BITS];
        StbiUint16 firstcode[16];
        int maxcode[17];
        StbiUint16 firstsymbol[16];
        stbi_uc size[STBI_ZNSYMS];
        StbiUint16 value[STBI_ZNSYMS];
    } stbi_zhuffman;

    static stbi_inline int stbi_bitreverse16(int n)
    {
        n = ((n & 0xAAAA) >> 1) | ((n & 0x5555) << 1);
        n = ((n & 0xCCCC) >> 2) | ((n & 0x3333) << 2);
        n = ((n & 0xF0F0) >> 4) | ((n & 0x0F0F) << 4);
        n = ((n & 0xFF00) >> 8) | ((n & 0x00FF) << 8);
        return n;
    }

    static stbi_inline int stbi_bit_reverse(const int v, const int bits)
    {
        STBI_ASSERT(bits <= 16);
        // to bit reverse n bits, reverse 16 and shift
        // e.g. 11 bits, bit reverse and shift away 5
        return stbi_bitreverse16(v) >> (16 - bits);
    }

    static int stbi_zbuild_huffman(stbi_zhuffman* z, const stbi_uc* sizelist, const int num)
    {
        int i, k = 0;
        int next_code[16], sizes[17];

        // DEFLATE spec for generating codes
        memset(sizes, 0, sizeof(sizes));
        memset(z->fast, 0, sizeof(z->fast));
        for (i = 0; i < num; ++i)
            ++sizes[sizelist[i]];
        sizes[0] = 0;
        for (i = 1; i < 16; ++i)
            if (sizes[i] > (1 << i))
                return stbi__err("bad sizes", "Corrupt PNG");
        int code = 0;
        for (i = 1; i < 16; ++i) {
            next_code[i] = code;
            z->firstcode[i] = static_cast<StbiUint16>(code);
            z->firstsymbol[i] = static_cast<StbiUint16>(k);
            code = (code + sizes[i]);
            if (sizes[i])
                if (code - 1 >= (1 << i))
                    return stbi__err("bad codelengths", "Corrupt PNG");
            z->maxcode[i] = code << (16 - i);  // preshift for inner loop
            code <<= 1;
            k += sizes[i];
        }
        z->maxcode[16] = 0x10000;  // sentinel
        for (i = 0; i < num; ++i) {
            const int s = sizelist[i];
            if (s) {
                const int c = next_code[s] - z->firstcode[s] + z->firstsymbol[s];
                const StbiUint16 fastv = static_cast<StbiUint16>((s << 9) | i);
                z->size[c] = static_cast<stbi_uc>(s);
                z->value[c] = static_cast<StbiUint16>(i);
                if (s <= STBI__ZFAST_BITS) {
                    int j = stbi_bit_reverse(next_code[s], s);
                    while (j < (1 << STBI__ZFAST_BITS)) {
                        z->fast[j] = fastv;
                        j += (1 << s);
                    }
                }
                ++next_code[s];
            }
        }
        return 1;
    }

    // zlib-from-memory implementation for PNG reading
    //    because PNG allows splitting the zlib stream arbitrarily,
    //    and it's annoying structurally to have PNG call ZLIB call PNG,
    //    we require PNG read all the IDATs and combine them into a single
    //    memory buffer

    typedef struct
    {
        stbi_uc *zbuffer, *zbuffer_end;
        int num_bits;
        int hit_zeof_once;
        StbiUint32 code_buffer;

        char* zout;
        char* zout_start;
        char* zout_end;
        int z_expandable;

        stbi_zhuffman z_length, z_distance;
    } stbi_zbuf;

    static stbi_inline int stbi_zeof(const stbi_zbuf* z)
    {
        return z->zbuffer >= z->zbuffer_end;
    }

    static stbi_inline stbi_uc stbi_zget8(stbi_zbuf* z)
    {
        return stbi_zeof(z) ? 0 : *z->zbuffer++;
    }

    static void stbi_fill_bits(stbi_zbuf* z)
    {
        do {
            if (z->code_buffer >= (1U << z->num_bits)) {
                z->zbuffer = z->zbuffer_end; /* treat this as EOF so we fail. */
                return;
            }
            z->code_buffer |= static_cast<unsigned int>(stbi_zget8(z)) << z->num_bits;
            z->num_bits += 8;
        }
        while (z->num_bits <= 24);
    }

    static stbi_inline unsigned int stbi_zreceive(stbi_zbuf* z, const int n)
    {
        if (z->num_bits < n)
            stbi_fill_bits(z);
        const unsigned int k = z->code_buffer & ((1 << n) - 1);
        z->code_buffer >>= n;
        z->num_bits -= n;
        return k;
    }

    static int stbi_zhuffman_decode_slowpath(stbi_zbuf* a, const stbi_zhuffman* z)
    {
        int s;
        // not resolved by fast table, so compute it the slow way
        // use jpeg approach, which requires MSbits at top
        const int k = stbi_bit_reverse(a->code_buffer, 16);
        for (s = STBI__ZFAST_BITS + 1;; ++s)
            if (k < z->maxcode[s])
                break;
        if (s >= 16)
            return -1;  // invalid code!
        // code size is s, so:
        const int b = (k >> (16 - s)) - z->firstcode[s] + z->firstsymbol[s];
        if (b >= STBI_ZNSYMS)
            return -1;  // some data was corrupt somewhere!
        if (z->size[b] != s)
            return -1;  // was originally an assert, but report failure instead.
        a->code_buffer >>= s;
        a->num_bits -= s;
        return z->value[b];
    }

    static stbi_inline int stbi_zhuffman_decode(stbi_zbuf* a, stbi_zhuffman* z)
    {
        if (a->num_bits < 16) {
            if (stbi_zeof(a)) {
                if (!a->hit_zeof_once) {
                    // This is the first time we hit eof, insert 16 extra padding btis
                    // to allow us to keep going; if we actually consume any of them
                    // though, that is invalid data. This is caught later.
                    a->hit_zeof_once = 1;
                    a->num_bits += 16;  // add 16 implicit zero bits
                }
                else {
                    // We already inserted our extra 16 padding bits and are again
                    // out, this stream is actually prematurely terminated.
                    return -1;
                }
            }
            else {
                stbi_fill_bits(a);
            }
        }
        const int b = z->fast[a->code_buffer & STBI_ZFAST_MASK];
        if (b) {
            const int s = b >> 9;
            a->code_buffer >>= s;
            a->num_bits -= s;
            return b & 511;
        }
        return stbi_zhuffman_decode_slowpath(a, z);
    }

    static int stbi_zexpand(stbi_zbuf* z, char* zout, const int n)  // need to make room for n
                                                                    // bytes
    {
        unsigned int old_limit;
        z->zout = zout;
        if (!z->z_expandable)
            return stbi__err("output buffer limit", "Corrupt PNG");
        const unsigned int cur = static_cast<unsigned int>(z->zout - z->zout_start);
        unsigned int limit = old_limit = static_cast<unsigned>(z->zout_end - z->zout_start);
        if (UINT_MAX - cur < static_cast<unsigned>(n))
            return stbi__err("outofmem", "Out of memory");
        while (cur + n > limit) {
            if (limit > UINT_MAX / 2)
                return stbi__err("outofmem", "Out of memory");
            limit *= 2;
        }
        char* q = static_cast<char*>(STBI_REALLOC_SIZED(z->zout_start, old_limit, limit));
        STBI_NOTUSED(old_limit);
        if (q == nullptr)
            return stbi__err("outofmem", "Out of memory");
        z->zout_start = q;
        z->zout = q + cur;
        z->zout_end = q + limit;
        return 1;
    }

    constexpr static int stbi_zlength_base[31] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15,
                                                   17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83,
                                                   99, 115, 131, 163, 195, 227, 258, 0, 0 };

    constexpr static int stbi_zlength_extra[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                                    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };

    constexpr static int stbi_zdist_base[32] = { 1, 2, 3, 4, 5, 7, 9, 13,
                                                 17, 25, 33, 49, 65, 97, 129, 193,
                                                 257, 385, 513, 769, 1025, 1537, 2049, 3073,
                                                 4097, 6145, 8193, 12289, 16385, 24577, 0, 0 };

    constexpr static int stbi_zdist_extra[32] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3,
                                                  4, 4, 5, 5, 6, 6, 7, 7, 8, 8,
                                                  9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };

    static int stbi_parse_huffman_block(stbi_zbuf* a)
    {
        char* zout = a->zout;
        for (;;) {
            int z = stbi_zhuffman_decode(a, &a->z_length);
            if (z < 256) {
                if (z < 0)
                    return stbi__err("bad huffman code", "Corrupt PNG");  // error in huffman codes
                if (zout >= a->zout_end) {
                    if (!stbi_zexpand(a, zout, 1))
                        return 0;
                    zout = a->zout;
                }
                *zout++ = static_cast<char>(z);
            }
            else {
                if (z == 256) {
                    a->zout = zout;
                    if (a->hit_zeof_once && a->num_bits < 16) {
                        // The first time we hit zeof, we inserted 16 extra zero bits into our bit
                        // buffer so the decoder can just do its speculative decoding. But if we
                        // actually consumed any of those bits (which is the case when num_bits <
                        // 16), the stream actually read past the end so it is malformed.
                        return stbi__err("unexpected end", "Corrupt PNG");
                    }
                    return 1;
                }
                if (z >= 286)
                    return stbi__err("bad huffman code", "Corrupt PNG");  // per DEFLATE, length
                                                                          // codes 286 and 287 must
                                                                          // not appear in
                                                                          // compressed data
                z -= 257;
                int len = stbi_zlength_base[z];
                if (stbi_zlength_extra[z])
                    len += stbi_zreceive(a, stbi_zlength_extra[z]);
                z = stbi_zhuffman_decode(a, &a->z_distance);
                if (z < 0 || z >= 30)
                    return stbi__err("bad huffman code", "Corrupt PNG");  // per DEFLATE, distance
                                                                          // codes 30 and 31 must
                                                                          // not appear in
                                                                          // compressed data
                int dist = stbi_zdist_base[z];
                if (stbi_zdist_extra[z])
                    dist += stbi_zreceive(a, stbi_zdist_extra[z]);
                if (zout - a->zout_start < dist)
                    return stbi__err("bad dist", "Corrupt PNG");
                if (len > a->zout_end - zout) {
                    if (!stbi_zexpand(a, zout, len))
                        return 0;
                    zout = a->zout;
                }
                const stbi_uc* p = (stbi_uc*)(zout - dist);
                if (dist == 1) {  // run of one byte; common in images.
                    const stbi_uc v = *p;
                    if (len) {
                        do
                            *zout++ = v;
                        while (--len);
                    }
                }
                else if (len) {
                    do
                        *zout++ = *p++;
                    while (--len);
                }
            }
        }
    }

    static int stbi_compute_huffman_codes(stbi_zbuf* a)
    {
        static const stbi_uc length_dezigzag[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5,
                                                     11, 4, 12, 3, 13, 2, 14, 1, 15 };
        stbi_zhuffman z_codelength;
        stbi_uc lencodes[286 + 32 + 137];  // padding for maximum single op
        stbi_uc codelength_sizes[19];

        const int hlit = stbi_zreceive(a, 5) + 257;
        const int hdist = stbi_zreceive(a, 5) + 1;
        const int hclen = stbi_zreceive(a, 4) + 4;
        const int ntot = hlit + hdist;

        memset(codelength_sizes, 0, sizeof(codelength_sizes));
        for (int i = 0; i < hclen; ++i) {
            const int s = stbi_zreceive(a, 3);
            codelength_sizes[length_dezigzag[i]] = static_cast<stbi_uc>(s);
        }
        if (!stbi_zbuild_huffman(&z_codelength, codelength_sizes, 19))
            return 0;

        int n = 0;
        while (n < ntot) {
            int c = stbi_zhuffman_decode(a, &z_codelength);
            if (c < 0 || c >= 19)
                return stbi__err("bad codelengths", "Corrupt PNG");
            if (c < 16)
                lencodes[n++] = static_cast<stbi_uc>(c);
            else {
                stbi_uc fill = 0;
                if (c == 16) {
                    c = stbi_zreceive(a, 2) + 3;
                    if (n == 0)
                        return stbi__err("bad codelengths", "Corrupt PNG");
                    fill = lencodes[n - 1];
                }
                else if (c == 17) {
                    c = stbi_zreceive(a, 3) + 3;
                }
                else if (c == 18) {
                    c = stbi_zreceive(a, 7) + 11;
                }
                else {
                    return stbi__err("bad codelengths", "Corrupt PNG");
                }
                if (ntot - n < c)
                    return stbi__err("bad codelengths", "Corrupt PNG");
                memset(lencodes + n, fill, c);
                n += c;
            }
        }
        if (n != ntot)
            return stbi__err("bad codelengths", "Corrupt PNG");
        if (!stbi_zbuild_huffman(&a->z_length, lencodes, hlit))
            return 0;
        if (!stbi_zbuild_huffman(&a->z_distance, lencodes + hlit, hdist))
            return 0;
        return 1;
    }

    static int stbi_parse_uncompressed_block(stbi_zbuf* a)
    {
        stbi_uc header[4];
        if (a->num_bits & 7)
            stbi_zreceive(a, a->num_bits & 7);  // discard
        // drain the bit-packed data into header
        int k = 0;
        while (a->num_bits > 0) {
            header[k++] = static_cast<stbi_uc>(a->code_buffer & 255);  // suppress MSVC run-time
                                                                       // check
            a->code_buffer >>= 8;
            a->num_bits -= 8;
        }
        if (a->num_bits < 0)
            return stbi__err("zlib corrupt", "Corrupt PNG");
        // now fill header the normal way
        while (k < 4)
            header[k++] = stbi_zget8(a);
        const int len = header[1] * 256 + header[0];
        const int nlen = header[3] * 256 + header[2];
        if (nlen != (len ^ 0xffff))
            return stbi__err("zlib corrupt", "Corrupt PNG");
        if (a->zbuffer + len > a->zbuffer_end)
            return stbi__err("read past buffer", "Corrupt PNG");
        if (a->zout + len > a->zout_end)
            if (!stbi_zexpand(a, a->zout, len))
                return 0;
        memcpy(a->zout, a->zbuffer, len);
        a->zbuffer += len;
        a->zout += len;
        return 1;
    }

    static int stbi_parse_zlib_header(stbi_zbuf* a)
    {
        const int cmf = stbi_zget8(a);
        const int cm = cmf & 15;
        /* int cinfo = cmf >> 4; */
        const int flg = stbi_zget8(a);
        if (stbi_zeof(a))
            return stbi__err("bad zlib header", "Corrupt PNG");  // zlib spec
        if ((cmf * 256 + flg) % 31 != 0)
            return stbi__err("bad zlib header", "Corrupt PNG");  // zlib spec
        if (flg & 32)
            return stbi__err("no preset dict", "Corrupt PNG");  // preset dictionary not allowed in
                                                                // png
        if (cm != 8)
            return stbi__err("bad compression", "Corrupt PNG");  // DEFLATE required for png
        // window = 1 << (8 + cinfo)... but who cares, we fully buffer output
        return 1;
    }

    constexpr static stbi_uc stbi_zdefault_length[STBI_ZNSYMS] = {
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
        8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
        9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 7, 7, 7, 7, 7,
        7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8
    };
    constexpr static stbi_uc stbi_zdefault_distance[32] = { 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                                            5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
                                                            5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };

    /*
    Init algorithm:
    {
       int i;   // use <= to match clearly with spec
       for (i=0; i <= 143; ++i)     stbi__zdefault_length[i]   = 8;
       for (   ; i <= 255; ++i)     stbi__zdefault_length[i]   = 9;
       for (   ; i <= 279; ++i)     stbi__zdefault_length[i]   = 7;
       for (   ; i <= 287; ++i)     stbi__zdefault_length[i]   = 8;

       for (i=0; i <=  31; ++i)     stbi__zdefault_distance[i] = 5;
    }
    */

    static int stbi_parse_zlib(stbi_zbuf* a, const int parse_header)
    {
        int final;
        if (parse_header)
            if (!stbi_parse_zlib_header(a))
                return 0;
        a->num_bits = 0;
        a->code_buffer = 0;
        a->hit_zeof_once = 0;
        do {
            final = stbi_zreceive(a, 1);
            const int type = stbi_zreceive(a, 2);
            if (type == 0) {
                if (!stbi_parse_uncompressed_block(a))
                    return 0;
            }
            else if (type == 3) {
                return 0;
            }
            else {
                if (type == 1) {
                    // use fixed code lengths
                    if (!stbi_zbuild_huffman(&a->z_length, stbi_zdefault_length, STBI_ZNSYMS))
                        return 0;
                    if (!stbi_zbuild_huffman(&a->z_distance, stbi_zdefault_distance, 32))
                        return 0;
                }
                else {
                    if (!stbi_compute_huffman_codes(a))
                        return 0;
                }
                if (!stbi_parse_huffman_block(a))
                    return 0;
            }
        }
        while (!final);
        return 1;
    }

    static int stbi_do_zlib(stbi_zbuf* a, char* obuf, const int olen, const int exp,
                            const int parse_header)
    {
        a->zout_start = obuf;
        a->zout = obuf;
        a->zout_end = obuf + olen;
        a->z_expandable = exp;

        return stbi_parse_zlib(a, parse_header);
    }

    char* stbi_zlib_decode_malloc_guesssize(char* buffer, const int len, const int initial_size,
                                            int* outlen)
    {
        stbi_zbuf a;
        char* p = static_cast<char*>(stbi_malloc(initial_size));
        if (p == nullptr)
            return nullptr;
        a.zbuffer = (stbi_uc*)buffer;
        a.zbuffer_end = reinterpret_cast<stbi_uc*>(buffer + len);
        if (stbi_do_zlib(&a, p, initial_size, 1, 1)) {
            if (outlen)
                *outlen = static_cast<int>(a.zout - a.zout_start);
            return a.zout_start;
        }
        else {
            STBI_FREE(a.zout_start);
            return nullptr;
        }
    }

    char* stbi_zlib_decode_malloc(char* buffer, const int len, int* outlen)
    {
        return stbi_zlib_decode_malloc_guesssize(buffer, len, 16384, outlen);
    }

    char* stbi_zlib_decode_malloc_guesssize_headerflag(
        char* buffer, const int len, const int initial_size, int* outlen, const int parse_header)
    {
        stbi_zbuf a;
        char* p = static_cast<char*>(stbi_malloc(initial_size));
        if (p == nullptr)
            return nullptr;
        a.zbuffer = reinterpret_cast<stbi_uc*>(buffer);
        a.zbuffer_end = reinterpret_cast<stbi_uc*>(buffer + len);
        if (stbi_do_zlib(&a, p, initial_size, 1, parse_header)) {
            if (outlen)
                *outlen = static_cast<int>(a.zout - a.zout_start);
            return a.zout_start;
        }
        else {
            STBI_FREE(a.zout_start);
            return nullptr;
        }
    }

    int stbi_zlib_decode_buffer(char* obuffer, const int olen, char* ibuffer, const int ilen)
    {
        stbi_zbuf a;
        a.zbuffer = (stbi_uc*)ibuffer;
        a.zbuffer_end = (stbi_uc*)ibuffer + ilen;
        if (stbi_do_zlib(&a, obuffer, olen, 0, 1))
            return static_cast<int>(a.zout - a.zout_start);
        else
            return -1;
    }

    char* stbi_zlib_decode_noheader_malloc(char* buffer, const int len, int* outlen)
    {
        stbi_zbuf a;
        char* p = static_cast<char*>(stbi_malloc(16384));
        if (p == nullptr)
            return nullptr;
        a.zbuffer = reinterpret_cast<stbi_uc*>(buffer);
        a.zbuffer_end = reinterpret_cast<stbi_uc*>(buffer) + len;
        if (stbi_do_zlib(&a, p, 16384, 1, 0)) {
            if (outlen)
                *outlen = static_cast<int>(a.zout - a.zout_start);
            return a.zout_start;
        }
        else {
            STBI_FREE(a.zout_start);
            return nullptr;
        }
    }

    int stbi_zlib_decode_noheader_buffer(char* obuffer, const int olen, char* ibuffer,
                                         const int ilen)
    {
        stbi_zbuf a;
        a.zbuffer = reinterpret_cast<stbi_uc*>(ibuffer);
        a.zbuffer_end = reinterpret_cast<stbi_uc*>(ibuffer) + ilen;
        if (stbi_do_zlib(&a, obuffer, olen, 0, 0))
            return static_cast<int>(a.zout - a.zout_start);
        else
            return -1;
    }
#endif

    // public domain "baseline" PNG decoder   v0.10  Sean Barrett 2006-11-18
    //    simple implementation
    //      - only 8-bit samples
    //      - no CRC checking
    //      - allocates lots of intermediate memory
    //        - avoids problem of streaming data between subsystems
    //        - avoids explicit window management
    //    performance
    //      - uses stb_zlib, a PD zlib implementation with fast huffman decoding

#ifndef STBI_NO_PNG
    typedef struct
    {
        StbiUint32 length;
        StbiUint32 type;
    } stbi_pngchunk;

    stbi_pngchunk stbi_get_chunk_header(StbiContext* s)
    {
        stbi_pngchunk c;
        c.length = stbi_get32be(s);
        c.type = stbi_get32be(s);
        return c;
    }

    static int stbi_check_png_header(StbiContext* s)
    {
        static const stbi_uc png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
        for (int i = 0; i < 8; ++i)
            if (stbi_get8(s) != png_sig[i])
                return stbi__err("bad png sig", "Not a PNG");
        return 1;
    }

    typedef struct
    {
        StbiContext* s;
        stbi_uc *idata, *expanded, *out;
        int depth;
    } stbi_png;

    enum {
        STBI_F_none = 0,
        STBI_F_sub = 1,
        STBI_F_up = 2,
        STBI_F_avg = 3,
        STBI_F_paeth = 4,
        // synthetic filter used for first scanline to avoid needing a dummy row of 0s
        STBI_F_avg_first
    };

    static stbi_uc first_row_filter[5] = {
        STBI_F_none, STBI_F_sub, STBI_F_none, STBI_F_avg_first,
        STBI_F_sub  // Paeth with b=c=0 turns out to be equivalent to sub
    };

    static int stbi_paeth(const int a, const int b, const int c)
    {
        // This formulation looks very different from the reference in the PNG spec, but is
        // actually equivalent and has favorable data dependencies and admits straightforward
        // generation of branch-free code, which helps performance significantly.
        const int thresh = c * 3 - (a + b);
        const int lo = a < b ? a : b;
        const int hi = a < b ? b : a;
        const int t0 = (hi <= thresh) ? lo : c;
        const int t1 = (thresh <= lo) ? hi : t0;
        return t1;
    }

    static const stbi_uc stbi_depth_scale_table[9] = { 0, 0xff, 0x55, 0, 0x11, 0, 0, 0, 0x01 };

    // adds an extra all-255 alpha channel
    // dest == src is legal
    // img_n must be 1 or 3
    static void stbi_create_png_alpha_expand8(stbi_uc* dest, const stbi_uc* src, const StbiUint32 x,
                                              const int img_n)
    {
        int i;
        // must process data backwards since we allow dest==src
        if (img_n == 1) {
            for (i = x - 1; i >= 0; --i) {
                dest[i * 2 + 1] = 255;
                dest[i * 2 + 0] = src[i];
            }
        }
        else {
            STBI_ASSERT(img_n == 3);
            for (i = x - 1; i >= 0; --i) {
                dest[i * 4 + 3] = 255;
                dest[i * 4 + 2] = src[i * 3 + 2];
                dest[i * 4 + 1] = src[i * 3 + 1];
                dest[i * 4 + 0] = src[i * 3 + 0];
            }
        }
    }

    // create the png data from post-deflated data
    static int stbi_create_png_image_raw(stbi_png* a, const stbi_uc* raw, const StbiUint32 raw_len,
                                         const int out_n, const StbiUint32 x, const StbiUint32 y,
                                         const int depth, const int color)
    {
        const int bytes = (depth == 16 ? 2 : 1);
        const StbiContext* s = a->s;
        StbiUint32 i, stride = x * out_n * bytes;
        int all_ok = 1;
        int k;
        const int img_n = s->img_n;  // copy it into a local for later

        const int output_bytes = out_n * bytes;
        int filter_bytes = img_n * bytes;
        int width = x;

        STBI_ASSERT(out_n == s->img_n || out_n == s->img_n + 1);
        a->out = static_cast<stbi_uc*>(stbi_malloc_mad3(x, y, output_bytes, 0));  // extra bytes to
                                                                                  // write off the
                                                                                  // end into
        if (!a->out)
            return stbi__err("outofmem", "Out of memory");

        // note: error exits here don't need to clean up a->out individually,
        // stbi__do_png always does on error.
        if (!stbi_mad3sizes_valid(img_n, x, depth, 7))
            return stbi__err("too large", "Corrupt PNG");
        const StbiUint32 img_width_bytes = (((img_n * x * depth) + 7) >> 3);
        if (!stbi_mad2sizes_valid(img_width_bytes, y, img_width_bytes))
            return stbi__err("too large", "Corrupt PNG");
        const StbiUint32 img_len = (img_width_bytes + 1) * y;

        // we used to check for exact match between raw_len and img_len on non-interlaced PNGs,
        // but issue #276 reported a PNG in the wild that had extra data at the end (all zeros),
        // so just check for raw_len < img_len always.
        if (raw_len < img_len)
            return stbi__err("not enough pixels", "Corrupt PNG");

        // Allocate two scan lines worth of filter workspace buffer.
        stbi_uc* filter_buf = static_cast<stbi_uc*>(stbi_malloc_mad2(img_width_bytes, 2, 0));
        if (!filter_buf)
            return stbi__err("outofmem", "Out of memory");

        // Filtering for low-bit-depth images
        if (depth < 8) {
            filter_bytes = 1;
            width = img_width_bytes;
        }

        for (StbiUint32 j = 0; j < y; ++j) {
            // cur/prior filter buffers alternate
            stbi_uc* cur = filter_buf + (j & 1) * img_width_bytes;
            const stbi_uc* prior = filter_buf + (~j & 1) * img_width_bytes;
            stbi_uc* dest = a->out + stride * j;
            const int nk = width * filter_bytes;
            int filter = *raw++;

            // check filter type
            if (filter > 4) {
                all_ok = stbi__err("invalid filter", "Corrupt PNG");
                break;
            }

            // if first row, use special filter that doesn't sample previous row
            if (j == 0)
                filter = first_row_filter[filter];

            // perform actual filtering
            switch (filter) {
                case STBI_F_none:
                    memcpy(cur, raw, nk);
                    break;
                case STBI_F_sub:
                    memcpy(cur, raw, filter_bytes);
                    for (k = filter_bytes; k < nk; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + cur[k - filter_bytes]);
                    break;
                case STBI_F_up:
                    for (k = 0; k < nk; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + prior[k]);
                    break;
                case STBI_F_avg:
                    for (k = 0; k < filter_bytes; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + (prior[k] >> 1));
                    for (k = filter_bytes; k < nk; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + ((prior[k] + cur[k - filter_bytes]) >> 1));
                    break;
                case STBI_F_paeth:
                    for (k = 0; k < filter_bytes; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + prior[k]);  // prior[k] ==
                                                                    // stbi__paeth(0,prior[k],0)
                    for (k = filter_bytes; k < nk; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + stbi_paeth(cur[k - filter_bytes], prior[k],
                                                                   prior[k - filter_bytes]));
                    break;
                case STBI_F_avg_first:
                    memcpy(cur, raw, filter_bytes);
                    for (k = filter_bytes; k < nk; ++k)
                        cur[k] = STBI_BYTECAST(raw[k] + (cur[k - filter_bytes] >> 1));
                    break;
            }

            raw += nk;

            // expand decoded bits in cur to dest, also adding an extra alpha channel if desired
            if (depth < 8) {
                const stbi_uc scale = (color == 0) ? stbi_depth_scale_table[depth]
                                                   : 1;  // scale grayscale values to 0..255 range
                const stbi_uc* in = cur;
                stbi_uc* out = dest;
                stbi_uc inb = 0;
                const StbiUint32 nsmp = x * img_n;

                // expand bits to bytes first
                if (depth == 4) {
                    for (i = 0; i < nsmp; ++i) {
                        if ((i & 1) == 0)
                            inb = *in++;
                        *out++ = scale * (inb >> 4);
                        inb <<= 4;
                    }
                }
                else if (depth == 2) {
                    for (i = 0; i < nsmp; ++i) {
                        if ((i & 3) == 0)
                            inb = *in++;
                        *out++ = scale * (inb >> 6);
                        inb <<= 2;
                    }
                }
                else {
                    STBI_ASSERT(depth == 1);
                    for (i = 0; i < nsmp; ++i) {
                        if ((i & 7) == 0)
                            inb = *in++;
                        *out++ = scale * (inb >> 7);
                        inb <<= 1;
                    }
                }

                // insert alpha=255 values if desired
                if (img_n != out_n)
                    stbi_create_png_alpha_expand8(dest, dest, x, img_n);
            }
            else if (depth == 8) {
                if (img_n == out_n)
                    memcpy(dest, cur, x * img_n);
                else
                    stbi_create_png_alpha_expand8(dest, cur, x, img_n);
            }
            else if (depth == 16) {
                // convert the image data from big-endian to platform-native
                StbiUint16* dest16 = reinterpret_cast<StbiUint16*>(dest);
                const StbiUint32 nsmp = x * img_n;

                if (img_n == out_n) {
                    for (i = 0; i < nsmp; ++i, ++dest16, cur += 2)
                        *dest16 = static_cast<u16>((cur[0] << 8) | cur[1]);
                }
                else {
                    STBI_ASSERT(img_n + 1 == out_n);
                    if (img_n == 1) {
                        for (i = 0; i < x; ++i, dest16 += 2, cur += 2) {
                            dest16[0] = static_cast<u16>((cur[0] << 8) | cur[1]);
                            dest16[1] = 0xffff;
                        }
                    }
                    else {
                        STBI_ASSERT(img_n == 3);
                        for (i = 0; i < x; ++i, dest16 += 4, cur += 6) {
                            dest16[0] = static_cast<u16>((cur[0] << 8) | cur[1]);
                            dest16[1] = static_cast<u16>((cur[2] << 8) | cur[3]);
                            dest16[2] = static_cast<u16>((cur[4] << 8) | cur[5]);
                            dest16[3] = 0xffff;
                        }
                    }
                }
            }
        }

        STBI_FREE(filter_buf);
        if (!all_ok)
            return 0;

        return 1;
    }

    static int stbi_create_png_image(stbi_png* a, stbi_uc* image_data, StbiUint32 image_data_len,
                                     const int out_n, const int depth, const int color,
                                     const int interlaced)
    {
        const int bytes = (depth == 16 ? 2 : 1);
        const int out_bytes = out_n * bytes;
        if (!interlaced)
            return stbi_create_png_image_raw(a, image_data, image_data_len, out_n, a->s->img_x,
                                             a->s->img_y, depth, color);

        // de-interlacing
        stbi_uc* final = static_cast<stbi_uc*>(
            stbi_malloc_mad3(a->s->img_x, a->s->img_y, out_bytes, 0));
        if (!final)
            return stbi__err("outofmem", "Out of memory");
        for (int p = 0; p < 7; ++p) {
            const int xorig[] = { 0, 4, 0, 2, 0, 1, 0 };
            const int yorig[] = { 0, 0, 4, 0, 2, 0, 1 };
            const int xspc[] = { 8, 8, 4, 4, 2, 2, 1 };
            const int yspc[] = { 8, 8, 8, 4, 4, 2, 2 };
            // pass1_x[4] = 0, pass1_x[5] = 1, pass1_x[12] = 1
            const int x = (a->s->img_x - xorig[p] + xspc[p] - 1) / xspc[p];
            const int y = (a->s->img_y - yorig[p] + yspc[p] - 1) / yspc[p];
            if (x && y) {
                const StbiUint32 img_len = ((((a->s->img_n * x * depth) + 7) >> 3) + 1) * y;
                if (!stbi_create_png_image_raw(a, image_data, image_data_len, out_n, x, y, depth,
                                               color)) {
                    STBI_FREE(final);
                    return 0;
                }
                for (int j = 0; j < y; ++j) {
                    for (int i = 0; i < x; ++i) {
                        const int out_y = j * yspc[p] + yorig[p];
                        const int out_x = i * xspc[p] + xorig[p];
                        memcpy(final + out_y * a->s->img_x * out_bytes + out_x * out_bytes,
                               a->out + (j * x + i) * out_bytes, out_bytes);
                    }
                }
                STBI_FREE(a->out);
                image_data += img_len;
                image_data_len -= img_len;
            }
        }
        a->out = final;

        return 1;
    }

    static int stbi_compute_transparency(const stbi_png* z, stbi_uc tc[3], const int out_n)
    {
        const StbiContext* s = z->s;
        StbiUint32 i, pixel_count = s->img_x * s->img_y;
        stbi_uc* p = z->out;

        // compute color-based transparency, assuming we've
        // already got 255 as the alpha value in the output
        STBI_ASSERT(out_n == 2 || out_n == 4);

        if (out_n == 2) {
            for (i = 0; i < pixel_count; ++i) {
                p[1] = (p[0] == tc[0] ? 0 : 255);
                p += 2;
            }
        }
        else {
            for (i = 0; i < pixel_count; ++i) {
                if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
                    p[3] = 0;
                p += 4;
            }
        }
        return 1;
    }

    static int stbi_compute_transparency16(const stbi_png* z, StbiUint16 tc[3], const int out_n)
    {
        const StbiContext* s = z->s;
        StbiUint32 i, pixel_count = s->img_x * s->img_y;
        StbiUint16* p = reinterpret_cast<StbiUint16*>(z->out);

        // compute color-based transparency, assuming we've
        // already got 65535 as the alpha value in the output
        STBI_ASSERT(out_n == 2 || out_n == 4);

        if (out_n == 2) {
            for (i = 0; i < pixel_count; ++i) {
                p[1] = (p[0] == tc[0] ? 0 : 65535);
                p += 2;
            }
        }
        else {
            for (i = 0; i < pixel_count; ++i) {
                if (p[0] == tc[0] && p[1] == tc[1] && p[2] == tc[2])
                    p[3] = 0;
                p += 4;
            }
        }
        return 1;
    }

    static int stbi_expand_png_palette(stbi_png* a, const stbi_uc* palette, const int len,
                                       const int pal_img_n)
    {
        StbiUint32 i, pixel_count = a->s->img_x * a->s->img_y;
        const stbi_uc* orig = a->out;

        stbi_uc* p = static_cast<stbi_uc*>(stbi_malloc_mad2(pixel_count, pal_img_n, 0));
        if (p == nullptr)
            return stbi__err("outofmem", "Out of memory");

        // between here and free(out) below, exitting would leak
        stbi_uc* temp_out = p;

        if (pal_img_n == 3) {
            for (i = 0; i < pixel_count; ++i) {
                const int n = orig[i] * 4;
                p[0] = palette[n];
                p[1] = palette[n + 1];
                p[2] = palette[n + 2];
                p += 3;
            }
        }
        else {
            for (i = 0; i < pixel_count; ++i) {
                const int n = orig[i] * 4;
                p[0] = palette[n];
                p[1] = palette[n + 1];
                p[2] = palette[n + 2];
                p[3] = palette[n + 3];
                p += 4;
            }
        }
        STBI_FREE(a->out);
        a->out = temp_out;

        STBI_NOTUSED(len);

        return 1;
    }

    static int stbi__unpremultiply_on_load_global = 0;
    static int stbi__de_iphone_flag_global = 0;

    void stbi_set_unpremultiply_on_load(const int flag_true_if_should_unpremultiply)
    {
        stbi__unpremultiply_on_load_global = flag_true_if_should_unpremultiply;
    }

    void stbi_convert_iphone_png_to_rgb(const int flag_true_if_should_convert)
    {
        stbi__de_iphone_flag_global = flag_true_if_should_convert;
    }

  #ifndef STBI_THREAD_LOCAL
    #define stbi__unpremultiply_on_load stbi__unpremultiply_on_load_global
    #define stbi__de_iphone_flag        stbi__de_iphone_flag_global
  #else
    static STBI_THREAD_LOCAL int stbi__unpremultiply_on_load_local, stbi__unpremultiply_on_load_set;
    static STBI_THREAD_LOCAL int stbi__de_iphone_flag_local, stbi__de_iphone_flag_set;

    void stbi_set_unpremultiply_on_load_thread(const int flag_true_if_should_unpremultiply)
    {
        stbi__unpremultiply_on_load_local = flag_true_if_should_unpremultiply;
        stbi__unpremultiply_on_load_set = 1;
    }

    void stbi_convert_iphone_png_to_rgb_thread(const int flag_true_if_should_convert)
    {
        stbi__de_iphone_flag_local = flag_true_if_should_convert;
        stbi__de_iphone_flag_set = 1;
    }

    #define stbi_unpremultiply_on_load                                       \
        (stbi__unpremultiply_on_load_set ? stbi__unpremultiply_on_load_local \
                                         : stbi__unpremultiply_on_load_global)
    #define stbi_de_iphone_flag \
        (stbi__de_iphone_flag_set ? stbi__de_iphone_flag_local : stbi__de_iphone_flag_global)
  #endif  // STBI_THREAD_LOCAL

    static void stbi_de_iphone(const stbi_png* z)
    {
        const StbiContext* s = z->s;
        StbiUint32 i, pixel_count = s->img_x * s->img_y;
        stbi_uc* p = z->out;

        if (s->img_out_n == 3) {  // convert bgr to rgb
            for (i = 0; i < pixel_count; ++i) {
                const stbi_uc t = p[0];
                p[0] = p[2];
                p[2] = t;
                p += 3;
            }
        }
        else {
            STBI_ASSERT(s->img_out_n == 4);
            if (stbi_unpremultiply_on_load) {
                // convert bgr to rgb and unpremultiply
                for (i = 0; i < pixel_count; ++i) {
                    const stbi_uc a = p[3];
                    const stbi_uc t = p[0];
                    if (a) {
                        const stbi_uc half = a / 2;
                        p[0] = (p[2] * 255 + half) / a;
                        p[1] = (p[1] * 255 + half) / a;
                        p[2] = (t * 255 + half) / a;
                    }
                    else {
                        p[0] = p[2];
                        p[2] = t;
                    }
                    p += 4;
                }
            }
            else {
                // convert bgr to rgb
                for (i = 0; i < pixel_count; ++i) {
                    const stbi_uc t = p[0];
                    p[0] = p[2];
                    p[2] = t;
                    p += 4;
                }
            }
        }
    }

  #define STBI_PNG_TYPE(a, b, c, d) \
      (((unsigned)(a) << 24) + ((unsigned)(b) << 16) + ((unsigned)(c) << 8) + (unsigned)(d))

    static int stbi_parse_png_file(stbi_png* z, const int scan, const int req_comp)
    {
        stbi_uc palette[1024], pal_img_n = 0;
        stbi_uc has_trans = 0, tc[3] = { 0 };
        StbiUint16 tc16[3];
        StbiUint32 ioff = 0, idata_limit = 0, i, pal_len = 0;
        int first = 1, k, interlace = 0, color = 0, is_iphone = 0;
        StbiContext* s = z->s;

        z->expanded = nullptr;
        z->idata = nullptr;
        z->out = nullptr;

        if (!stbi_check_png_header(s))
            return 0;

        if (scan == STBI_SCAN_type)
            return 1;

        for (;;) {
            const stbi_pngchunk c = stbi_get_chunk_header(s);
            switch (c.type) {
                case STBI_PNG_TYPE('C', 'g', 'B', 'I'):
                    is_iphone = 1;
                    stbi_skip(s, c.length);
                    break;
                case STBI_PNG_TYPE('I', 'H', 'D', 'R'):
                {
                    if (!first)
                        return stbi__err("multiple IHDR", "Corrupt PNG");
                    first = 0;
                    if (c.length != 13)
                        return stbi__err("bad IHDR len", "Corrupt PNG");
                    s->img_x = stbi_get32be(s);
                    s->img_y = stbi_get32be(s);
                    if (s->img_y > STBI_MAX_DIMENSIONS)
                        return stbi__err("too large", "Very large image (corrupt?)");
                    if (s->img_x > STBI_MAX_DIMENSIONS)
                        return stbi__err("too large", "Very large image (corrupt?)");
                    z->depth = stbi_get8(s);
                    if (z->depth != 1 && z->depth != 2 && z->depth != 4 && z->depth != 8 &&
                        z->depth != 16)
                        return stbi__err("1/2/4/8/16-bit only",
                                         "PNG not supported: 1/2/4/8/16-bit only");
                    color = stbi_get8(s);
                    if (color > 6)
                        return stbi__err("bad ctype", "Corrupt PNG");
                    if (color == 3 && z->depth == 16)
                        return stbi__err("bad ctype", "Corrupt PNG");
                    if (color == 3)
                        pal_img_n = 3;
                    else if (color & 1)
                        return stbi__err("bad ctype", "Corrupt PNG");
                    const int comp = stbi_get8(s);
                    if (comp)
                        return stbi__err("bad comp method", "Corrupt PNG");
                    const int filter = stbi_get8(s);
                    if (filter)
                        return stbi__err("bad filter method", "Corrupt PNG");
                    interlace = stbi_get8(s);
                    if (interlace > 1)
                        return stbi__err("bad interlace method", "Corrupt PNG");
                    if (!s->img_x || !s->img_y)
                        return stbi__err("0-pixel image", "Corrupt PNG");
                    if (!pal_img_n) {
                        s->img_n = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);
                        if ((1 << 30) / s->img_x / s->img_n < s->img_y)
                            return stbi__err("too large", "Image too large to decode");
                    }
                    else {
                        // if paletted, then pal_n is our final components, and
                        // img_n is # components to decompress/filter.
                        s->img_n = 1;
                        if ((1 << 30) / s->img_x / 4 < s->img_y)
                            return stbi__err("too large", "Corrupt PNG");
                    }
                    // even with SCAN_header, have to scan to see if we have a tRNS
                    break;
                }

                case STBI_PNG_TYPE('P', 'L', 'T', 'E'):
                {
                    if (first)
                        return stbi__err("first not IHDR", "Corrupt PNG");
                    if (c.length > 256 * 3)
                        return stbi__err("invalid PLTE", "Corrupt PNG");
                    pal_len = c.length / 3;
                    if (pal_len * 3 != c.length)
                        return stbi__err("invalid PLTE", "Corrupt PNG");
                    for (i = 0; i < pal_len; ++i) {
                        palette[i * 4 + 0] = stbi_get8(s);
                        palette[i * 4 + 1] = stbi_get8(s);
                        palette[i * 4 + 2] = stbi_get8(s);
                        palette[i * 4 + 3] = 255;
                    }
                    break;
                }

                case STBI_PNG_TYPE('t', 'R', 'N', 'S'):
                {
                    if (first)
                        return stbi__err("first not IHDR", "Corrupt PNG");
                    if (z->idata)
                        return stbi__err("tRNS after IDAT", "Corrupt PNG");
                    if (pal_img_n) {
                        if (scan == STBI_SCAN_header) {
                            s->img_n = 4;
                            return 1;
                        }
                        if (pal_len == 0)
                            return stbi__err("tRNS before PLTE", "Corrupt PNG");
                        if (c.length > pal_len)
                            return stbi__err("bad tRNS len", "Corrupt PNG");
                        pal_img_n = 4;
                        for (i = 0; i < c.length; ++i)
                            palette[i * 4 + 3] = stbi_get8(s);
                    }
                    else {
                        if (!(s->img_n & 1))
                            return stbi__err("tRNS with alpha", "Corrupt PNG");
                        if (c.length != static_cast<StbiUint32>(s->img_n) * 2)
                            return stbi__err("bad tRNS len", "Corrupt PNG");
                        has_trans = 1;
                        // non-paletted with tRNS = constant alpha. if header-scanning, we can stop
                        // now.
                        if (scan == STBI_SCAN_header) {
                            ++s->img_n;
                            return 1;
                        }
                        if (z->depth == 16)
                            for (k = 0; k < s->img_n; ++k)
                                tc16[k] = static_cast<StbiUint16>(stbi_get16be(s));  // copy the
                                                                                     // values
                                                                                     // as-is
                        else
                            for (k = 0; k < s->img_n; ++k)
                                tc[k] = static_cast<stbi_uc>(stbi_get16be(s) & 255) *
                                        stbi_depth_scale_table[z->depth];  // non 8-bit images will
                                                                           // be larger
                    }
                    break;
                }

                case STBI_PNG_TYPE('I', 'D', 'A', 'T'):
                {
                    if (first)
                        return stbi__err("first not IHDR", "Corrupt PNG");
                    if (pal_img_n && !pal_len)
                        return stbi__err("no PLTE", "Corrupt PNG");
                    if (scan == STBI_SCAN_header) {
                        // header scan definitely stops at first IDAT
                        if (pal_img_n)
                            s->img_n = pal_img_n;
                        return 1;
                    }
                    if (c.length > (1u << 30))
                        return stbi__err("IDAT size limit", "IDAT section larger than 2^30 bytes");
                    if (static_cast<int>(ioff + c.length) < static_cast<int>(ioff))
                        return 0;
                    if (ioff + c.length > idata_limit) {
                        const StbiUint32 idata_limit_old = idata_limit;
                        if (idata_limit == 0)
                            idata_limit = c.length > 4096 ? c.length : 4096;
                        while (ioff + c.length > idata_limit)
                            idata_limit *= 2;
                        STBI_NOTUSED(idata_limit_old);
                        stbi_uc* p = static_cast<stbi_uc*>(
                            STBI_REALLOC_SIZED(z->idata, idata_limit_old, idata_limit));
                        if (p == nullptr)
                            return stbi__err("outofmem", "Out of memory");
                        z->idata = p;
                    }
                    if (!stbi_getn(s, z->idata + ioff, c.length))
                        return stbi__err("outofdata", "Corrupt PNG");
                    ioff += c.length;
                    break;
                }

                case STBI_PNG_TYPE('I', 'E', 'N', 'D'):
                {
                    StbiUint32 raw_len;
                    if (first)
                        return stbi__err("first not IHDR", "Corrupt PNG");
                    if (scan != STBI_SCAN_load)
                        return 1;
                    if (z->idata == nullptr)
                        return stbi__err("no IDAT", "Corrupt PNG");
                    // initial guess for decoded data size to avoid unnecessary reallocs
                    const StbiUint32 bpl = (s->img_x * z->depth + 7) / 8;  // bytes per line, per
                                                                           // component
                    raw_len = bpl * s->img_y * s->img_n                    /* pixels */
                            + s->img_y /* filter mode per row */;
                    z->expanded = (stbi_uc*)stbi_zlib_decode_malloc_guesssize_headerflag(
                        (char*)z->idata, ioff, raw_len, (int*)&raw_len, !is_iphone);
                    if (z->expanded == nullptr)
                        return 0;  // zlib should set error
                    STBI_FREE(z->idata);
                    z->idata = nullptr;
                    if ((req_comp == s->img_n + 1 && req_comp != 3 && !pal_img_n) || has_trans)
                        s->img_out_n = s->img_n + 1;
                    else
                        s->img_out_n = s->img_n;
                    if (!stbi_create_png_image(z, z->expanded, raw_len, s->img_out_n, z->depth,
                                               color, interlace))
                        return 0;
                    if (has_trans) {
                        if (z->depth == 16) {
                            if (!stbi_compute_transparency16(z, tc16, s->img_out_n))
                                return 0;
                        }
                        else {
                            if (!stbi_compute_transparency(z, tc, s->img_out_n))
                                return 0;
                        }
                    }
                    if (is_iphone && stbi_de_iphone_flag && s->img_out_n > 2)
                        stbi_de_iphone(z);
                    if (pal_img_n) {
                        // pal_img_n == 3 or 4
                        s->img_n = pal_img_n;  // record the actual colors we had
                        s->img_out_n = pal_img_n;
                        if (req_comp >= 3)
                            s->img_out_n = req_comp;
                        if (!stbi_expand_png_palette(z, palette, pal_len, s->img_out_n))
                            return 0;
                    }
                    else if (has_trans) {
                        // non-paletted image with tRNS -> source image has (constant) alpha
                        ++s->img_n;
                    }
                    STBI_FREE(z->expanded);
                    z->expanded = nullptr;
                    // end of PNG chunk, read and skip CRC
                    stbi_get32be(s);
                    return 1;
                }

                default:
                    // if critical, fail
                    if (first)
                        return stbi__err("first not IHDR", "Corrupt PNG");
                    if ((c.type & (1 << 29)) == 0) {
  #ifndef STBI_NO_FAILURE_STRINGS
                        // not threadsafe
                        static char invalid_chunk[] = "XXXX PNG chunk not known";
                        invalid_chunk[0] = STBI_BYTECAST(c.type >> 24);
                        invalid_chunk[1] = STBI_BYTECAST(c.type >> 16);
                        invalid_chunk[2] = STBI_BYTECAST(c.type >> 8);
                        invalid_chunk[3] = STBI_BYTECAST(c.type >> 0);
  #endif
                        return stbi__err(invalid_chunk, "PNG not supported: unknown PNG chunk type");
                    }
                    stbi_skip(s, c.length);
                    break;
            }
            // end of PNG chunk, read and skip CRC
            stbi_get32be(s);
        }
    }

    static void* stbi_do_png(stbi_png* p, int* x, int* y, int* n, const int req_comp,
                             StbiResultInfo* ri)
    {
        void* result = nullptr;
        if (req_comp < 0 || req_comp > 4)
            return stbi_errpuc("bad req_comp", "Internal error");
        if (stbi_parse_png_file(p, STBI_SCAN_load, req_comp)) {
            if (p->depth <= 8)
                ri->bits_per_channel = 8;
            else if (p->depth == 16)
                ri->bits_per_channel = 16;
            else
                return stbi_errpuc("bad bits_per_channel",
                                   "PNG not supported: unsupported color depth");
            result = p->out;
            p->out = nullptr;
            if (req_comp && req_comp != p->s->img_out_n) {
                if (ri->bits_per_channel == 8)
                    result = stbi_convert_format(static_cast<unsigned char*>(result),
                                                 p->s->img_out_n, req_comp, p->s->img_x,
                                                 p->s->img_y);
                else
                    result = stbi_convert_format16(static_cast<StbiUint16*>(result),
                                                   p->s->img_out_n, req_comp, p->s->img_x,
                                                   p->s->img_y);
                p->s->img_out_n = req_comp;
                if (result == nullptr)
                    return result;
            }
            *x = p->s->img_x;
            *y = p->s->img_y;
            if (n)
                *n = p->s->img_n;
        }
        STBI_FREE(p->out);
        p->out = nullptr;
        STBI_FREE(p->expanded);
        p->expanded = nullptr;
        STBI_FREE(p->idata);
        p->idata = nullptr;

        return result;
    }

    static void* stbi_png_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                               StbiResultInfo* ri)
    {
        stbi_png p;
        p.s = s;
        return stbi_do_png(&p, x, y, comp, req_comp, ri);
    }

    static int stbi_png_test(StbiContext* s)
    {
        const int r = stbi_check_png_header(s);
        stbi_rewind(s);
        return r;
    }

    static int stbi_png_info_raw(stbi_png* p, int* x, int* y, int* comp)
    {
        if (!stbi_parse_png_file(p, STBI_SCAN_header, 0)) {
            stbi_rewind(p->s);
            return 0;
        }
        if (x)
            *x = p->s->img_x;
        if (y)
            *y = p->s->img_y;
        if (comp)
            *comp = p->s->img_n;
        return 1;
    }

    static int stbi_png_info(StbiContext* s, int* x, int* y, int* comp)
    {
        stbi_png p;
        p.s = s;
        return stbi_png_info_raw(&p, x, y, comp);
    }

    static int stbi_png_is16(StbiContext* s)
    {
        stbi_png p;
        p.s = s;
        if (!stbi_png_info_raw(&p, nullptr, nullptr, nullptr))
            return 0;
        if (p.depth != 16) {
            stbi_rewind(p.s);
            return 0;
        }
        return 1;
    }
#endif

    // Microsoft/Windows BMP image

#ifndef STBI_NO_BMP
    static int stbi_bmp_test_raw(StbiContext* s)
    {
        if (stbi_get8(s) != 'B')
            return 0;
        if (stbi_get8(s) != 'M')
            return 0;
        stbi_get32le(s);  // discard filesize
        stbi_get16le(s);  // discard reserved
        stbi_get16le(s);  // discard reserved
        stbi_get32le(s);  // discard data offset
        const int sz = stbi_get32le(s);
        const int r = (sz == 12 || sz == 40 || sz == 56 || sz == 108 || sz == 124);
        return r;
    }

    static int stbi_bmp_test(StbiContext* s)
    {
        const int r = stbi_bmp_test_raw(s);
        stbi_rewind(s);
        return r;
    }

    // returns 0..31 for the highest set bit
    static int stbi_high_bit(unsigned int z)
    {
        int n = 0;
        if (z == 0)
            return -1;
        if (z >= 0x10000) {
            n += 16;
            z >>= 16;
        }
        if (z >= 0x00100) {
            n += 8;
            z >>= 8;
        }
        if (z >= 0x00010) {
            n += 4;
            z >>= 4;
        }
        if (z >= 0x00004) {
            n += 2;
            z >>= 2;
        }
        if (z >= 0x00002)
            n += 1; /* >>=  1;*/
        return n;
    }

    static int stbi_bitcount(unsigned int a)
    {
        a = (a & 0x55555555) + ((a >> 1) & 0x55555555);  // max 2
        a = (a & 0x33333333) + ((a >> 2) & 0x33333333);  // max 4
        a = (a + (a >> 4)) & 0x0f0f0f0f;                 // max 8 per 4, now 8 bits
        a = (a + (a >> 8));                              // max 16 per 8 bits
        a = (a + (a >> 16));                             // max 32 per 8 bits
        return a & 0xff;
    }

    // extract an arbitrarily-aligned N-bit value (N=bits)
    // from v, and then make it 8-bits long and fractionally
    // extend it to full full range.
    static int stbi_shiftsigned(unsigned int v, const int shift, const int bits)
    {
        constexpr static i32 mul_table[] = {
            0x00 /* 0000 0000 */,
            0xff /* 1111 1111 */,
            0x55 /* 0101 0101 */,
            0x49 /* 0100 1001 */,
            0x11 /* 0001 0001 */,
            0x21 /* 0010 0001 */,
            0x41 /* 0100 0001 */,
            0x81 /* 1000 0001 */,
            0x01 /* 0000 0001 */,
        };

        constexpr static std::array shift_table{ 0U, 0U, 0U, 1U, 0U, 2U, 4U, 6U, 0U };

        if (shift < 0)
            v <<= -shift;
        else
            v >>= shift;
        STBI_ASSERT(v < 256);
        v >>= (8 - bits);
        STBI_ASSERT(bits >= 0 && bits <= 8);
        return static_cast<int>((unsigned)v * mul_table[bits]) >> shift_table[bits];
    }

    typedef struct
    {
        int bpp, offset, hsz;
        unsigned int mr, mg, mb, ma, all_a;
        int extra_read;
    } stbi_bmp_data;

    static int stbi_bmp_set_mask_defaults(stbi_bmp_data* info, const int compress)
    {
        // BI_BITFIELDS specifies masks explicitly, don't override
        if (compress == 3)
            return 1;

        if (compress == 0) {
            if (info->bpp == 16) {
                info->mr = 31u << 10;
                info->mg = 31u << 5;
                info->mb = 31u << 0;
            }
            else if (info->bpp == 32) {
                info->mr = 0xffu << 16;
                info->mg = 0xffu << 8;
                info->mb = 0xffu << 0;
                info->ma = 0xffu << 24;
                info->all_a = 0;  // if all_a is 0 at end, then we loaded alpha channel but it was
                                  // all 0
            }
            else {
                // otherwise, use defaults, which is all-0
                info->mr = info->mg = info->mb = info->ma = 0;
            }
            return 1;
        }
        return 0;  // error
    }

    static void* stbi_bmp_parse_header(StbiContext* s, stbi_bmp_data* info)
    {
        int hsz;
        if (stbi_get8(s) != 'B' || stbi_get8(s) != 'M')
            return stbi_errpuc("not BMP", "Corrupt BMP");
        stbi_get32le(s);  // discard filesize
        stbi_get16le(s);  // discard reserved
        stbi_get16le(s);  // discard reserved
        info->offset = stbi_get32le(s);
        info->hsz = hsz = stbi_get32le(s);
        info->mr = info->mg = info->mb = info->ma = 0;
        info->extra_read = 14;

        if (info->offset < 0)
            return stbi_errpuc("bad BMP", "bad BMP");

        if (hsz != 12 && hsz != 40 && hsz != 56 && hsz != 108 && hsz != 124)
            return stbi_errpuc("unknown BMP", "BMP type not supported: unknown");
        if (hsz == 12) {
            s->img_x = stbi_get16le(s);
            s->img_y = stbi_get16le(s);
        }
        else {
            s->img_x = stbi_get32le(s);
            s->img_y = stbi_get32le(s);
        }
        if (stbi_get16le(s) != 1)
            return stbi_errpuc("bad BMP", "bad BMP");
        info->bpp = stbi_get16le(s);
        if (hsz != 12) {
            const int compress = stbi_get32le(s);
            if (compress == 1 || compress == 2)
                return stbi_errpuc("BMP RLE", "BMP type not supported: RLE");
            if (compress >= 4)
                return stbi_errpuc("BMP JPEG/PNG",
                                   "BMP type not supported: unsupported compression");  // this
                                                                                        // includes
                                                                                        // PNG/JPEG
                                                                                        // modes
            if (compress == 3 && info->bpp != 16 && info->bpp != 32)
                return stbi_errpuc("bad BMP", "bad BMP");  // bitfields requires 16 or 32
                                                           // bits/pixel
            stbi_get32le(s);                               // discard sizeof
            stbi_get32le(s);                               // discard hres
            stbi_get32le(s);                               // discard vres
            stbi_get32le(s);                               // discard colorsused
            stbi_get32le(s);                               // discard max important
            if (hsz == 40 || hsz == 56) {
                if (hsz == 56) {
                    stbi_get32le(s);
                    stbi_get32le(s);
                    stbi_get32le(s);
                    stbi_get32le(s);
                }
                if (info->bpp == 16 || info->bpp == 32) {
                    if (compress == 0) {
                        stbi_bmp_set_mask_defaults(info, compress);
                    }
                    else if (compress == 3) {
                        info->mr = stbi_get32le(s);
                        info->mg = stbi_get32le(s);
                        info->mb = stbi_get32le(s);
                        info->extra_read += 12;
                        // not documented, but generated by photoshop and handled by mspaint
                        if (info->mr == info->mg && info->mg == info->mb) {
                            // ?!?!?
                            return stbi_errpuc("bad BMP", "bad BMP");
                        }
                    }
                    else
                        return stbi_errpuc("bad BMP", "bad BMP");
                }
            }
            else {
                // V4/V5 header
                if (hsz != 108 && hsz != 124)
                    return stbi_errpuc("bad BMP", "bad BMP");
                info->mr = stbi_get32le(s);
                info->mg = stbi_get32le(s);
                info->mb = stbi_get32le(s);
                info->ma = stbi_get32le(s);
                if (compress != 3)  // override mr/mg/mb unless in BI_BITFIELDS mode, as per docs
                    stbi_bmp_set_mask_defaults(info, compress);
                stbi_get32le(s);  // discard color space
                for (int i = 0; i < 12; ++i)
                    stbi_get32le(s);  // discard color space parameters
                if (hsz == 124) {
                    stbi_get32le(s);  // discard rendering intent
                    stbi_get32le(s);  // discard offset of profile data
                    stbi_get32le(s);  // discard size of profile data
                    stbi_get32le(s);  // discard reserved
                }
            }
        }
        return (void*)1;
    }

    static void* stbi_bmp_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                               StbiResultInfo* ri)
    {
        stbi_uc* out;
        unsigned int mr = 0, mg = 0, mb = 0, ma = 0, all_a;
        stbi_uc pal[256][4];
        int psize = 0, i, j, width;
        int flip_vertically, pad, target;
        stbi_bmp_data info;
        STBI_NOTUSED(ri);

        info.all_a = 255;
        if (stbi_bmp_parse_header(s, &info) == nullptr)
            return nullptr;  // error code already set

        flip_vertically = static_cast<int>(s->img_y) > 0;
        s->img_y = abs(static_cast<int>(s->img_y));

        if (s->img_y > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");
        if (s->img_x > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");

        mr = info.mr;
        mg = info.mg;
        mb = info.mb;
        ma = info.ma;
        all_a = info.all_a;

        if (info.hsz == 12) {
            if (info.bpp < 24)
                psize = (info.offset - info.extra_read - 24) / 3;
        }
        else {
            if (info.bpp < 16)
                psize = (info.offset - info.extra_read - info.hsz) >> 2;
        }
        if (psize == 0) {
            // accept some number of extra bytes after the header, but if the offset points either
            // to before the header ends or implies a large amount of extra data, reject the file as
            // malformed
            int bytes_read_so_far = s->callback_already_read +
                                    static_cast<int>(s->img_buffer - s->img_buffer_original);
            int header_limit = 1024;         // max we actually read is below 256 bytes currently.
            int extra_data_limit = 256 * 4;  // what ordinarily goes here is a palette; 256
                                             // entries*4 bytes is its max size.
            if (bytes_read_so_far <= 0 || bytes_read_so_far > header_limit)
                return stbi_errpuc("bad header", "Corrupt BMP");
            // we established that bytes_read_so_far is positive and sensible.
            // the first half of this test rejects offsets that are either too small positives, or
            // negative, and guarantees that info.offset >= bytes_read_so_far > 0. this in turn
            // ensures the number computed in the second half of the test can't overflow.
            if (info.offset < bytes_read_so_far ||
                info.offset - bytes_read_so_far > extra_data_limit)
                return stbi_errpuc("bad offset", "Corrupt BMP");
            else
                stbi_skip(s, info.offset - bytes_read_so_far);
        }

        if (info.bpp == 24 && ma == 0xff000000)
            s->img_n = 3;
        else
            s->img_n = ma ? 4 : 3;
        if (req_comp && req_comp >= 3)  // we can directly decode 3 or 4
            target = req_comp;
        else
            target = s->img_n;  // if they want monochrome, we'll post-convert

        // sanity-check size
        if (!stbi_mad3sizes_valid(target, s->img_x, s->img_y, 0))
            return stbi_errpuc("too large", "Corrupt BMP");

        out = static_cast<stbi_uc*>(stbi_malloc_mad3(target, s->img_x, s->img_y, 0));
        if (!out)
            return stbi_errpuc("outofmem", "Out of memory");
        if (info.bpp < 16) {
            int z = 0;
            if (psize == 0 || psize > 256) {
                STBI_FREE(out);
                return stbi_errpuc("invalid", "Corrupt BMP");
            }
            for (i = 0; i < psize; ++i) {
                pal[i][2] = stbi_get8(s);
                pal[i][1] = stbi_get8(s);
                pal[i][0] = stbi_get8(s);
                if (info.hsz != 12)
                    stbi_get8(s);
                pal[i][3] = 255;
            }
            stbi_skip(s,
                      info.offset - info.extra_read - info.hsz - psize * (info.hsz == 12 ? 3 : 4));
            if (info.bpp == 1)
                width = (s->img_x + 7) >> 3;
            else if (info.bpp == 4)
                width = (s->img_x + 1) >> 1;
            else if (info.bpp == 8)
                width = s->img_x;
            else {
                STBI_FREE(out);
                return stbi_errpuc("bad bpp", "Corrupt BMP");
            }
            pad = (-width) & 3;
            if (info.bpp == 1) {
                for (j = 0; j < static_cast<int>(s->img_y); ++j) {
                    int bit_offset = 7, v = stbi_get8(s);
                    for (i = 0; i < static_cast<int>(s->img_x); ++i) {
                        int color = (v >> bit_offset) & 0x1;
                        out[z++] = pal[color][0];
                        out[z++] = pal[color][1];
                        out[z++] = pal[color][2];
                        if (target == 4)
                            out[z++] = 255;
                        if (i + 1 == static_cast<int>(s->img_x))
                            break;
                        if ((--bit_offset) < 0) {
                            bit_offset = 7;
                            v = stbi_get8(s);
                        }
                    }
                    stbi_skip(s, pad);
                }
            }
            else {
                for (j = 0; j < static_cast<int>(s->img_y); ++j) {
                    for (i = 0; i < static_cast<int>(s->img_x); i += 2) {
                        int v = stbi_get8(s), v2 = 0;
                        if (info.bpp == 4) {
                            v2 = v & 15;
                            v >>= 4;
                        }
                        out[z++] = pal[v][0];
                        out[z++] = pal[v][1];
                        out[z++] = pal[v][2];
                        if (target == 4)
                            out[z++] = 255;
                        if (i + 1 == static_cast<int>(s->img_x))
                            break;
                        v = (info.bpp == 8) ? stbi_get8(s) : v2;
                        out[z++] = pal[v][0];
                        out[z++] = pal[v][1];
                        out[z++] = pal[v][2];
                        if (target == 4)
                            out[z++] = 255;
                    }
                    stbi_skip(s, pad);
                }
            }
        }
        else {
            int rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0,
                acount = 0;
            int z = 0;
            int easy = 0;
            stbi_skip(s, info.offset - info.extra_read - info.hsz);
            if (info.bpp == 24)
                width = 3 * s->img_x;
            else if (info.bpp == 16)
                width = 2 * s->img_x;
            else /* bpp = 32 and pad = 0 */
                width = 0;
            pad = (-width) & 3;
            if (info.bpp == 24) {
                easy = 1;
            }
            else if (info.bpp == 32) {
                if (mb == 0xff && mg == 0xff00 && mr == 0x00ff0000 && ma == 0xff000000)
                    easy = 2;
            }
            if (!easy) {
                if (!mr || !mg || !mb) {
                    STBI_FREE(out);
                    return stbi_errpuc("bad masks", "Corrupt BMP");
                }
                // right shift amt to put high bit in position #7
                rshift = stbi_high_bit(mr) - 7;
                rcount = stbi_bitcount(mr);
                gshift = stbi_high_bit(mg) - 7;
                gcount = stbi_bitcount(mg);
                bshift = stbi_high_bit(mb) - 7;
                bcount = stbi_bitcount(mb);
                ashift = stbi_high_bit(ma) - 7;
                acount = stbi_bitcount(ma);
                if (rcount > 8 || gcount > 8 || bcount > 8 || acount > 8) {
                    STBI_FREE(out);
                    return stbi_errpuc("bad masks", "Corrupt BMP");
                }
            }
            for (j = 0; j < static_cast<int>(s->img_y); ++j) {
                if (easy) {
                    for (i = 0; i < static_cast<int>(s->img_x); ++i) {
                        unsigned char a;
                        out[z + 2] = stbi_get8(s);
                        out[z + 1] = stbi_get8(s);
                        out[z + 0] = stbi_get8(s);
                        z += 3;
                        a = (easy == 2 ? stbi_get8(s) : 255);
                        all_a |= a;
                        if (target == 4)
                            out[z++] = a;
                    }
                }
                else {
                    int bpp = info.bpp;
                    for (i = 0; i < static_cast<int>(s->img_x); ++i) {
                        StbiUint32 v = (bpp == 16 ? static_cast<StbiUint32>(stbi_get16le(s))
                                                  : stbi_get32le(s));
                        unsigned int a;
                        out[z++] = STBI_BYTECAST(stbi_shiftsigned(v & mr, rshift, rcount));
                        out[z++] = STBI_BYTECAST(stbi_shiftsigned(v & mg, gshift, gcount));
                        out[z++] = STBI_BYTECAST(stbi_shiftsigned(v & mb, bshift, bcount));
                        a = (ma ? stbi_shiftsigned(v & ma, ashift, acount) : 255);
                        all_a |= a;
                        if (target == 4)
                            out[z++] = STBI_BYTECAST(a);
                    }
                }
                stbi_skip(s, pad);
            }
        }

        // if alpha channel is all 0s, replace with all 255s
        if (target == 4 && all_a == 0)
            for (i = 4 * s->img_x * s->img_y - 1; i >= 0; i -= 4)
                out[i] = 255;

        if (flip_vertically) {
            stbi_uc t;
            for (j = 0; j < static_cast<int>(s->img_y) >> 1; ++j) {
                stbi_uc* p1 = out + j * s->img_x * target;
                stbi_uc* p2 = out + (s->img_y - 1 - j) * s->img_x * target;
                for (i = 0; i < static_cast<int>(s->img_x) * target; ++i) {
                    t = p1[i];
                    p1[i] = p2[i];
                    p2[i] = t;
                }
            }
        }

        if (req_comp && req_comp != target) {
            out = stbi_convert_format(out, target, req_comp, s->img_x, s->img_y);
            if (out == nullptr)
                return out;  // stbi__convert_format frees input on failure
        }

        *x = s->img_x;
        *y = s->img_y;
        if (comp)
            *comp = s->img_n;
        return out;
    }
#endif

// Targa Truevision - TGA
// by Jonathan Dummer
#ifndef STBI_NO_TGA
    // returns STBI_rgb or whatever, 0 on error
    static int stbi_tga_get_comp(const int bits_per_pixel, const int is_grey, int* is_rgb16)
    {
        // only RGB or RGBA (incl. 16bit) or grey allowed
        if (is_rgb16)
            *is_rgb16 = 0;
        switch (bits_per_pixel) {
            case 8:
                return STBI_grey;

            case 16:
                if (is_grey)
                    return STBI_grey_alpha;
                [[fallthrough]];
            case 15:
                if (is_rgb16)
                    *is_rgb16 = 1;
                return STBI_rgb;

            case 24:
                [[fallthrough]];
            case 32:
                return bits_per_pixel / 8;
            default:
                return 0;
        }
    }

    static int stbi_tga_info(StbiContext* s, int* x, int* y, int* comp)
    {
        int tga_comp, tga_colormap_bpp;
        stbi_get8(s);                                // discard Offset
        const int tga_colormap_type = stbi_get8(s);  // colormap type
        if (tga_colormap_type > 1) {
            stbi_rewind(s);
            return 0;  // only RGB or indexed allowed
        }
        const int tga_image_type = stbi_get8(s);  // image type
        if (tga_colormap_type == 1) {             // colormapped (paletted) image
            if (tga_image_type != 1 && tga_image_type != 9) {
                stbi_rewind(s);
                return 0;
            }
            stbi_skip(s, 4);              // skip index of first colormap entry and number of entries
            const int sz = stbi_get8(s);  //   check bits per palette color entry
            if ((sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32)) {
                stbi_rewind(s);
                return 0;
            }
            stbi_skip(s, 4);  // skip image x and y origin
            tga_colormap_bpp = sz;
        }
        else {  // "normal" image w/o colormap - only RGB or grey allowed, +/- RLE
            if ((tga_image_type != 2) && (tga_image_type != 3) && (tga_image_type != 10) &&
                (tga_image_type != 11)) {
                stbi_rewind(s);
                return 0;  // only RGB or grey allowed, +/- RLE
            }
            stbi_skip(s, 9);  // skip colormap specification and image x/y origin
            tga_colormap_bpp = 0;
        }
        const int tga_w = stbi_get16le(s);
        if (tga_w < 1) {
            stbi_rewind(s);
            return 0;  // test width
        }
        const int tga_h = stbi_get16le(s);
        if (tga_h < 1) {
            stbi_rewind(s);
            return 0;  // test height
        }
        const int tga_bits_per_pixel = stbi_get8(s);  // bits per pixel
        stbi_get8(s);                                 // ignore alpha bits
        if (tga_colormap_bpp != 0) {
            if ((tga_bits_per_pixel != 8) && (tga_bits_per_pixel != 16)) {
                // when using a colormap, tga_bits_per_pixel is the size of the indexes
                // I don't think anything but 8 or 16bit indexes makes sense
                stbi_rewind(s);
                return 0;
            }
            tga_comp = stbi_tga_get_comp(tga_colormap_bpp, 0, nullptr);
        }
        else {
            tga_comp = stbi_tga_get_comp(tga_bits_per_pixel,
                                         (tga_image_type == 3) || (tga_image_type == 11), nullptr);
        }
        if (!tga_comp) {
            stbi_rewind(s);
            return 0;
        }
        if (x)
            *x = tga_w;
        if (y)
            *y = tga_h;
        if (comp)
            *comp = tga_comp;
        return 1;  // seems to have passed everything
    }

    static int stbi_tga_test(StbiContext* s)
    {
        int res = 0;
        stbi_get8(s);  //   discard Offset

        bool error = false;
        const int tga_color_type = stbi_get8(s);  //   color type
        if (tga_color_type > 1)
            error = true;  //   only RGB or indexed allowed

        int sz = stbi_get8(s);  //   image type
        if (!error && tga_color_type == 1) {
            // colormapped (paletted) image
            if (sz != 1 && sz != 9)
                error = true;  // colortype 1 demands image type 1 or 9

            if (!error) {
                stbi_skip(s, 4);    // skip index of first colormap entry and number of entries
                sz = stbi_get8(s);  //   check bits per palette color entry
                if (!error && (sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32))
                    error = true;

                stbi_skip(s, 4);  // skip image x and y origin
            }
        }
        else if (!error) {  // "normal" image w/o colormap
            if ((sz != 2) && (sz != 3) && (sz != 10) && (sz != 11))
                error = true;  // only RGB or grey allowed, +/- RLE
            if (!error)
                stbi_skip(s, 9);  // skip colormap specification and image x/y origin
        }

        if (!error && stbi_get16le(s) < 1)
            error = true;  //   test width
        if (!error && stbi_get16le(s) < 1)
            error = true;  //   test height

        sz = stbi_get8(s);  //   bits per pixel
        if (!error && (tga_color_type == 1) && (sz != 8) && (sz != 16))
            error = true;  // for colormapped images, bpp is size of an index
        if (!error && (sz != 8) && (sz != 15) && (sz != 16) && (sz != 24) && (sz != 32))
            error = true;

        if (!error)
            res = 1;  // if we got this far, everything's good and we can return 1 instead of 0

        stbi_rewind(s);
        return res;
    }

    // read 16bit value and convert to 24bit RGB
    static void stbi_tga_read_rgb16(StbiContext* s, stbi_uc* out)
    {
        const StbiUint16 px = static_cast<StbiUint16>(stbi_get16le(s));
        const StbiUint16 fiveBitMask = 31;
        // we have 3 channels with 5bits each
        const int r = (px >> 10) & fiveBitMask;
        const int g = (px >> 5) & fiveBitMask;
        const int b = px & fiveBitMask;
        // Note that this saves the data in RGB(A) order, so it doesn't need to be swapped later
        out[0] = static_cast<stbi_uc>((r * 255) / 31);
        out[1] = static_cast<stbi_uc>((g * 255) / 31);
        out[2] = static_cast<stbi_uc>((b * 255) / 31);

        // some people claim that the most significant bit might be used for alpha
        // (possibly if an alpha-bit is set in the "image descriptor byte")
        // but that only made 16bit test images completely translucent..
        // so let's treat all 15 and 16bit TGAs as RGB with no alpha.
    }

    static void* stbi_tga_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                               const StbiResultInfo* ri)
    {
        //   read in the TGA header stuff
        const int tga_offset = stbi_get8(s);
        const int tga_indexed = stbi_get8(s);
        int tga_image_type = stbi_get8(s);
        int tga_is_RLE = 0;
        int tga_palette_start = stbi_get16le(s);
        int tga_palette_len = stbi_get16le(s);
        int tga_palette_bits = stbi_get8(s);
        int tga_x_origin = stbi_get16le(s);
        int tga_y_origin = stbi_get16le(s);
        const int tga_width = stbi_get16le(s);
        const int tga_height = stbi_get16le(s);
        const int tga_bits_per_pixel = stbi_get8(s);
        int tga_comp, tga_rgb16 = 0;
        int tga_inverted = stbi_get8(s);
        // int tga_alpha_bits = tga_inverted & 15; // the 4 lowest bits - unused (useless?)
        //   image data
        unsigned char* tga_palette = nullptr;
        int i, j;
        unsigned char raw_data[4] = { 0 };
        int RLE_count = 0;
        int RLE_repeating = 0;
        int read_next_pixel = 1;
        STBI_NOTUSED(ri);
        STBI_NOTUSED(tga_x_origin);  // @TODO
        STBI_NOTUSED(tga_y_origin);  // @TODO

        if (tga_height > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");
        if (tga_width > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");

        //   do a tiny bit of precessing
        if (tga_image_type >= 8) {
            tga_image_type -= 8;
            tga_is_RLE = 1;
        }
        tga_inverted = 1 - ((tga_inverted >> 5) & 1);

        //   If I'm paletted, then I'll use the number of bits from the palette
        if (tga_indexed)
            tga_comp = stbi_tga_get_comp(tga_palette_bits, 0, &tga_rgb16);
        else
            tga_comp = stbi_tga_get_comp(tga_bits_per_pixel, (tga_image_type == 3), &tga_rgb16);

        if (!tga_comp)  // shouldn't really happen, stbi__tga_test() should have ensured basic
                        // consistency
            return stbi_errpuc("bad format", "Can't find out TGA pixelformat");

        //   tga info
        *x = tga_width;
        *y = tga_height;
        if (comp)
            *comp = tga_comp;

        if (!stbi_mad3sizes_valid(tga_width, tga_height, tga_comp, 0))
            return stbi_errpuc("too large", "Corrupt TGA");

        unsigned char* tga_data = static_cast<unsigned char*>(
            stbi_malloc_mad3(tga_width, tga_height, tga_comp, 0));
        if (!tga_data)
            return stbi_errpuc("outofmem", "Out of memory");

        // skip to the data's starting position (offset usually = 0)
        stbi_skip(s, tga_offset);

        if (!tga_indexed && !tga_is_RLE && !tga_rgb16) {
            for (i = 0; i < tga_height; ++i) {
                const int row = tga_inverted ? tga_height - i - 1 : i;
                stbi_uc* tga_row = tga_data + row * tga_width * tga_comp;
                stbi_getn(s, tga_row, tga_width * tga_comp);
            }
        }
        else {
            //   do I need to load a palette?
            if (tga_indexed) {
                if (tga_palette_len == 0) { /* you have to have at least one entry! */
                    STBI_FREE(tga_data);
                    return stbi_errpuc("bad palette", "Corrupt TGA");
                }

                //   any data to skip? (offset usually = 0)
                stbi_skip(s, tga_palette_start);
                //   load the palette
                tga_palette = static_cast<unsigned char*>(
                    stbi_malloc_mad2(tga_palette_len, tga_comp, 0));
                if (!tga_palette) {
                    STBI_FREE(tga_data);
                    return stbi_errpuc("outofmem", "Out of memory");
                }
                if (tga_rgb16) {
                    stbi_uc* pal_entry = tga_palette;
                    STBI_ASSERT(tga_comp == STBI_rgb);
                    for (i = 0; i < tga_palette_len; ++i) {
                        stbi_tga_read_rgb16(s, pal_entry);
                        pal_entry += tga_comp;
                    }
                }
                else if (!stbi_getn(s, tga_palette, tga_palette_len * tga_comp)) {
                    STBI_FREE(tga_data);
                    STBI_FREE(tga_palette);
                    return stbi_errpuc("bad palette", "Corrupt TGA");
                }
            }
            //   load the data
            for (i = 0; i < tga_width * tga_height; ++i) {
                //   if I'm in RLE mode, do I need to get a RLE stbi__pngchunk?
                if (tga_is_RLE) {
                    if (RLE_count == 0) {
                        //   yep, get the next byte as a RLE command
                        const int RLE_cmd = stbi_get8(s);
                        RLE_count = 1 + (RLE_cmd & 127);
                        RLE_repeating = RLE_cmd >> 7;
                        read_next_pixel = 1;
                    }
                    else if (!RLE_repeating) {
                        read_next_pixel = 1;
                    }
                }
                else {
                    read_next_pixel = 1;
                }
                //   OK, if I need to read a pixel, do it now
                if (read_next_pixel) {
                    //   load however much data we did have
                    if (tga_indexed) {
                        // read in index, then perform the lookup
                        int pal_idx = (tga_bits_per_pixel == 8) ? stbi_get8(s) : stbi_get16le(s);
                        if (pal_idx >= tga_palette_len) {
                            // invalid index
                            pal_idx = 0;
                        }
                        pal_idx *= tga_comp;
                        for (j = 0; j < tga_comp; ++j)
                            raw_data[j] = tga_palette[pal_idx + j];
                    }
                    else if (tga_rgb16) {
                        STBI_ASSERT(tga_comp == STBI_rgb);
                        stbi_tga_read_rgb16(s, raw_data);
                    }
                    else {
                        //   read in the data raw
                        for (j = 0; j < tga_comp; ++j)
                            raw_data[j] = stbi_get8(s);
                    }
                    //   clear the reading flag for the next pixel
                    read_next_pixel = 0;
                }  // end of reading a pixel

                // copy data
                for (j = 0; j < tga_comp; ++j)
                    tga_data[i * tga_comp + j] = raw_data[j];

                //   in case we're in RLE mode, keep counting down
                --RLE_count;
            }
            //   do I need to invert the image?
            if (tga_inverted) {
                for (j = 0; j * 2 < tga_height; ++j) {
                    int index1 = j * tga_width * tga_comp;
                    int index2 = (tga_height - 1 - j) * tga_width * tga_comp;
                    for (i = tga_width * tga_comp; i > 0; --i) {
                        const unsigned char temp = tga_data[index1];
                        tga_data[index1] = tga_data[index2];
                        tga_data[index2] = temp;
                        ++index1;
                        ++index2;
                    }
                }
            }
            //   clear my palette, if I had one
            if (tga_palette != nullptr)
                STBI_FREE(tga_palette);
        }

        // swap RGB - if the source data was RGB16, it already is in the right order
        if (tga_comp >= 3 && !tga_rgb16) {
            unsigned char* tga_pixel = tga_data;
            for (i = 0; i < tga_width * tga_height; ++i) {
                const unsigned char temp = tga_pixel[0];
                tga_pixel[0] = tga_pixel[2];
                tga_pixel[2] = temp;
                tga_pixel += tga_comp;
            }
        }

        // convert to target component count
        if (req_comp && req_comp != tga_comp)
            tga_data = stbi_convert_format(tga_data, tga_comp, req_comp, tga_width, tga_height);

        //   the things I do to get rid of an error message, and yet keep
        //   Microsoft's C compilers happy... [8^(
        tga_palette_start = tga_palette_len = tga_palette_bits = tga_x_origin = tga_y_origin = 0;
        STBI_NOTUSED(tga_palette_start);
        //   OK, done
        return tga_data;
    }
#endif

    // *************************************************************************************************
    // Photoshop PSD loader -- PD by Thatcher Ulrich, integration by Nicolas Schulz, tweaked by STB

#ifndef STBI_NO_PSD
    static int stbi_psd_test(StbiContext* s)
    {
        const int r = (stbi_get32be(s) == 0x38425053);
        stbi_rewind(s);
        return r;
    }

    static int stbi_psd_decode_rle(StbiContext* s, stbi_uc* p, const int pixelCount)
    {
        int nleft;

        int count = 0;
        while ((nleft = pixelCount - count) > 0) {
            int len = stbi_get8(s);
            if (len == 128) {
                // No-op.
            }
            else if (len < 128) {
                // Copy next len+1 bytes literally.
                len++;
                if (len > nleft)
                    return 0;  // corrupt data
                count += len;
                while (len) {
                    *p = stbi_get8(s);
                    p += 4;
                    len--;
                }
            }
            else if (len > 128) {
                // Next -len+1 bytes in the dest are replicated from next source byte.
                // (Interpret len as a negative 8-bit int.)
                len = 257 - len;
                if (len > nleft)
                    return 0;  // corrupt data
                const stbi_uc val = stbi_get8(s);
                count += len;
                while (len) {
                    *p = val;
                    p += 4;
                    len--;
                }
            }
        }

        return 1;
    }

    static void* stbi_psd_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                               StbiResultInfo* ri, const int bpc)
    {
        int channel, i;
        stbi_uc* out;
        STBI_NOTUSED(ri);

        // Check identifier
        if (stbi_get32be(s) != 0x38425053)  // "8BPS"
            return stbi_errpuc("not PSD", "Corrupt PSD image");

        // Check file type version.
        if (stbi_get16be(s) != 1)
            return stbi_errpuc("wrong version", "Unsupported version of PSD image");

        // Skip 6 reserved bytes.
        stbi_skip(s, 6);

        // Read the number of channels (R, G, B, A, etc).
        const int channelCount = stbi_get16be(s);
        if (channelCount < 0 || channelCount > 16)
            return stbi_errpuc("wrong channel count", "Unsupported number of channels in PSD image");

        // Read the rows and columns of the image.
        const int h = stbi_get32be(s);
        const int w = stbi_get32be(s);

        if (h > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");
        if (w > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");

        // Make sure the depth is 8 bits.
        const int bitdepth = stbi_get16be(s);
        if (bitdepth != 8 && bitdepth != 16)
            return stbi_errpuc("unsupported bit depth", "PSD bit depth is not 8 or 16 bit");

        // Make sure the color mode is RGB.
        // Valid options are:
        //   0: Bitmap
        //   1: Grayscale
        //   2: Indexed color
        //   3: RGB color
        //   4: CMYK color
        //   7: Multichannel
        //   8: Duotone
        //   9: Lab color
        if (stbi_get16be(s) != 3)
            return stbi_errpuc("wrong color format", "PSD is not in RGB color format");

        // Skip the Mode Data.  (It's the palette for indexed color; other info for other modes.)
        stbi_skip(s, stbi_get32be(s));

        // Skip the image resources.  (resolution, pen tool paths, etc)
        stbi_skip(s, stbi_get32be(s));

        // Skip the reserved data.
        stbi_skip(s, stbi_get32be(s));

        // Find out if the data is compressed.
        // Known values:
        //   0: no compression
        //   1: RLE compressed
        const int compression = stbi_get16be(s);
        if (compression > 1)
            return stbi_errpuc("bad compression", "PSD has an unknown compression format");

        // Check size
        if (!stbi_mad3sizes_valid(4, w, h, 0))
            return stbi_errpuc("too large", "Corrupt PSD");

        // Create the destination image.

        if (!compression && bitdepth == 16 && bpc == 16) {
            out = static_cast<stbi_uc*>(stbi_malloc_mad3(8, w, h, 0));
            ri->bits_per_channel = 16;
        }
        else
            out = static_cast<stbi_uc*>(stbi_malloc(4 * w * h));

        if (!out)
            return stbi_errpuc("outofmem", "Out of memory");
        const int pixelCount = w * h;

        // Initialize the data to zero.
        // memset( out, 0, pixelCount * 4 );

        // Finally, the image data.
        if (compression) {
            // RLE as used by .PSD and .TIFF
            // Loop until you get the number of unpacked bytes you are expecting:
            //     Read the next source byte into n.
            //     If n is between 0 and 127 inclusive, copy the next n+1 bytes literally.
            //     Else if n is between -127 and -1 inclusive, copy the next byte -n+1 times.
            //     Else if n is 128, noop.
            // Endloop

            // The RLE-compressed data is preceded by a 2-byte data count for each row in the data,
            // which we're going to just skip.
            stbi_skip(s, h * channelCount * 2);

            // Read the RLE data by channel.
            for (channel = 0; channel < 4; channel++) {
                stbi_uc* p = out + channel;
                if (channel >= channelCount) {
                    // Fill this channel with default data.
                    for (i = 0; i < pixelCount; i++, p += 4)
                        *p = (channel == 3 ? 255 : 0);
                }
                else {
                    // Read the RLE data.
                    if (!stbi_psd_decode_rle(s, p, pixelCount)) {
                        STBI_FREE(out);
                        return stbi_errpuc("corrupt", "bad RLE data");
                    }
                }
            }
        }
        else {
            // We're at the raw image data.  It's each channel in order (Red, Green, Blue, Alpha,
            // ...) where each channel consists of an 8-bit (or 16-bit) value for each pixel in the
            // image.

            // Read the data by channel.
            for (channel = 0; channel < 4; channel++) {
                if (channel >= channelCount) {
                    // Fill this channel with default data.
                    if (bitdepth == 16 && bpc == 16) {
                        StbiUint16* q = reinterpret_cast<StbiUint16*>(out) + channel;
                        const StbiUint16 val = channel == 3 ? 65535 : 0;
                        for (i = 0; i < pixelCount; i++, q += 4)
                            *q = val;
                    }
                    else {
                        stbi_uc* p = out + channel;
                        const stbi_uc val = channel == 3 ? 255 : 0;
                        for (i = 0; i < pixelCount; i++, p += 4)
                            *p = val;
                    }
                }
                else if (ri->bits_per_channel == 16) {  // output bpc
                    StbiUint16* q = reinterpret_cast<StbiUint16*>(out) + channel;
                    for (i = 0; i < pixelCount; i++, q += 4)
                        *q = static_cast<StbiUint16>(stbi_get16be(s));
                }
                else {
                    stbi_uc* p = out + channel;
                    if (bitdepth == 16) {  // input bpc
                        for (i = 0; i < pixelCount; i++, p += 4)
                            *p = static_cast<stbi_uc>(stbi_get16be(s) >> 8);
                    }
                    else {
                        for (i = 0; i < pixelCount; i++, p += 4)
                            *p = stbi_get8(s);
                    }
                }
            }
        }

        // remove weird white matte from PSD
        if (channelCount >= 4) {
            if (ri->bits_per_channel == 16) {
                for (i = 0; i < w * h; ++i) {
                    StbiUint16* pixel = reinterpret_cast<StbiUint16*>(out) + 4 * i;
                    if (pixel[3] != 0 && pixel[3] != 65535) {
                        const float a = pixel[3] / 65535.0f;
                        const float ra = 1.0f / a;
                        const float inv_a = 65535.0f * (1 - ra);
                        pixel[0] = static_cast<StbiUint16>(pixel[0] * ra + inv_a);
                        pixel[1] = static_cast<StbiUint16>(pixel[1] * ra + inv_a);
                        pixel[2] = static_cast<StbiUint16>(pixel[2] * ra + inv_a);
                    }
                }
            }
            else {
                for (i = 0; i < w * h; ++i) {
                    unsigned char* pixel = out + 4 * i;
                    if (pixel[3] != 0 && pixel[3] != 255) {
                        const float a = pixel[3] / 255.0f;
                        const float ra = 1.0f / a;
                        const float inv_a = 255.0f * (1 - ra);
                        pixel[0] = static_cast<unsigned char>(pixel[0] * ra + inv_a);
                        pixel[1] = static_cast<unsigned char>(pixel[1] * ra + inv_a);
                        pixel[2] = static_cast<unsigned char>(pixel[2] * ra + inv_a);
                    }
                }
            }
        }

        // convert to desired output format
        if (req_comp && req_comp != 4) {
            if (ri->bits_per_channel == 16)
                out = reinterpret_cast<stbi_uc*>(
                    stbi_convert_format16(reinterpret_cast<StbiUint16*>(out), 4, req_comp, w, h));
            else
                out = stbi_convert_format(out, 4, req_comp, w, h);
            if (out == nullptr)
                return out;  // stbi__convert_format frees input on failure
        }

        if (comp)
            *comp = 4;
        *y = h;
        *x = w;

        return out;
    }
#endif

    // *************************************************************************************************
    // Softimage PIC loader
    // by Tom Seddon
    //
    // See http://softimage.wiki.softimage.com/index.php/INFO:_PIC_file_format
    // See http://ozviz.wasp.uwa.edu.au/~pbourke/dataformats/softimagepic/

#ifndef STBI_NO_PIC
    static int stbi_pic_is4(StbiContext* s, const char* str)
    {
        for (int i = 0; i < 4; ++i)
            if (stbi_get8(s) != static_cast<stbi_uc>(str[i]))
                return 0;

        return 1;
    }

    static int stbi_pic_test_core(StbiContext* s)
    {
        if (!stbi_pic_is4(s, "\x53\x80\xF6\x34"))
            return 0;

        for (int i = 0; i < 84; ++i)
            stbi_get8(s);

        if (!stbi_pic_is4(s, "PICT"))
            return 0;

        return 1;
    }

    typedef struct
    {
        stbi_uc size, type, channel;
    } stbi_pic_packet;

    static stbi_uc* stbi_readval(StbiContext* s, const int channel, stbi_uc* dest)
    {
        int mask = 0x80;

        for (int i = 0; i < 4; ++i, mask >>= 1) {
            if (channel & mask) {
                if (stbi_at_eof(s))
                    return stbi_errpuc("bad file", "PIC file too short");
                dest[i] = stbi_get8(s);
            }
        }

        return dest;
    }

    static void stbi_copyval(const int channel, stbi_uc* dest, const stbi_uc* src)
    {
        int mask = 0x80;

        for (int i = 0; i < 4; ++i, mask >>= 1)
            if (channel & mask)
                dest[i] = src[i];
    }

    static stbi_uc* stbi_pic_load_core(StbiContext* s, const int width, const int height, int* comp,
                                       stbi_uc* result)
    {
        int act_comp = 0, num_packets = 0, chained;
        stbi_pic_packet packets[10];

        // this will (should...) cater for even some bizarre stuff like having data
        // for the same channel in multiple packets.
        do {
            if (num_packets == sizeof(packets) / sizeof(packets[0]))
                return stbi_errpuc("bad format", "too many packets");

            stbi_pic_packet* packet = &packets[num_packets++];

            chained = stbi_get8(s);
            packet->size = stbi_get8(s);
            packet->type = stbi_get8(s);
            packet->channel = stbi_get8(s);

            act_comp |= packet->channel;

            if (stbi_at_eof(s))
                return stbi_errpuc("bad file", "file too short (reading packets)");
            if (packet->size != 8)
                return stbi_errpuc("bad format", "packet isn't 8bpp");
        }
        while (chained);

        *comp = (act_comp & 0x10 ? 4 : 3);  // has alpha channel?

        for (int y = 0; y < height; ++y) {
            for (int packet_idx = 0; packet_idx < num_packets; ++packet_idx) {
                const stbi_pic_packet* packet = &packets[packet_idx];
                stbi_uc* dest = result + y * width * 4;

                switch (packet->type) {
                    default:
                        return stbi_errpuc("bad format", "packet has bad compression type");

                    case 0:
                    {  // uncompressed

                        for (int x = 0; x < width; ++x, dest += 4)
                            if (!stbi_readval(s, packet->channel, dest))
                                return 0;
                        break;
                    }

                    case 1:  // Pure RLE
                    {
                        int left = width;

                        while (left > 0) {
                            stbi_uc value[4];

                            stbi_uc count = stbi_get8(s);
                            if (stbi_at_eof(s))
                                return stbi_errpuc("bad file", "file too short (pure read count)");

                            if (count > left)
                                count = static_cast<stbi_uc>(left);

                            if (!stbi_readval(s, packet->channel, value))
                                return 0;

                            for (int i = 0; i < count; ++i, dest += 4)
                                stbi_copyval(packet->channel, dest, value);
                            left -= count;
                        }
                    } break;

                    case 2:
                    {  // Mixed RLE
                        int left = width;
                        while (left > 0) {
                            int count = stbi_get8(s), i;
                            if (stbi_at_eof(s))
                                return stbi_errpuc("bad file", "file too short (mixed read count)");

                            if (count >= 128) {  // Repeated
                                stbi_uc value[4];

                                if (count == 128)
                                    count = stbi_get16be(s);
                                else
                                    count -= 127;
                                if (count > left)
                                    return stbi_errpuc("bad file", "scanline overrun");

                                if (!stbi_readval(s, packet->channel, value))
                                    return 0;

                                for (i = 0; i < count; ++i, dest += 4)
                                    stbi_copyval(packet->channel, dest, value);
                            }
                            else {  // Raw
                                ++count;
                                if (count > left)
                                    return stbi_errpuc("bad file", "scanline overrun");

                                for (i = 0; i < count; ++i, dest += 4)
                                    if (!stbi_readval(s, packet->channel, dest))
                                        return 0;
                            }
                            left -= count;
                        }
                        break;
                    }
                }
            }
        }

        return result;
    }

    static void* stbi_pic_load(StbiContext* s, int* px, int* py, int* comp, int req_comp,
                               const StbiResultInfo* ri)
    {
        int internal_comp;
        STBI_NOTUSED(ri);

        if (!comp)
            comp = &internal_comp;

        for (int i = 0; i < 92; ++i)
            stbi_get8(s);

        const int x = stbi_get16be(s);
        const int y = stbi_get16be(s);

        if (y > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");
        if (x > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");

        if (stbi_at_eof(s))
            return stbi_errpuc("bad file", "file too short (pic header)");
        if (!stbi_mad3sizes_valid(x, y, 4, 0))
            return stbi_errpuc("too large", "PIC image too large to decode");

        stbi_get32be(s);  // skip `ratio'
        stbi_get16be(s);  // skip `fields'
        stbi_get16be(s);  // skip `pad'

        // intermediate buffer is RGBA
        stbi_uc* result = static_cast<stbi_uc*>(stbi_malloc_mad3(x, y, 4, 0));
        if (!result)
            return stbi_errpuc("outofmem", "Out of memory");
        memset(result, 0xff, x * y * 4);

        if (!stbi_pic_load_core(s, x, y, comp, result)) {
            STBI_FREE(result);
            result = 0;
        }
        *px = x;
        *py = y;
        if (req_comp == 0)
            req_comp = *comp;
        result = stbi_convert_format(result, 4, req_comp, x, y);

        return result;
    }

    static int stbi_pic_test(StbiContext* s)
    {
        const int r = stbi_pic_test_core(s);
        stbi_rewind(s);
        return r;
    }
#endif

    // *************************************************************************************************
    // GIF loader -- public domain by Jean-Marc Lienher -- simplified/shrunk by stb

#ifndef STBI_NO_GIF
    typedef struct
    {
        StbiInt16 prefix;
        stbi_uc first;
        stbi_uc suffix;
    } stbi_gif_lzw;

    typedef struct
    {
        int w, h;
        stbi_uc* out;         // output buffer (always 4 components)
        stbi_uc* background;  // The current "background" as far as a gif is concerned
        stbi_uc* history;
        int flags, bgindex, ratio, transparent, eflags;
        stbi_uc pal[256][4];
        stbi_uc lpal[256][4];
        stbi_gif_lzw codes[8192];
        stbi_uc* color_table;
        int parse, step;
        int lflags;
        int start_x, start_y;
        int max_x, max_y;
        int cur_x, cur_y;
        int line_size;
        int delay;
    } stbi_gif;

    static int stbi_gif_test_raw(StbiContext* s)
    {
        if (stbi_get8(s) != 'G' || stbi_get8(s) != 'I' || stbi_get8(s) != 'F' || stbi_get8(s) != '8')
            return 0;
        const int sz = stbi_get8(s);
        if (sz != '9' && sz != '7')
            return 0;
        if (stbi_get8(s) != 'a')
            return 0;
        return 1;
    }

    static int stbi_gif_test(StbiContext* s)
    {
        const int r = stbi_gif_test_raw(s);
        stbi_rewind(s);
        return r;
    }

    static void stbi_gif_parse_colortable(StbiContext* s, stbi_uc pal[256][4],
                                          const int num_entries, const int transp)
    {
        for (int i = 0; i < num_entries; ++i) {
            pal[i][2] = stbi_get8(s);
            pal[i][1] = stbi_get8(s);
            pal[i][0] = stbi_get8(s);
            pal[i][3] = transp == i ? 0 : 255;
        }
    }

    static int stbi_gif_header(StbiContext* s, stbi_gif* g, int* comp, const int is_info)
    {
        if (stbi_get8(s) != 'G' || stbi_get8(s) != 'I' || stbi_get8(s) != 'F' || stbi_get8(s) != '8')
            return stbi__err("not GIF", "Corrupt GIF");

        const stbi_uc version = stbi_get8(s);
        if (version != '7' && version != '9')
            return stbi__err("not GIF", "Corrupt GIF");
        if (stbi_get8(s) != 'a')
            return stbi__err("not GIF", "Corrupt GIF");

        stbi_g_failure_reason = "";
        g->w = stbi_get16le(s);
        g->h = stbi_get16le(s);
        g->flags = stbi_get8(s);
        g->bgindex = stbi_get8(s);
        g->ratio = stbi_get8(s);
        g->transparent = -1;

        if (g->w > STBI_MAX_DIMENSIONS)
            return stbi__err("too large", "Very large image (corrupt?)");
        if (g->h > STBI_MAX_DIMENSIONS)
            return stbi__err("too large", "Very large image (corrupt?)");

        if (comp != 0)
            *comp = 4;  // can't actually tell whether it's 3 or 4 until we parse the comments

        if (is_info)
            return 1;

        if (g->flags & 0x80)
            stbi_gif_parse_colortable(s, g->pal, 2 << (g->flags & 7), -1);

        return 1;
    }

    static int stbi_gif_info_raw(StbiContext* s, int* x, int* y, int* comp)
    {
        stbi_gif* g = static_cast<stbi_gif*>(stbi_malloc(sizeof(stbi_gif)));
        if (!g)
            return stbi__err("outofmem", "Out of memory");
        if (!stbi_gif_header(s, g, comp, 1)) {
            STBI_FREE(g);
            stbi_rewind(s);
            return 0;
        }
        if (x)
            *x = g->w;
        if (y)
            *y = g->h;
        STBI_FREE(g);
        return 1;
    }

    static void stbi_out_gif_code(stbi_gif* g, const StbiUint16 code)
    {
        // recurse to decode the prefixes, since the linked-list is backwards,
        // and working backwards through an interleaved image would be nasty
        if (g->codes[code].prefix >= 0)
            stbi_out_gif_code(g, g->codes[code].prefix);

        if (g->cur_y >= g->max_y)
            return;

        const int idx = g->cur_x + g->cur_y;
        stbi_uc* p = &g->out[idx];
        g->history[idx / 4] = 1;

        const stbi_uc* c = &g->color_table[g->codes[code].suffix * 4];
        if (c[3] > 128) {  // don't render transparent pixels;
            p[0] = c[2];
            p[1] = c[1];
            p[2] = c[0];
            p[3] = c[3];
        }
        g->cur_x += 4;

        if (g->cur_x >= g->max_x) {
            g->cur_x = g->start_x;
            g->cur_y += g->step;

            while (g->cur_y >= g->max_y && g->parse > 0) {
                g->step = (1 << g->parse) * g->line_size;
                g->cur_y = g->start_y + (g->step >> 1);
                --g->parse;
            }
        }
    }

    static stbi_uc* stbi_process_gif_raster(StbiContext* s, stbi_gif* g)
    {
        const stbi_uc lzw_cs = stbi_get8(s);
        if (lzw_cs > 12)
            return nullptr;
        const StbiInt32 clear = 1 << lzw_cs;
        StbiUint32 first = 1;
        StbiInt32 codesize = lzw_cs + 1;
        StbiInt32 codemask = (1 << codesize) - 1;
        StbiInt32 bits = 0;
        StbiInt32 valid_bits = 0;
        for (StbiInt32 init_code = 0; init_code < clear; init_code++) {
            g->codes[init_code].prefix = -1;
            g->codes[init_code].first = static_cast<stbi_uc>(init_code);
            g->codes[init_code].suffix = static_cast<stbi_uc>(init_code);
        }

        // support no starting clear code
        StbiInt32 avail = clear + 2;
        StbiInt32 oldcode = -1;

        StbiInt32 len = 0;
        for (;;) {
            if (valid_bits < codesize) {
                if (len == 0) {
                    len = stbi_get8(s);  // start new block
                    if (len == 0)
                        return g->out;
                }
                --len;
                bits |= static_cast<StbiInt32>(stbi_get8(s)) << valid_bits;
                valid_bits += 8;
            }
            else {
                const StbiInt32 code = bits & codemask;
                bits >>= codesize;
                valid_bits -= codesize;
                // @OPTIMIZE: is there some way we can accelerate the non-clear path?
                if (code == clear) {  // clear code
                    codesize = lzw_cs + 1;
                    codemask = (1 << codesize) - 1;
                    avail = clear + 2;
                    oldcode = -1;
                    first = 0;
                }
                else if (code == clear + 1) {  // end of stream code
                    stbi_skip(s, len);
                    while ((len = stbi_get8(s)) > 0)
                        stbi_skip(s, len);
                    return g->out;
                }
                else if (code <= avail) {
                    if (first)
                        return stbi_errpuc("no clear code", "Corrupt GIF");

                    if (oldcode >= 0) {
                        stbi_gif_lzw* p = &g->codes[avail++];
                        if (avail > 8192)
                            return stbi_errpuc("too many codes", "Corrupt GIF");

                        p->prefix = static_cast<StbiInt16>(oldcode);
                        p->first = g->codes[oldcode].first;
                        p->suffix = (code == avail) ? p->first : g->codes[code].first;
                    }
                    else if (code == avail)
                        return stbi_errpuc("illegal code in raster", "Corrupt GIF");

                    stbi_out_gif_code(g, static_cast<StbiUint16>(code));

                    if ((avail & codemask) == 0 && avail <= 0x0FFF) {
                        codesize++;
                        codemask = (1 << codesize) - 1;
                    }

                    oldcode = code;
                }
                else {
                    return stbi_errpuc("illegal code in raster", "Corrupt GIF");
                }
            }
        }
    }

    // this function is designed to support animated gifs, although stb_image doesn't support it
    // two back is the image from two frames ago, used for a very specific disposal format
    static stbi_uc* stbi_gif_load_next(StbiContext* s, stbi_gif* g, int* comp, const int req_comp,
                                       const stbi_uc* two_back)
    {
        int pi;
        int pcount;
        STBI_NOTUSED(req_comp);

        // on first frame, any non-written pixels get the background colour (non-transparent)
        int first_frame = 0;
        if (g->out == 0) {
            if (!stbi_gif_header(s, g, comp, 0))
                return 0;  // stbi__g_failure_reason set by stbi__gif_header
            if (!stbi_mad3sizes_valid(4, g->w, g->h, 0))
                return stbi_errpuc("too large", "GIF image is too large");
            pcount = g->w * g->h;
            g->out = static_cast<stbi_uc*>(stbi_malloc(4 * pcount));
            g->background = static_cast<stbi_uc*>(stbi_malloc(4 * pcount));
            g->history = static_cast<stbi_uc*>(stbi_malloc(pcount));
            if (!g->out || !g->background || !g->history)
                return stbi_errpuc("outofmem", "Out of memory");

            // image is treated as "transparent" at the start - ie, nothing overwrites the current
            // background; background colour is only used for pixels that are not rendered first
            // frame, after that "background" color refers to the color that was there the previous
            // frame.
            memset(g->out, 0x00, 4 * pcount);
            memset(g->background, 0x00, 4 * pcount);  // state of the background (starts
                                                      // transparent)
            memset(g->history, 0x00, pcount);         // pixels that were affected previous frame
            first_frame = 1;
        }
        else {
            // second frame - how do we dispose of the previous one?
            int dispose = (g->eflags & 0x1C) >> 2;
            pcount = g->w * g->h;

            if ((dispose == 3) && (two_back == 0)) {
                dispose = 2;  // if I don't have an image to revert back to, default to the old
                              // background
            }

            if (dispose == 3) {  // use previous graphic
                for (pi = 0; pi < pcount; ++pi)
                    if (g->history[pi])
                        memcpy(&g->out[pi * 4], &two_back[pi * 4], 4);
            }
            else if (dispose == 2) {
                // restore what was changed last frame to background before that frame;
                for (pi = 0; pi < pcount; ++pi)
                    if (g->history[pi])
                        memcpy(&g->out[pi * 4], &g->background[pi * 4], 4);
            }
            else {
                // This is a non-disposal case eithe way, so just
                // leave the pixels as is, and they will become the new background
                // 1: do not dispose
                // 0:  not specified.
            }

            // background is what out is after the undoing of the previou frame;
            memcpy(g->background, g->out, 4 * g->w * g->h);
        }

        // clear my history;
        memset(g->history, 0x00, g->w * g->h);  // pixels that were affected previous frame

        for (;;) {
            const int tag = stbi_get8(s);
            switch (tag) {
                case 0x2C: /* Image Descriptor */
                {
                    const StbiInt32 x = stbi_get16le(s);
                    const StbiInt32 y = stbi_get16le(s);
                    const StbiInt32 w = stbi_get16le(s);
                    const StbiInt32 h = stbi_get16le(s);
                    if (((x + w) > (g->w)) || ((y + h) > (g->h)))
                        return stbi_errpuc("bad Image Descriptor", "Corrupt GIF");

                    g->line_size = g->w * 4;
                    g->start_x = x * 4;
                    g->start_y = y * g->line_size;
                    g->max_x = g->start_x + w * 4;
                    g->max_y = g->start_y + h * g->line_size;
                    g->cur_x = g->start_x;
                    g->cur_y = g->start_y;

                    // if the width of the specified rectangle is 0, that means
                    // we may not see *any* pixels or the image is malformed;
                    // to make sure this is caught, move the current y down to
                    // max_y (which is what out_gif_code checks).
                    if (w == 0)
                        g->cur_y = g->max_y;

                    g->lflags = stbi_get8(s);

                    if (g->lflags & 0x40) {
                        g->step = 8 * g->line_size;  // first interlaced spacing
                        g->parse = 3;
                    }
                    else {
                        g->step = g->line_size;
                        g->parse = 0;
                    }

                    if (g->lflags & 0x80) {
                        stbi_gif_parse_colortable(s, g->lpal, 2 << (g->lflags & 7),
                                                  g->eflags & 0x01 ? g->transparent : -1);
                        g->color_table = (stbi_uc*)g->lpal;
                    }
                    else if (g->flags & 0x80) {
                        g->color_table = (stbi_uc*)g->pal;
                    }
                    else
                        return stbi_errpuc("missing color table", "Corrupt GIF");

                    stbi_uc* o = stbi_process_gif_raster(s, g);
                    if (!o)
                        return nullptr;

                    // if this was the first frame,
                    pcount = g->w * g->h;
                    if (first_frame && (g->bgindex > 0)) {
                        // if first frame, any pixel not drawn to gets the background color
                        for (pi = 0; pi < pcount; ++pi) {
                            if (g->history[pi] == 0) {
                                g->pal[g->bgindex][3] = 255;  // just in case it was made
                                                              // transparent, undo that; It will be
                                                              // reset next frame if need be;
                                memcpy(&g->out[pi * 4], &g->pal[g->bgindex], 4);
                            }
                        }
                    }

                    return o;
                }

                case 0x21:  // Comment Extension.
                {
                    int len;
                    const int ext = stbi_get8(s);
                    if (ext == 0xF9) {  // Graphic Control Extension.
                        len = stbi_get8(s);
                        if (len == 4) {
                            g->eflags = stbi_get8(s);
                            g->delay = 10 * stbi_get16le(s);  // delay - 1/100th of a second,
                                                              // saving as 1/1000ths.

                            // unset old transparent
                            if (g->transparent >= 0)
                                g->pal[g->transparent][3] = 255;
                            if (g->eflags & 0x01) {
                                g->transparent = stbi_get8(s);
                                if (g->transparent >= 0)
                                    g->pal[g->transparent][3] = 0;
                            }
                            else {
                                // don't need transparent
                                stbi_skip(s, 1);
                                g->transparent = -1;
                            }
                        }
                        else {
                            stbi_skip(s, len);
                            break;
                        }
                    }
                    while ((len = stbi_get8(s)) != 0)
                        stbi_skip(s, len);
                    break;
                }

                case 0x3B:               // gif stream termination code
                    return (stbi_uc*)s;  // using '1' causes warning on some compilers

                default:
                    return stbi_errpuc("unknown code", "Corrupt GIF");
            }
        }
    }

    static void* stbi_load_gif_main_outofmem(const stbi_gif* g, stbi_uc* out, int** delays)
    {
        STBI_FREE(g->out);
        STBI_FREE(g->history);
        STBI_FREE(g->background);

        if (out)
            STBI_FREE(out);
        if (delays && *delays)
            STBI_FREE(*delays);
        return stbi_errpuc("outofmem", "Out of memory");
    }

    static void* stbi_load_gif_main(StbiContext* s, int** delays, int* x, int* y, int* z, int* comp,
                                    const int req_comp)
    {
        if (stbi_gif_test(s)) {
            int layers = 0;
            const stbi_uc* u = 0;
            stbi_uc* out = 0;
            const stbi_uc* two_back = 0;
            stbi_gif g;
            int out_size = 0;
            int delays_size = 0;

            STBI_NOTUSED(out_size);
            STBI_NOTUSED(delays_size);

            memset(&g, 0, sizeof(g));
            if (delays)
                *delays = 0;

            do {
                u = stbi_gif_load_next(s, &g, comp, req_comp, two_back);
                if (u == (stbi_uc*)s)
                    u = 0;  // end of animated gif marker

                if (u) {
                    *x = g.w;
                    *y = g.h;
                    ++layers;
                    const int stride = g.w * g.h * 4;

                    if (out) {
                        void* tmp = static_cast<stbi_uc*>(
                            STBI_REALLOC_SIZED(out, out_size, layers * stride));
                        if (!tmp)
                            return stbi_load_gif_main_outofmem(&g, out, delays);
                        else {
                            out = static_cast<stbi_uc*>(tmp);
                            out_size = layers * stride;
                        }

                        if (delays) {
                            int* new_delays = static_cast<int*>(
                                STBI_REALLOC_SIZED(*delays, delays_size, sizeof(int) * layers));
                            if (!new_delays)
                                return stbi_load_gif_main_outofmem(&g, out, delays);
                            *delays = new_delays;
                            delays_size = layers * sizeof(int);
                        }
                    }
                    else {
                        out = static_cast<stbi_uc*>(stbi_malloc(layers * stride));
                        if (!out)
                            return stbi_load_gif_main_outofmem(&g, out, delays);
                        out_size = layers * stride;
                        if (delays) {
                            *delays = static_cast<int*>(stbi_malloc(layers * sizeof(int)));
                            if (!*delays)
                                return stbi_load_gif_main_outofmem(&g, out, delays);
                            delays_size = layers * sizeof(int);
                        }
                    }
                    memcpy(out + ((layers - 1) * stride), u, stride);
                    if (layers >= 2)
                        two_back = out - 2 * stride;

                    if (delays)
                        (*delays)[layers - 1U] = g.delay;
                }
            }
            while (u != 0);

            // free temp buffer;
            STBI_FREE(g.out);
            STBI_FREE(g.history);
            STBI_FREE(g.background);

            // do the final conversion after loading everything;
            if (req_comp && req_comp != 4)
                out = stbi_convert_format(out, 4, req_comp, layers * g.w, g.h);

            *z = layers;
            return out;
        }
        else {
            return stbi_errpuc("not GIF", "Image was not as a gif type.");
        }
    }

    static void* stbi_gif_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                               const StbiResultInfo* ri)
    {
        stbi_uc* u = 0;
        stbi_gif g;
        memset(&g, 0, sizeof(g));
        STBI_NOTUSED(ri);

        u = stbi_gif_load_next(s, &g, comp, req_comp, 0);
        if (u == (stbi_uc*)s)
            u = 0;  // end of animated gif marker
        if (u) {
            *x = g.w;
            *y = g.h;

            // moved conversion to after successful load so that the same
            // can be done for multiple frames.
            if (req_comp && req_comp != 4)
                u = stbi_convert_format(u, 4, req_comp, g.w, g.h);
        }
        else if (g.out) {
            // if there was an error and we allocated an image buffer, free it!
            STBI_FREE(g.out);
        }

        // free buffers needed for multiple frame loading;
        STBI_FREE(g.history);
        STBI_FREE(g.background);

        return u;
    }

    static int stbi_gif_info(StbiContext* s, int* x, int* y, int* comp)
    {
        return stbi_gif_info_raw(s, x, y, comp);
    }
#endif

// *************************************************************************************************
// Radiance RGBE HDR loader
// originally by Nicolas Schulz
#ifndef STBI_NO_HDR
    static int stbi_hdr_test_core(StbiContext* s, const char* signature)
    {
        for (int i = 0; signature[i]; ++i)
            if (stbi_get8(s) != signature[i])
                return 0;
        stbi_rewind(s);
        return 1;
    }

    static int stbi_hdr_test(StbiContext* s)
    {
        int r = stbi_hdr_test_core(s, "#?RADIANCE\n");
        stbi_rewind(s);
        if (!r) {
            r = stbi_hdr_test_core(s, "#?RGBE\n");
            stbi_rewind(s);
        }
        return r;
    }

  #define STBI_HDR_BUFLEN 1024

    static char* stbi_hdr_gettoken(StbiContext* z, char* buffer)
    {
        int len = 0;
        char c = '\0';

        c = static_cast<char>(stbi_get8(z));

        while (!stbi_at_eof(z) && c != '\n') {
            buffer[len++] = c;
            if (len == STBI_HDR_BUFLEN - 1) {
                // flush to end of line
                while (!stbi_at_eof(z) && stbi_get8(z) != '\n')
                    ;
                break;
            }
            c = static_cast<char>(stbi_get8(z));
        }

        buffer[len] = 0;
        return buffer;
    }

    static void stbi_hdr_convert(float* output, const stbi_uc* input, const int req_comp)
    {
        if (input[3] != 0) {
            // Exponent
            const float f1 = static_cast<float>(ldexp(1.0f, input[3] - (int)(128 + 8)));
            if (req_comp <= 2)
                output[0] = (input[0] + input[1] + input[2]) * f1 / 3;
            else {
                output[0] = static_cast<f32>(input[0] * f1);
                output[1] = static_cast<f32>(input[1] * f1);
                output[2] = static_cast<f32>(input[2] * f1);
            }
            if (req_comp == 2)
                output[1] = 1;
            if (req_comp == 4)
                output[3] = 1;
        }
        else {
            switch (req_comp) {
                case 4:
                    output[3] = 1;
                    [[fallthrough]];
                case 3:
                    output[0] = output[1] = output[2] = 0;
                    break;
                case 2:
                    output[1] = 1;
                    [[fallthrough]];
                case 1:
                    output[0] = 0;
                    break;
            }
        }
    }

    static float* stbi_hdr_load(StbiContext* s, int* x, int* y, int* comp, int req_comp,
                                const StbiResultInfo* ri)
    {
        char buffer[STBI_HDR_BUFLEN];
        char* token;
        int valid = 0;
        int i, j, z;
        STBI_NOTUSED(ri);

        // Check identifier
        const char* headerToken = stbi_hdr_gettoken(s, buffer);
        if (strcmp(headerToken, "#?RADIANCE") != 0 && strcmp(headerToken, "#?RGBE") != 0)
            return stbi_errpf("not HDR", "Corrupt HDR image");

        // Parse header
        for (;;) {
            token = stbi_hdr_gettoken(s, buffer);
            if (token[0] == 0)
                break;
            if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0)
                valid = 1;
        }

        if (!valid)
            return stbi_errpf("unsupported format", "Unsupported HDR format");

        // Parse width and height
        // can't use sscanf() if we're not using stdio!
        token = stbi_hdr_gettoken(s, buffer);
        if (strncmp(token, "-Y ", 3))
            return stbi_errpf("unsupported data layout", "Unsupported HDR format");
        token += 3;
        const int height = static_cast<int>(strtol(token, &token, 10));
        while (*token == ' ')
            ++token;
        if (strncmp(token, "+X ", 3))
            return stbi_errpf("unsupported data layout", "Unsupported HDR format");
        token += 3;
        const int width = static_cast<int>(strtol(token, nullptr, 10));

        if (height > STBI_MAX_DIMENSIONS)
            return stbi_errpf("too large", "Very large image (corrupt?)");
        if (width > STBI_MAX_DIMENSIONS)
            return stbi_errpf("too large", "Very large image (corrupt?)");

        *x = width;
        *y = height;

        if (comp)
            *comp = 3;
        if (req_comp == 0)
            req_comp = 3;

        if (!stbi_mad4sizes_valid(width, height, req_comp, sizeof(float), 0))
            return stbi_errpf("too large", "HDR image is too large");

        // Read data
        float* hdr_data = static_cast<float*>(
            stbi_malloc_mad4(width, height, req_comp, sizeof(float), 0));
        if (!hdr_data)
            return stbi_errpf("outofmem", "Out of memory");

        // Load image data
        // image data is stored as some number of sca
        if (width < 8 || width >= 32768) {
            // Read flat data
            for (j = 0; j < height; ++j) {
                for (i = 0; i < width; ++i) {
                    stbi_uc rgbe[4];
                main_decode_loop:
                    stbi_getn(s, rgbe, 4);
                    stbi_hdr_convert(hdr_data + j * width * req_comp + i * req_comp, rgbe, req_comp);
                }
            }
        }
        else {
            // Read RLE-encoded data
            stbi_uc* scanline = nullptr;

            for (j = 0; j < height; ++j) {
                const int c1 = stbi_get8(s);
                const int c2 = stbi_get8(s);
                int len = stbi_get8(s);
                if (c1 != 2 || c2 != 2 || (len & 0x80)) {
                    // not run-length encoded, so we have to actually use THIS data as a decoded
                    // pixel (note this can't be a valid pixel--one of RGB must be >= 128)
                    stbi_uc rgbe[4];
                    rgbe[0] = static_cast<stbi_uc>(c1);
                    rgbe[1] = static_cast<stbi_uc>(c2);
                    rgbe[2] = static_cast<stbi_uc>(len);
                    rgbe[3] = (stbi_uc)stbi_get8(s);
                    stbi_hdr_convert(hdr_data, rgbe, req_comp);
                    i = 1;
                    j = 0;
                    STBI_FREE(scanline);
                    goto main_decode_loop;  // yes, this makes no sense
                }
                len <<= 8;
                len |= stbi_get8(s);
                if (len != width) {
                    STBI_FREE(hdr_data);
                    STBI_FREE(scanline);
                    return stbi_errpf("invalid decoded scanline length", "corrupt HDR");
                }
                if (scanline == nullptr) {
                    scanline = static_cast<stbi_uc*>(stbi_malloc_mad2(width, 4, 0));
                    if (!scanline) {
                        STBI_FREE(hdr_data);
                        return stbi_errpf("outofmem", "Out of memory");
                    }
                }

                for (int k = 0; k < 4; ++k) {
                    int nleft;
                    i = 0;
                    while ((nleft = width - i) > 0) {
                        unsigned char count = stbi_get8(s);
                        if (count > 128) {
                            // Run
                            const unsigned char value = stbi_get8(s);
                            count -= 128;
                            if ((count == 0) || (count > nleft)) {
                                STBI_FREE(hdr_data);
                                STBI_FREE(scanline);
                                return stbi_errpf("corrupt", "bad RLE data in HDR");
                            }
                            for (z = 0; z < count; ++z)
                                scanline[i++ * 4 + k] = value;
                        }
                        else {
                            // Dump
                            if ((count == 0) || (count > nleft)) {
                                STBI_FREE(hdr_data);
                                STBI_FREE(scanline);
                                return stbi_errpf("corrupt", "bad RLE data in HDR");
                            }
                            for (z = 0; z < count; ++z)
                                scanline[i++ * 4 + k] = stbi_get8(s);
                        }
                    }
                }
                for (i = 0; i < width; ++i)
                    stbi_hdr_convert(hdr_data + (j * width + i) * req_comp, scanline + i * 4,
                                     req_comp);
            }
            if (scanline)
                STBI_FREE(scanline);
        }

        return hdr_data;
    }

    static int stbi_hdr_info(StbiContext* s, int* x, int* y, int* comp)
    {
        char buffer[STBI_HDR_BUFLEN];
        char* token;
        int valid = 0;
        int dummy;

        if (!x)
            x = &dummy;
        if (!y)
            y = &dummy;
        if (!comp)
            comp = &dummy;

        if (stbi_hdr_test(s) == 0) {
            stbi_rewind(s);
            return 0;
        }

        for (;;) {
            token = stbi_hdr_gettoken(s, buffer);
            if (token[0] == 0)
                break;
            if (strcmp(token, "FORMAT=32-bit_rle_rgbe") == 0)
                valid = 1;
        }

        if (!valid) {
            stbi_rewind(s);
            return 0;
        }
        token = stbi_hdr_gettoken(s, buffer);
        if (strncmp(token, "-Y ", 3)) {
            stbi_rewind(s);
            return 0;
        }
        token += 3;
        *y = static_cast<int>(strtol(token, &token, 10));
        while (*token == ' ')
            ++token;
        if (strncmp(token, "+X ", 3)) {
            stbi_rewind(s);
            return 0;
        }
        token += 3;
        *x = static_cast<int>(strtol(token, nullptr, 10));
        *comp = 3;
        return 1;
    }
#endif  // STBI_NO_HDR

#ifndef STBI_NO_BMP
    static int stbi_bmp_info(StbiContext* s, int* x, int* y, int* comp)
    {
        stbi_bmp_data info;

        info.all_a = 255;
        const void* p = stbi_bmp_parse_header(s, &info);
        if (p == nullptr) {
            stbi_rewind(s);
            return 0;
        }
        if (x)
            *x = s->img_x;
        if (y)
            *y = s->img_y;
        if (comp) {
            if (info.bpp == 24 && info.ma == 0xff000000)
                *comp = 3;
            else
                *comp = info.ma ? 4 : 3;
        }
        return 1;
    }
#endif

#ifndef STBI_NO_PSD
    static int stbi_psd_info(StbiContext* s, int* x, int* y, int* comp)
    {
        int dummy;
        if (!x)
            x = &dummy;
        if (!y)
            y = &dummy;
        if (!comp)
            comp = &dummy;
        if (stbi_get32be(s) != 0x38425053) {
            stbi_rewind(s);
            return 0;
        }
        if (stbi_get16be(s) != 1) {
            stbi_rewind(s);
            return 0;
        }
        stbi_skip(s, 6);
        const int channelCount = stbi_get16be(s);
        if (channelCount < 0 || channelCount > 16) {
            stbi_rewind(s);
            return 0;
        }
        *y = stbi_get32be(s);
        *x = stbi_get32be(s);
        const int depth = stbi_get16be(s);
        if (depth != 8 && depth != 16) {
            stbi_rewind(s);
            return 0;
        }
        if (stbi_get16be(s) != 3) {
            stbi_rewind(s);
            return 0;
        }
        *comp = 4;
        return 1;
    }

    static int stbi_psd_is16(StbiContext* s)
    {
        if (stbi_get32be(s) != 0x38425053) {
            stbi_rewind(s);
            return 0;
        }
        if (stbi_get16be(s) != 1) {
            stbi_rewind(s);
            return 0;
        }
        stbi_skip(s, 6);
        const int channelCount = stbi_get16be(s);
        if (channelCount < 0 || channelCount > 16) {
            stbi_rewind(s);
            return 0;
        }
        STBI_NOTUSED(stbi_get32be(s));
        STBI_NOTUSED(stbi_get32be(s));
        const int depth = stbi_get16be(s);
        if (depth != 16) {
            stbi_rewind(s);
            return 0;
        }
        return 1;
    }
#endif

#ifndef STBI_NO_PIC
    static int stbi_pic_info(StbiContext* s, int* x, int* y, int* comp)
    {
        int act_comp = 0, num_packets = 0, chained, dummy;
        stbi_pic_packet packets[10];

        if (!x)
            x = &dummy;
        if (!y)
            y = &dummy;
        if (!comp)
            comp = &dummy;

        if (!stbi_pic_is4(s, "\x53\x80\xF6\x34")) {
            stbi_rewind(s);
            return 0;
        }

        stbi_skip(s, 88);

        *x = stbi_get16be(s);
        *y = stbi_get16be(s);
        if (stbi_at_eof(s)) {
            stbi_rewind(s);
            return 0;
        }
        if ((*x) != 0 && (1 << 28) / (*x) < (*y)) {
            stbi_rewind(s);
            return 0;
        }

        stbi_skip(s, 8);

        do {
            if (num_packets == sizeof(packets) / sizeof(packets[0]))
                return 0;

            stbi_pic_packet* packet = &packets[num_packets++];
            chained = stbi_get8(s);
            packet->size = stbi_get8(s);
            packet->type = stbi_get8(s);
            packet->channel = stbi_get8(s);
            act_comp |= packet->channel;

            if (stbi_at_eof(s)) {
                stbi_rewind(s);
                return 0;
            }
            if (packet->size != 8) {
                stbi_rewind(s);
                return 0;
            }
        }
        while (chained);

        *comp = (act_comp & 0x10 ? 4 : 3);

        return 1;
    }
#endif

    // *************************************************************************************************
    // Portable Gray Map and Portable Pixel Map loader
    // by Ken Miller
    //
    // PGM: http://netpbm.sourceforge.net/doc/pgm.html
    // PPM: http://netpbm.sourceforge.net/doc/ppm.html
    //
    // Known limitations:
    //    Does not support comments in the header section
    //    Does not support ASCII image data (formats P2 and P3)

#ifndef STBI_NO_PNM

    static int stbi_pnm_test(StbiContext* s)
    {
        const char p = static_cast<char>(stbi_get8(s));
        const char t = static_cast<char>(stbi_get8(s));
        if (p != 'P' || (t != '5' && t != '6')) {
            stbi_rewind(s);
            return 0;
        }
        return 1;
    }

    static void* stbi_pnm_load(StbiContext* s, int* x, int* y, int* comp, const int req_comp,
                               StbiResultInfo* ri)
    {
        STBI_NOTUSED(ri);

        ri->bits_per_channel = stbi_pnm_info(s, (int*)&s->img_x, (int*)&s->img_y, (int*)&s->img_n);
        if (ri->bits_per_channel == 0)
            return 0;

        if (s->img_y > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");
        if (s->img_x > STBI_MAX_DIMENSIONS)
            return stbi_errpuc("too large", "Very large image (corrupt?)");

        *x = s->img_x;
        *y = s->img_y;
        if (comp)
            *comp = s->img_n;

        if (!stbi_mad4sizes_valid(s->img_n, s->img_x, s->img_y, ri->bits_per_channel / 8, 0))
            return stbi_errpuc("too large", "PNM too large");

        stbi_uc* out = static_cast<stbi_uc*>(
            stbi_malloc_mad4(s->img_n, s->img_x, s->img_y, ri->bits_per_channel / 8, 0));
        if (!out)
            return stbi_errpuc("outofmem", "Out of memory");
        if (!stbi_getn(s, out, s->img_n * s->img_x * s->img_y * (ri->bits_per_channel / 8))) {
            STBI_FREE(out);
            return stbi_errpuc("bad PNM", "PNM file truncated");
        }

        if (req_comp && req_comp != s->img_n) {
            if (ri->bits_per_channel == 16) {
                out = reinterpret_cast<stbi_uc*>(stbi_convert_format16(
                    reinterpret_cast<StbiUint16*>(out), s->img_n, req_comp, s->img_x, s->img_y));
            }
            else {
                out = stbi_convert_format(out, s->img_n, req_comp, s->img_x, s->img_y);
            }
            if (out == nullptr)
                return out;  // stbi__convert_format frees input on failure
        }
        return out;
    }

    static int stbi_pnm_isspace(const char c)
    {
        return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
    }

    static void stbi_pnm_skip_whitespace(StbiContext* s, char* c)
    {
        for (;;) {
            while (!stbi_at_eof(s) && stbi_pnm_isspace(*c))
                *c = static_cast<char>(stbi_get8(s));

            if (stbi_at_eof(s) || *c != '#')
                break;

            while (!stbi_at_eof(s) && *c != '\n' && *c != '\r')
                *c = static_cast<char>(stbi_get8(s));
        }
    }

    static int stbi_pnm_isdigit(const char c)
    {
        return c >= '0' && c <= '9';
    }

    static int stbi_pnm_getinteger(StbiContext* s, char* c)
    {
        int value = 0;

        while (!stbi_at_eof(s) && stbi_pnm_isdigit(*c)) {
            value = value * 10 + (*c - '0');
            *c = static_cast<char>(stbi_get8(s));
            if ((value > 214748364) || (value == 214748364 && *c > '7'))
                return stbi__err("integer parse overflow",
                                 "Parsing an integer in the PPM header overflowed a 32-bit int");
        }

        return value;
    }

    static int stbi_pnm_info(StbiContext* s, int* x, int* y, int* comp)
    {
        int dummy;
        char c;

        if (!x)
            x = &dummy;
        if (!y)
            y = &dummy;
        if (!comp)
            comp = &dummy;

        stbi_rewind(s);

        // Get identifier
        const char p = static_cast<char>(stbi_get8(s));
        const char t = static_cast<char>(stbi_get8(s));
        if (p != 'P' || (t != '5' && t != '6')) {
            stbi_rewind(s);
            return 0;
        }

        *comp = (t == '6') ? 3 : 1;  // '5' is 1-component .pgm; '6' is 3-component .ppm

        c = static_cast<char>(stbi_get8(s));
        stbi_pnm_skip_whitespace(s, &c);

        *x = stbi_pnm_getinteger(s, &c);  // read width
        if (*x == 0)
            return stbi__err("invalid width", "PPM image header had zero or overflowing width");
        stbi_pnm_skip_whitespace(s, &c);

        *y = stbi_pnm_getinteger(s, &c);  // read height
        if (*y == 0)
            return stbi__err("invalid width", "PPM image header had zero or overflowing width");
        stbi_pnm_skip_whitespace(s, &c);

        const int maxv = stbi_pnm_getinteger(s, &c);  // read max value
        if (maxv > 65535)
            return stbi__err("max value > 65535", "PPM image supports only 8-bit and 16-bit images");
        else if (maxv > 255)
            return 16;
        else
            return 8;
    }

    static int stbi_pnm_is16(StbiContext* s)
    {
        if (stbi_pnm_info(s, nullptr, nullptr, nullptr) == 16)
            return 1;
        return 0;
    }
#endif

    static int stbi_info_main(StbiContext* s, int* x, int* y, int* comp)
    {
#ifndef STBI_NO_JPEG
        if (stbi_jpeg_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_PNG
        if (stbi_png_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_GIF
        if (stbi_gif_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_BMP
        if (stbi_bmp_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_PSD
        if (stbi_psd_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_PIC
        if (stbi_pic_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_PNM
        if (stbi_pnm_info(s, x, y, comp))
            return 1;
#endif

#ifndef STBI_NO_HDR
        if (stbi_hdr_info(s, x, y, comp))
            return 1;
#endif

// test tga last because it's a crappy test!
#ifndef STBI_NO_TGA
        if (stbi_tga_info(s, x, y, comp))
            return 1;
#endif
        return stbi__err("unknown image type", "Image not of any known type, or corrupt");
    }

    static int stbi_is_16_main(StbiContext* s)
    {
#ifndef STBI_NO_PNG
        if (stbi_png_is16(s))
            return 1;
#endif

#ifndef STBI_NO_PSD
        if (stbi_psd_is16(s))
            return 1;
#endif

#ifndef STBI_NO_PNM
        if (stbi_pnm_is16(s))
            return 1;
#endif
        return 0;
    }

#ifndef STBI_NO_STDIO
    int stbi_info(const char* filename, int* x, int* y, int* comp)
    {
        FILE* f = stbi_fopen(filename, "rb");
        if (!f)
            return stbi__err("can't fopen", "Unable to open file");
        const int result = stbi_info_from_file(f, x, y, comp);
        fclose(f);
        return result;
    }

    int stbi_info_from_file(FILE* f, int* x, int* y, int* comp)
    {
        StbiContext s;
        const long pos = ftell(f);
        stbi_start_file(&s, f);
        const int r = stbi_info_main(&s, x, y, comp);
        fseek(f, pos, SEEK_SET);
        return r;
    }

    int stbi_is_16_bit(const char* filename)
    {
        FILE* f = stbi_fopen(filename, "rb");
        if (!f)
            return stbi__err("can't fopen", "Unable to open file");
        const int result = stbi_is_16_bit_from_file(f);
        fclose(f);
        return result;
    }

    int stbi_is_16_bit_from_file(FILE* f)
    {
        StbiContext s;
        const long pos = ftell(f);
        stbi_start_file(&s, f);
        const int r = stbi_is_16_main(&s);
        fseek(f, pos, SEEK_SET);
        return r;
    }
#endif  // !STBI_NO_STDIO

    int stbi_info_from_memory(const stbi_uc* buffer, const int len, int* x, int* y, int* comp)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_info_main(&s, x, y, comp);
    }

    int stbi_info_from_callbacks(const stbi_io_callbacks* c, void* user, int* x, int* y, int* comp)
    {
        StbiContext s;
        stbi_start_callbacks(&s, const_cast<stbi_io_callbacks*>(c), user);
        return stbi_info_main(&s, x, y, comp);
    }

    int stbi_is_16_bit_from_memory(const stbi_uc* buffer, const int len)
    {
        StbiContext s;
        stbi_start_mem(&s, buffer, len);
        return stbi_is_16_main(&s);
    }

    int stbi_is_16_bit_from_callbacks(const stbi_io_callbacks* c, void* user)
    {
        StbiContext s;
        stbi_start_callbacks(&s, const_cast<stbi_io_callbacks*>(c), user);
        return stbi_is_16_main(&s);
    }
}