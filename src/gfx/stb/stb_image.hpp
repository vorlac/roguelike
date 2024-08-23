#pragma once

#include <cstdio>

namespace rl::stb {

    enum {
        STBI_default = 0,  // only used for desired_channels

        STBI_grey = 1,
        STBI_grey_alpha = 2,
        STBI_rgb = 3,
        STBI_rgb_alpha = 4
    };

#include <cstdlib>
    using stbi_uc = unsigned char;
    using stbi_us = unsigned short;

    //////////////////////////////////////////////////////////////////////////////
    //
    // PRIMARY API - works on images of any type
    //

    //
    // load image by filename, open file, or memory buffer
    //

    struct stbi_io_callbacks {
        // fill 'data' with 'size' bytes. return number of bytes actually read
        int (*read)(void* user, char* data, int size);
        // skip the next 'n' bytes, or 'unget' the last -n bytes if negative
        void (*skip)(void* user, int n);
        // returns nonzero if we are at end of file/data
        int (*eof)(void* user);
    };

    ////////////////////////////////////
    //
    // 8-bits-per-channel interface
    //

    stbi_uc* stbi_load_from_memory(const stbi_uc* buffer, int len, int* x, int* y,
                                   int* channels_in_file, int desired_channels);
    stbi_uc* stbi_load_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                      int* channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
    stbi_uc* stbi_load(const char* filename, int* x, int* y, int* channels_in_file,
                       int desired_channels);
    stbi_uc* stbi_load_from_file(FILE* f, int* x, int* y, int* channels_in_file,
                                 int desired_channels);
// for stbi_load_from_file, file pointer is left pointing immediately after image
#endif

#ifndef STBI_NO_GIF
    stbi_uc* stbi_load_gif_from_memory(const stbi_uc* buffer, int len, int** delays, int* x, int* y,
                                       int* z, int* comp, int req_comp);
#endif

#ifdef STBI_WINDOWS_UTF8
    int stbi_convert_wchar_to_utf8(char* buffer, size_t bufferlen, const wchar_t* input);
#endif

    ////////////////////////////////////
    //
    // 16-bits-per-channel interface
    //

    stbi_us* stbi_load_16_from_memory(const stbi_uc* buffer, int len, int* x, int* y,
                                      int* channels_in_file, int desired_channels);
    stbi_us* stbi_load_16_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                         int* channels_in_file, int desired_channels);

#ifndef STBI_NO_STDIO
    stbi_us* stbi_load_16(const char* filename, int* x, int* y, int* channels_in_file,
                          int desired_channels);
    stbi_us* stbi_load_from_file_16(FILE* f, int* x, int* y, int* channels_in_file,
                                    int desired_channels);
#endif

////////////////////////////////////
//
// float-per-channel interface
//
#ifndef STBI_NO_LINEAR
    float* stbi_loadf_from_memory(const stbi_uc* buffer, int len, int* x, int* y,
                                  int* channels_in_file, int desired_channels);
    float* stbi_loadf_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                     int* channels_in_file, int desired_channels);

  #ifndef STBI_NO_STDIO
    float* stbi_loadf(const char* filename, int* x, int* y, int* channels_in_file,
                      int desired_channels);
    float* stbi_loadf_from_file(FILE* f, int* x, int* y, int* channels_in_file,
                                int desired_channels);
  #endif
#endif

#ifndef STBI_NO_HDR
    void stbi_hdr_to_ldr_gamma(float gamma);
    void stbi_hdr_to_ldr_scale(float scale);
#endif  // STBI_NO_HDR

#ifndef STBI_NO_LINEAR
    void stbi_ldr_to_hdr_gamma(float gamma);
    void stbi_ldr_to_hdr_scale(float scale);
#endif  // STBI_NO_LINEAR

    // stbi_is_hdr is always defined, but always returns false if STBI_NO_HDR
    int stbi_is_hdr_from_callbacks(const stbi_io_callbacks* clbk, void* user);
    int stbi_is_hdr_from_memory(const stbi_uc* buffer, int len);
#ifndef STBI_NO_STDIO
    int stbi_is_hdr(const char* filename);
    int stbi_is_hdr_from_file(FILE* f);
#endif  // STBI_NO_STDIO

    // get a VERY brief reason for failure
    // on most compilers (and ALL modern mainstream compilers) this is threadsafe
    const char* stbi_failure_reason(void);

    // free the loaded image -- this is just free()
    void stbi_image_free(void* retval_from_stbi_load);

    // get image dimensions & components without fully decoding
    int stbi_info_from_memory(const stbi_uc* buffer, int len, int* x, int* y, int* comp);
    int stbi_info_from_callbacks(const stbi_io_callbacks* clbk, void* user, int* x, int* y,
                                 int* comp);
    int stbi_is_16_bit_from_memory(const stbi_uc* buffer, int len);
    int stbi_is_16_bit_from_callbacks(const stbi_io_callbacks* clbk, void* user);

#ifndef STBI_NO_STDIO
    int stbi_info(const char* filename, int* x, int* y, int* comp);
    int stbi_info_from_file(FILE* f, int* x, int* y, int* comp);
    int stbi_is_16_bit(const char* filename);
    int stbi_is_16_bit_from_file(FILE* f);
#endif

    // for image formats that explicitly notate that they have premultiplied alpha,
    // we just return the colors as stored in the file. set this flag to force
    // unpremultiplication. results are undefined if the unpremultiply overflow.
    void stbi_set_unpremultiply_on_load(int flag_true_if_should_unpremultiply);

    // indicate whether we should process iphone images back to canonical format,
    // or just pass them through "as-is"
    void stbi_convert_iphone_png_to_rgb(int flag_true_if_should_convert);

    // flip the image vertically, so the first pixel in the output array is the bottom left
    void stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);

    // as above, but only applies to images loaded on the thread that calls the function
    // this function is only available if your compiler supports thread-local variables;
    // calling it will fail to link if your compiler doesn't
    void stbi_set_unpremultiply_on_load_thread(int flag_true_if_should_unpremultiply);
    void stbi_convert_iphone_png_to_rgb_thread(int flag_true_if_should_convert);
    void stbi_set_flip_vertically_on_load_thread(int flag_true_if_should_flip);

    // ZLIB client - used by PNG, available for other purposes

    char* stbi_zlib_decode_malloc_guesssize(char* buffer, int len, int initial_size, int* outlen);
    char* stbi_zlib_decode_malloc_guesssize_headerflag(char* buffer, int len, int initial_size,
                                                       int* outlen, int parse_header);
    char* stbi_zlib_decode_malloc(char* buffer, int len, int* outlen);
    int stbi_zlib_decode_buffer(char* obuffer, int olen, char* ibuffer, int ilen);

    char* stbi_zlib_decode_noheader_malloc(char* buffer, int len, int* outlen);
    int stbi_zlib_decode_noheader_buffer(char* obuffer, int olen, char* ibuffer, int ilen);

}
