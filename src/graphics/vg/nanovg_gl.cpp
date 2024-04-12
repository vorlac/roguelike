#include <glad/gl.h>

#include <cstring>
#include <print>

#include "graphics/vg/nanovg_gl.hpp"

namespace rl::nvg::gl {

    enum GLUniformLoc {
        LocViewsize,
        LocTex,
        LocFrag,
        MaxLocs
    };

    enum GLShaderType {
        SVGShaderFillgrad,
        SVGShaderFillimg,
        SVGShaderSimple,
        SVGShaderImg
    };

    enum GLUniformBindings {
        FragBinding = 0,
    };

    enum GLCallType {
        NVGNone = 0,
        NVGFill,
        NVGConvexFill,
        NVGStroke,
        NVGTriangles,
    };

    struct GLShader
    {
        GLuint prog{ 0 };
        GLuint frag{ 0 };
        GLuint vert{ 0 };
        GLint loc[MaxLocs] = {};
    };

    struct GLTexture
    {
        i32 id{ 0 };
        GLuint tex{ 0 };
        i32 width{ 0 };
        i32 height{ 0 };
        TextureProperty type{ TextureProperty::None };
        ImageFlags flags{ ImageFlags::None };
    };

    struct GLBlend
    {
        GLenum src_rgb{ 0 };
        GLenum dst_rgb{ 0 };
        GLenum src_alpha{ 0 };
        GLenum dst_alpha{ 0 };
    };

    struct GLCall
    {
        i32 type{ 0 };
        i32 image{ 0 };
        i32 path_offset{ 0 };
        i32 path_count{ 0 };
        i32 triangle_offset{ 0 };
        i32 triangle_count{ 0 };
        i32 uniform_offset{ 0 };
        GLBlend blend_func{ 0 };
    };

    struct GLPath
    {
        i32 fill_offset{ 0 };
        i32 fill_count{ 0 };
        i32 stroke_offset{ 0 };
        i32 stroke_count{ 0 };
    };

    struct GLFragUniforms
    {
        // matrices are actually 3 vec4s
        f32 scissor_mat[12] = {};
        f32 paint_mat[12] = {};
        ds::color<f32> inner_col{ 0, 0, 0, 0 };
        ds::color<f32> outer_col{ 0, 0, 0, 0 };
        f32 scissor_ext[2] = {};
        f32 scissor_scale[2] = {};
        f32 extent[2] = {};
        f32 radius{ 0.0f };
        f32 feather{ 0.0f };
        f32 stroke_mult{ 0.0f };
        f32 stroke_thr{ 0.0f };
        i32 tex_type{ 0 };
        i32 type{ 0 };
    };

    struct GLContext
    {
        GLShader shader{};
        GLTexture* textures{ nullptr };
        f32 view[2] = {};
        i32 ntextures{ 0 };
        i32 ctextures{ 0 };
        i32 texture_id{ 0 };
        GLuint vert_buf{ 0 };
        GLuint vert_arr{ 0 };
        GLuint frag_buf{ 0 };
        i32 frag_size{ 0 };
        CreateFlags flags{ CreateFlags::None };

        // Per frame buffers
        GLCall* calls{ nullptr };
        i32 ccalls{ 0 };
        i32 ncalls{ 0 };
        GLPath* paths{ nullptr };
        i32 cpaths{ 0 };
        i32 npaths{ 0 };
        Vertex* verts{ nullptr };
        i32 cverts{ 0 };
        i32 nverts{ 0 };
        uint8_t* uniforms{ nullptr };
        i32 cuniforms{ 0 };
        i32 nuniforms{ 0 };

        // cached state
        GLuint bound_texture{ 0 };
        GLuint stencil_mask{ 0 };
        GLenum stencil_func{ 0 };
        GLint stencil_func_ref{ 0 };
        GLuint stencil_func_mask{ 0 };
        GLBlend blend_func{ 0 };

        i32 dummy_tex{ 0 };
    };

    namespace {
        namespace detail {
            i32 maxi(const i32 a, const i32 b)
            {
                return a > b ? a : b;
            }

            void bind_texture(GLContext* gl, const GLuint tex)
            {
                if (gl->bound_texture != tex)
                {
                    gl->bound_texture = tex;
                    glBindTexture(GL_TEXTURE_2D, tex);
                }
            }

            void stencil_mask(GLContext* gl, const GLuint mask)
            {
                if (gl->stencil_mask != mask)
                {
                    gl->stencil_mask = mask;
                    glStencilMask(mask);
                }
            }

            void stencil_func(GLContext* gl, const GLenum func, const GLint ref, const GLuint mask)
            {
                if ((gl->stencil_func != func) || (gl->stencil_func_ref != ref) ||
                    (gl->stencil_func_mask != mask))
                {
                    gl->stencil_func = func;
                    gl->stencil_func_ref = ref;
                    gl->stencil_func_mask = mask;
                    glStencilFunc(func, ref, mask);
                }
            }

            void blend_func_separate(GLContext* gl, const GLBlend* blend)
            {
                if ((gl->blend_func.src_rgb != blend->src_rgb) ||
                    (gl->blend_func.dst_rgb != blend->dst_rgb) ||
                    (gl->blend_func.src_alpha != blend->src_alpha) ||
                    (gl->blend_func.dst_alpha != blend->dst_alpha))
                {
                    gl->blend_func = *blend;
                    glBlendFuncSeparate(blend->src_rgb, blend->dst_rgb, blend->src_alpha,
                                        blend->dst_alpha);
                }
            }

            GLTexture* alloc_texture(GLContext* gl)
            {
                GLTexture* tex{ nullptr };
                for (i32 i = 0; i < gl->ntextures; i++)
                {
                    if (gl->textures[i].id == 0)
                    {
                        tex = &gl->textures[i];
                        break;
                    }
                }

                if (tex == nullptr)
                {
                    if (gl->ntextures + 1 > gl->ctextures)
                    {
                        // 1.5x Overallocate
                        const i32 ctextures{ maxi(gl->ntextures + 1, 4) + gl->ctextures / 2 };
                        auto textures{ static_cast<GLTexture*>(
                            std::realloc(gl->textures, sizeof(GLTexture) * ctextures)) };

                        if (textures == nullptr)
                            return nullptr;

                        gl->textures = textures;
                        gl->ctextures = ctextures;
                    }
                    tex = &gl->textures[gl->ntextures++];
                }

                std::memset(tex, 0, sizeof(*tex));
                tex->id = ++gl->texture_id;

                return tex;
            }

            GLTexture* find_texture(const GLContext* gl, const i32 id)
            {
                for (i32 i = 0; i < gl->ntextures; i++)
                    if (gl->textures[i].id == id)
                        return &gl->textures[i];

                return nullptr;
            }

            i32 delete_texture(const GLContext* gl, const i32 id)
            {
                for (i32 i = 0; i < gl->ntextures; i++)
                {
                    if (gl->textures[i].id == id)
                    {
                        if (gl->textures[i].tex != 0 &&
                            (gl->textures[i].flags & ImageFlags::NoDelete) == 0)
                            glDeleteTextures(1, &gl->textures[i].tex);

                        std::memset(&gl->textures[i], 0, sizeof(gl->textures[i]));
                        return 1;
                    }
                }
                return 0;
            }

            void dump_shader_error(const GLuint shader, const char* name, const char* type)
            {
                GLsizei len{ 0 };
                GLchar str[512 + 1];

                glGetShaderInfoLog(shader, 512, &len, str);
                if (len > 512)
                    len = 512;

                str[len] = '\0';
                std::println("Shader {}/{} error:\n{}", name, type, str);
            }

            void dump_program_error(const GLuint prog, const char* name)
            {
                GLsizei len{ 0 };
                GLchar str[512 + 1]{};

                glGetProgramInfoLog(prog, 512, &len, str);
                if (len > 512)
                    len = 512;

                str[len] = '\0';
                std::println("Program {} error:\n{}", name, str);
            }

            void check_error(const GLContext* gl, const char* str)
            {
                if ((gl->flags & CreateFlags::Debug) == 0)
                    return;

                const GLenum err = glGetError();
                if (err != GL_NO_ERROR)
                    std::print("Error {:08x} after {}", err, str);
            }

            i32 create_shader(GLShader* shader, const char* name, const char* header,
                              const char* opts, const char* vshader, const char* fshader)
            {
                GLint status;
                const char* str[3];
                str[0] = header;
                str[1] = opts != nullptr ? opts : "";

                memset(shader, 0, sizeof(*shader));

                const GLuint prog = glCreateProgram();
                const GLuint vert = glCreateShader(GL_VERTEX_SHADER);
                const GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
                str[2] = vshader;
                glShaderSource(vert, 3, str, nullptr);
                str[2] = fshader;
                glShaderSource(frag, 3, str, nullptr);

                glCompileShader(vert);
                glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
                if (status != GL_TRUE)
                {
                    dump_shader_error(vert, name, "vert");
                    return 0;
                }

                glCompileShader(frag);
                glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
                if (status != GL_TRUE)
                {
                    dump_shader_error(frag, name, "frag");
                    return 0;
                }

                glAttachShader(prog, vert);
                glAttachShader(prog, frag);

                glBindAttribLocation(prog, 0, "vertex");
                glBindAttribLocation(prog, 1, "tcoord");

                glLinkProgram(prog);
                glGetProgramiv(prog, GL_LINK_STATUS, &status);
                if (status != GL_TRUE)
                {
                    dump_program_error(prog, name);
                    return 0;
                }

                shader->prog = prog;
                shader->vert = vert;
                shader->frag = frag;

                return 1;
            }

            void delete_shader(const GLShader* shader)
            {
                if (shader->prog != 0)
                    glDeleteProgram(shader->prog);
                if (shader->vert != 0)
                    glDeleteShader(shader->vert);
                if (shader->frag != 0)
                    glDeleteShader(shader->frag);
            }

            void get_uniforms(GLShader* shader)
            {
                shader->loc[LocViewsize] = glGetUniformLocation(shader->prog, "viewSize");
                shader->loc[LocTex] = glGetUniformLocation(shader->prog, "tex");
                shader->loc[LocFrag] = glGetUniformBlockIndex(shader->prog, "frag");
            }

            i32 render_create_texture(void* uptr, TextureProperty type, i32 w, i32 h,
                                      ImageFlags image_flags, const uint8_t* data);

            i32 render_create(void* uptr)
            {
                auto gl{ static_cast<GLContext*>(uptr) };
                static auto shader_header =
                    "#version 330 core\n"
                    "\n";

                static auto fill_vert_shader =
                    "uniform vec2 viewSize;\n"
                    "in vec2 vertex;\n"
                    "in vec2 tcoord;\n"
                    "out vec2 ftcoord;\n"
                    "out vec2 fpos;\n"
                    "\n"
                    "void main(void) {\n"
                    "    ftcoord = tcoord;\n"
                    "    fpos = vertex;\n"
                    "    gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
                    "}\n";

                static auto fill_frag_shader =
                    "layout(std140) uniform frag {\n"
                    "    mat3 scissorMat;\n"
                    "    mat3 paintMat;\n"
                    "    vec4 innerCol;\n"
                    "    vec4 outerCol;\n"
                    "    vec2 scissorExt;\n"
                    "    vec2 scissorScale;\n"
                    "    vec2 extent;\n"
                    "    float radius;\n"
                    "    float feather;\n"
                    "    float strokeMult;\n"
                    "    float strokeThr;\n"
                    "    int texType;\n"
                    "    int type;\n"
                    "};\n"
                    "uniform sampler2D tex;\n"
                    "in vec2 ftcoord;\n"
                    "in vec2 fpos;\n"
                    "out vec4 outColor;\n"
                    "\n"
                    "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
                    "    vec2 ext2 = ext - vec2(rad,rad);\n"
                    "    vec2 d = abs(pt) - ext2;\n"
                    "    return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
                    "}\n"
                    "\n"
                    "// Scissoring\n"
                    "float scissorMask(vec2 p) {\n"
                    "    vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
                    "    sc = vec2(0.5,0.5) - sc * scissorScale;\n"
                    "    return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
                    "}\n"
                    "#ifdef EDGE_AA\n"
                    "  // Stroke - from [0..1] to clipped pyramid, where the slope is 1px.\n"
                    "  float strokeMask() {\n"
                    "      return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);\n"
                    "  }\n"
                    "#endif\n"
                    "\n"
                    "void main(void) {\n"
                    "    vec4 result;\n"
                    "    float scissor = scissorMask(fpos);\n"
                    "#ifdef EDGE_AA\n"
                    "    float strokeAlpha = strokeMask();\n"
                    "    if (strokeAlpha < strokeThr) discard;\n"
                    "#else\n"
                    "    float strokeAlpha = 1.0;\n"
                    "#endif\n"
                    "    if (type == 0) {            // Gradient\n"
                    "        // Calculate gradient color using box gradient\n"
                    "        vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
                    "        float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);\n"
                    "        vec4 color = mix(innerCol,outerCol,d);\n"
                    "        // Combine alpha\n"
                    "        color *= strokeAlpha * scissor;\n"
                    "        result = color;\n"
                    "    } else if (type == 1) {        // Image\n"
                    "        // Calculate color fron texture\n"
                    "        vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
                    "        vec4 color = texture(tex, pt);\n"
                    "        if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
                    "        if (texType == 2) color = vec4(color.x);"
                    "        // Apply color tint and alpha.\n"
                    "        color *= innerCol;\n"
                    "        // Combine alpha\n"
                    "        color *= strokeAlpha * scissor;\n"
                    "        result = color;\n"
                    "    } else if (type == 2) {        // Stencil fill\n"
                    "        result = vec4(1,1,1,1);\n"
                    "    } else if (type == 3) {        // Textured tris\n"
                    "        vec4 color = texture(tex, ftcoord);\n"
                    "        if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
                    "        if (texType == 2) color = vec4(color.x);"
                    "        color *= scissor;\n"
                    "        result = color * innerCol;\n"
                    "    }\n"
                    "    outColor = result;\n"
                    "}\n";

                check_error(gl, "init");

                if ((gl->flags & CreateFlags::AntiAlias) != 0)
                {
                    if (create_shader(&gl->shader, "shader", shader_header, "#define EDGE_AA 1\n",
                                      fill_vert_shader, fill_frag_shader) == 0)
                        return 0;
                }
                else
                {
                    if (create_shader(&gl->shader, "shader", shader_header, nullptr,
                                      fill_vert_shader, fill_frag_shader) == 0)
                        return 0;
                }

                check_error(gl, "uniform locations");
                get_uniforms(&gl->shader);

                // Create dynamic vertex array
                glGenVertexArrays(1, &gl->vert_arr);
                glGenBuffers(1, &gl->vert_buf);

                // Create UBOs
                i32 align = 4;
                glUniformBlockBinding(gl->shader.prog, gl->shader.loc[LocFrag], FragBinding);
                glGenBuffers(1, &gl->frag_buf);
                glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

                gl->frag_size = static_cast<i32>(
                    sizeof(GLFragUniforms) + align - sizeof(GLFragUniforms) % align);

                // Some platforms does not allow to have samples to unset textures.
                // Create empty one which is bound when there's no texture specified.
                gl->dummy_tex = render_create_texture(gl, TextureProperty::Alpha, 1, 1,
                                                      ImageFlags::None, nullptr);

                check_error(gl, "create done");

                glFinish();

                return 1;
            }

            i32 render_create_texture(void* uptr, const TextureProperty type, const i32 w,
                                      const i32 h, const ImageFlags image_flags,
                                      const uint8_t* data)
            {
                auto gl = static_cast<GLContext*>(uptr);
                GLTexture* tex = alloc_texture(gl);

                if (tex == nullptr)
                    return 0;

                glGenTextures(1, &tex->tex);
                tex->width = w;
                tex->height = h;
                tex->type = type;
                tex->flags = image_flags;
                bind_texture(gl, tex->tex);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

                if (type == TextureProperty::RGBA)
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                                 data);
                else
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);

                if ((image_flags & ImageFlags::NVGImageGenerateMipmaps) != 0)
                {
                    if ((image_flags & ImageFlags::NVGImageNearest) != 0)
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                        GL_NEAREST_MIPMAP_NEAREST);
                    else
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                        GL_LINEAR_MIPMAP_LINEAR);
                }
                else if ((image_flags & ImageFlags::NVGImageNearest) != 0)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

                if ((image_flags & ImageFlags::NVGImageNearest) != 0)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                if ((image_flags & ImageFlags::NVGImageRepeatX) != 0)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

                if ((image_flags & ImageFlags::NVGImageRepeatY) != 0)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

                // The new way to build mipmaps on GLES and GL3
                if ((image_flags & ImageFlags::NVGImageGenerateMipmaps) != 0)
                    glGenerateMipmap(GL_TEXTURE_2D);

                check_error(gl, "create tex");
                bind_texture(gl, 0);

                return tex->id;
            }

            i32 render_delete_texture(void* uptr, const i32 image)
            {
                auto gl{ static_cast<GLContext*>(uptr) };
                return delete_texture(gl, image);
            }

            i32 render_update_texture(void* uptr, const i32 image, const i32 x, const i32 y,
                                      const i32 w, const i32 h, const uint8_t* data)
            {
                auto gl{ static_cast<GLContext*>(uptr) };
                const GLTexture* tex = find_texture(gl, image);

                if (tex == nullptr)
                    return 0;

                bind_texture(gl, tex->tex);
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

                if (tex->type == TextureProperty::RGBA)
                    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
                else
                    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, data);

                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
                glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
                glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
                bind_texture(gl, 0);

                return 1;
            }

            i32 render_get_texture_size(void* uptr, const i32 image, f32* w, f32* h)
            {
                auto gl = static_cast<GLContext*>(uptr);
                const GLTexture* tex = find_texture(gl, image);
                if (tex == nullptr)
                    return 0;

                *w = static_cast<f32>(tex->width);
                *h = static_cast<f32>(tex->height);
                return 1;
            }

            void xform_to_mat3_x4(f32* m3, const f32* t)
            {
                m3[0] = t[0];
                m3[1] = t[1];
                m3[2] = 0.0f;
                m3[3] = 0.0f;
                m3[4] = t[2];
                m3[5] = t[3];
                m3[6] = 0.0f;
                m3[7] = 0.0f;
                m3[8] = t[4];
                m3[9] = t[5];
                m3[10] = 1.0f;
                m3[11] = 0.0f;
            }

            ds::color<f32> premul_color(ds::color<f32> c)
            {
                c.r *= c.a;
                c.g *= c.a;
                c.b *= c.a;
                return c;
            }

            i32 convert_paint(const GLContext* gl, GLFragUniforms* frag, const PaintStyle* paint,
                              const ScissorParams* scissor, const f32 width, const f32 fringe,
                              const f32 stroke_thr)
            {
                f32 invxform[6];

                std::memset(frag, 0, sizeof(*frag));

                frag->inner_col = premul_color(paint->inner_color);
                frag->outer_col = premul_color(paint->outer_color);

                if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
                {
                    std::memset(frag->scissor_mat, 0, sizeof(frag->scissor_mat));
                    frag->scissor_ext[0] = 1.0f;
                    frag->scissor_ext[1] = 1.0f;
                    frag->scissor_scale[0] = 1.0f;
                    frag->scissor_scale[1] = 1.0f;
                }
                else
                {
                    transform_inverse(invxform, scissor->xform);
                    xform_to_mat3_x4(frag->scissor_mat, invxform);
                    frag->scissor_ext[0] = scissor->extent[0];
                    frag->scissor_ext[1] = scissor->extent[1];
                    frag->scissor_scale[0] = std::sqrt(scissor->xform[0] * scissor->xform[0] +
                                                       scissor->xform[2] * scissor->xform[2]) /
                                             fringe;
                    frag->scissor_scale[1] = std::sqrt(scissor->xform[1] * scissor->xform[1] +
                                                       scissor->xform[3] * scissor->xform[3]) /
                                             fringe;
                }

                std::memcpy(frag->extent, paint->extent, sizeof(frag->extent));
                frag->stroke_mult = (width * 0.5f + fringe * 0.5f) / fringe;
                frag->stroke_thr = stroke_thr;

                if (paint->image != 0)
                {
                    const GLTexture* tex = find_texture(gl, paint->image);
                    if (tex == nullptr)
                        return 0;

                    if ((tex->flags & ImageFlags::NVGImageFlipY) != 0)
                    {
                        f32 m1[6]{};
                        f32 m2[6]{};
                        transform_translate(m1, 0.0f, frag->extent[1] * 0.5f);
                        transform_multiply(m1, paint->xform);
                        transform_scale(m2, 1.0f, -1.0f);
                        transform_multiply(m2, m1);
                        transform_translate(m1, 0.0f, -frag->extent[1] * 0.5f);
                        transform_multiply(m1, m2);
                        transform_inverse(invxform, m1);
                    }
                    else
                    {
                        transform_inverse(invxform, paint->xform);
                    }

                    frag->type = SVGShaderFillimg;
                    if (tex->type == TextureProperty::RGBA)
                        frag->tex_type = (tex->flags & ImageFlags::PreMultiplied) != 0 ? 0 : 1;
                    else
                        frag->tex_type = 2;
                }
                else
                {
                    frag->type = SVGShaderFillgrad;
                    frag->radius = paint->radius;
                    frag->feather = paint->feather;
                    transform_inverse(invxform, paint->xform);
                }

                xform_to_mat3_x4(frag->paint_mat, invxform);
                return 1;
            }

            GLFragUniforms* frag_uniform_ptr(const GLContext* gl, const i32 i)
            {
                return reinterpret_cast<GLFragUniforms*>(&gl->uniforms[i]);
            }

            void set_uniforms(GLContext* gl, const i32 uniformOffset, const i32 image)
            {
                const GLTexture* tex = nullptr;
                glBindBufferRange(GL_UNIFORM_BUFFER, FragBinding, gl->frag_buf, uniformOffset,
                                  sizeof(GLFragUniforms));

                if (image != 0)
                    tex = find_texture(gl, image);

                // If no image is set, use empty texture
                if (tex == nullptr)
                    tex = find_texture(gl, gl->dummy_tex);

                bind_texture(gl, tex != nullptr ? tex->tex : 0);
                check_error(gl, "tex paint tex");
            }

            void render_viewport(void* uptr, const f32 width, const f32 height, f32)
            {
                auto gl = static_cast<GLContext*>(uptr);
                gl->view[0] = width;
                gl->view[1] = height;
            }

            void fill(GLContext* gl, const GLCall* call)
            {
                const GLPath* paths = &gl->paths[call->path_offset];
                const i32 npaths = call->path_count;

                // Draw shapes
                glEnable(GL_STENCIL_TEST);
                stencil_mask(gl, 0xff);
                stencil_func(gl, GL_ALWAYS, 0, 0xff);
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

                // set bindpoint for solid loc
                set_uniforms(gl, call->uniform_offset, 0);
                check_error(gl, "fill simple");

                glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
                glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
                glDisable(GL_CULL_FACE);

                for (i32 i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_FAN, paths[i].fill_offset, paths[i].fill_count);

                glEnable(GL_CULL_FACE);

                // Draw anti-aliased pixels
                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

                set_uniforms(gl, call->uniform_offset + gl->frag_size, call->image);
                check_error(gl, "fill fill");

                if ((gl->flags & CreateFlags::AntiAlias) != 0)
                {
                    stencil_func(gl, GL_EQUAL, 0x00, 0xff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    // Draw fringes
                    for (i32 i = 0; i < npaths; i++)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);
                }

                // Draw fill
                stencil_func(gl, GL_NOTEQUAL, 0x0, 0xff);
                glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
                glDrawArrays(GL_TRIANGLE_STRIP, call->triangle_offset, call->triangle_count);
                glDisable(GL_STENCIL_TEST);
            }

            void convex_fill(GLContext* gl, const GLCall* call)
            {
                const GLPath* paths = &gl->paths[call->path_offset];
                const i32 npaths = call->path_count;

                set_uniforms(gl, call->uniform_offset, call->image);
                check_error(gl, "convex fill");

                for (i32 i = 0; i < npaths; i++)
                {
                    glDrawArrays(GL_TRIANGLE_FAN, paths[i].fill_offset, paths[i].fill_count);
                    // Draw fringes
                    if (paths[i].stroke_count > 0)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);
                }
            }

            void stroke(GLContext* gl, const GLCall* call)
            {
                const GLPath* paths = &gl->paths[call->path_offset];
                const i32 npaths = call->path_count;

                if ((gl->flags & CreateFlags::StencilStrokes) != 0)
                {
                    glEnable(GL_STENCIL_TEST);
                    stencil_mask(gl, 0xff);

                    // Fill the stroke base without overlap
                    stencil_func(gl, GL_EQUAL, 0x0, 0xff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                    set_uniforms(gl, call->uniform_offset + gl->frag_size, call->image);
                    check_error(gl, "stroke fill 0");

                    for (i32 i = 0; i < npaths; i++)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);

                    // Draw anti-aliased pixels.
                    set_uniforms(gl, call->uniform_offset, call->image);
                    stencil_func(gl, GL_EQUAL, 0x00, 0xff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

                    for (i32 i = 0; i < npaths; i++)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);

                    // Clear stencil buffer.
                    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                    stencil_func(gl, GL_ALWAYS, 0x0, 0xff);
                    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
                    check_error(gl, "stroke fill 1");

                    for (i32 i = 0; i < npaths; i++)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);

                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glDisable(GL_STENCIL_TEST);
                }
                else
                {
                    set_uniforms(gl, call->uniform_offset, call->image);
                    check_error(gl, "stroke fill");
                    // Draw Strokes
                    for (i32 i = 0; i < npaths; i++)
                        glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset,
                                     paths[i].stroke_count);
                }
            }

            void triangles(GLContext* gl, const GLCall* call)
            {
                set_uniforms(gl, call->uniform_offset, call->image);
                check_error(gl, "triangles fill");

                glDrawArrays(GL_TRIANGLES, call->triangle_offset, call->triangle_count);
            }

            void render_cancel(void* uptr)
            {
                auto gl = static_cast<GLContext*>(uptr);
                gl->nverts = 0;
                gl->npaths = 0;
                gl->ncalls = 0;
                gl->nuniforms = 0;
            }

            GLenum convert_blend_func_factor(const BlendFactor factor)
            {
                if (factor == BlendFactor::Zero)
                    return GL_ZERO;
                if (factor == BlendFactor::One)
                    return GL_ONE;
                if (factor == BlendFactor::SrcColor)
                    return GL_SRC_COLOR;
                if (factor == BlendFactor::OneMinusSrcColor)
                    return GL_ONE_MINUS_SRC_COLOR;
                if (factor == BlendFactor::DstColor)
                    return GL_DST_COLOR;
                if (factor == BlendFactor::OneMinusDstColor)
                    return GL_ONE_MINUS_DST_COLOR;
                if (factor == BlendFactor::SrcAlpha)
                    return GL_SRC_ALPHA;
                if (factor == BlendFactor::OneMinusSrcAlpha)
                    return GL_ONE_MINUS_SRC_ALPHA;
                if (factor == BlendFactor::DstAlpha)
                    return GL_DST_ALPHA;
                if (factor == BlendFactor::OneMinusDstAlpha)
                    return GL_ONE_MINUS_DST_ALPHA;
                if (factor == BlendFactor::SrcAlphaSaturate)
                    return GL_SRC_ALPHA_SATURATE;
                return GL_INVALID_ENUM;
            }

            GLBlend blend_composite_operation(const CompositeOperationState op)
            {
                GLBlend blend;
                blend.src_rgb = convert_blend_func_factor(op.src_rgb);
                blend.dst_rgb = convert_blend_func_factor(op.dst_rgb);
                blend.src_alpha = convert_blend_func_factor(op.src_alpha);
                blend.dst_alpha = convert_blend_func_factor(op.dst_alpha);

                if (blend.src_rgb == GL_INVALID_ENUM || blend.dst_rgb == GL_INVALID_ENUM ||
                    blend.src_alpha == GL_INVALID_ENUM || blend.dst_alpha == GL_INVALID_ENUM)
                {
                    blend.src_rgb = GL_ONE;
                    blend.dst_rgb = GL_ONE_MINUS_SRC_ALPHA;
                    blend.src_alpha = GL_ONE;
                    blend.dst_alpha = GL_ONE_MINUS_SRC_ALPHA;
                }
                return blend;
            }

            void render_flush(void* uptr)
            {
                auto gl = static_cast<GLContext*>(uptr);

                if (gl->ncalls > 0)
                {
                    // Setup require GL state.
                    glUseProgram(gl->shader.prog);

                    glEnable(GL_CULL_FACE);
                    glCullFace(GL_BACK);
                    glFrontFace(GL_CCW);
                    glEnable(GL_BLEND);
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_SCISSOR_TEST);
                    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                    glStencilMask(0xffffffff);
                    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                    glStencilFunc(GL_ALWAYS, 0, 0xffffffff);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    gl->bound_texture = 0;
                    gl->stencil_mask = 0xffffffff;
                    gl->stencil_func = GL_ALWAYS;
                    gl->stencil_func_ref = 0;
                    gl->stencil_func_mask = 0xffffffff;
                    gl->blend_func.src_rgb = GL_INVALID_ENUM;
                    gl->blend_func.src_alpha = GL_INVALID_ENUM;
                    gl->blend_func.dst_rgb = GL_INVALID_ENUM;
                    gl->blend_func.dst_alpha = GL_INVALID_ENUM;

                    // Upload ubo for frag shaders
                    glBindBuffer(GL_UNIFORM_BUFFER, gl->frag_buf);
                    glBufferData(GL_UNIFORM_BUFFER, static_cast<i32>(gl->nuniforms * gl->frag_size),
                                 gl->uniforms, GL_STREAM_DRAW);

                    // Upload vertex data
                    glBindVertexArray(gl->vert_arr);
                    glBindBuffer(GL_ARRAY_BUFFER, gl->vert_buf);
                    glBufferData(GL_ARRAY_BUFFER, gl->nverts * static_cast<int64_t>(sizeof(Vertex)),
                                 gl->verts, GL_STREAM_DRAW);
                    glEnableVertexAttribArray(0);
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                          static_cast<const GLvoid*>(nullptr));
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                                          reinterpret_cast<const void*>(0 + 2 * sizeof(f32)));

                    // Set view and texture just once per frame.
                    glUniform1i(gl->shader.loc[LocTex], 0);
                    glUniform2fv(gl->shader.loc[LocViewsize], 1, gl->view);

                    glBindBuffer(GL_UNIFORM_BUFFER, gl->frag_buf);

                    for (i32 i = 0; i < gl->ncalls; i++)
                    {
                        GLCall* call{ &gl->calls[i] };
                        blend_func_separate(gl, &call->blend_func);
                        if (call->type == NVGFill)
                            fill(gl, call);
                        else if (call->type == NVGConvexFill)
                            convex_fill(gl, call);
                        else if (call->type == NVGStroke)
                            stroke(gl, call);
                        else if (call->type == NVGTriangles)
                            triangles(gl, call);
                    }

                    glDisableVertexAttribArray(0);
                    glDisableVertexAttribArray(1);
                    glBindVertexArray(0);
                    glDisable(GL_CULL_FACE);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glUseProgram(0);

                    bind_texture(gl, 0);
                }

                // Reset calls
                gl->nverts = 0;
                gl->npaths = 0;
                gl->ncalls = 0;
                gl->nuniforms = 0;
            }

            i32 max_vert_count(const NVGpath* paths, const i32 npaths)
            {
                i32 count = 0;
                for (i32 i = 0; i < npaths; i++)
                {
                    count += paths[i].nfill;
                    count += paths[i].nstroke;
                }

                return count;
            }

            GLCall* alloc_call(GLContext* gl)
            {
                if (gl->ncalls + 1 > gl->ccalls)
                {
                    const i32 ccalls{ math::max(gl->ncalls + 1, 128) + gl->ccalls / 2 };
                    auto calls = static_cast<GLCall*>(
                        std::realloc(gl->calls, sizeof(GLCall) * ccalls));

                    if (calls == nullptr)
                        return nullptr;
                    gl->calls = calls;
                    gl->ccalls = ccalls;
                }

                GLCall* ret = &gl->calls[gl->ncalls++];
                std::memset(ret, 0, sizeof(GLCall));
                return ret;
            }

            i32 alloc_paths(GLContext* gl, const i32 n)
            {
                if (gl->npaths + n > gl->cpaths)
                {
                    const i32 cpaths{ math::max(gl->npaths + n, 128) + gl->cpaths / 2 };
                    auto paths = static_cast<GLPath*>(
                        std::realloc(gl->paths, sizeof(GLPath) * cpaths));

                    if (paths == nullptr)
                        return -1;

                    gl->paths = paths;
                    gl->cpaths = cpaths;
                }

                const i32 ret = gl->npaths;
                gl->npaths += n;

                return ret;
            }

            i32 alloc_verts(GLContext* gl, const i32 n)
            {
                if (gl->nverts + n > gl->cverts)
                {
                    // 1.5x Overallocate
                    i32 cverts{ math::max(gl->nverts + n, 4096) + gl->cverts / 2 };
                    auto verts = static_cast<Vertex*>(
                        std::realloc(gl->verts, sizeof(Vertex) * cverts));

                    if (verts == nullptr)
                        return -1;

                    gl->verts = verts;
                    gl->cverts = cverts;
                }

                const i32 ret = gl->nverts;
                gl->nverts += n;

                return ret;
            }

            i32 alloc_frag_uniforms(GLContext* gl, const i32 n)
            {
                const i32 struct_size{ gl->frag_size };
                if (gl->nuniforms + n > gl->cuniforms)
                {
                    const i32 cuniforms{ math::max(gl->nuniforms + n, 128) + gl->cuniforms / 2 };
                    u8* uniforms{ static_cast<u8*>(
                        std::realloc(gl->uniforms, static_cast<i32>(struct_size) * cuniforms)) };

                    if (uniforms == nullptr)
                        return -1;

                    gl->uniforms = uniforms;
                    gl->cuniforms = cuniforms;
                }

                const i32 ret{ gl->nuniforms * struct_size };
                gl->nuniforms += n;

                return ret;
            }

            void vset(Vertex* vtx, const f32 x, const f32 y, const f32 u, const f32 v)
            {
                vtx->x = x;
                vtx->y = y;
                vtx->u = u;
                vtx->v = v;
            }

            void render_fill(void* uptr, const PaintStyle* paint,
                             const CompositeOperationState composite_operation,
                             const ScissorParams* scissor, const f32 fringe, const f32* bounds,
                             const NVGpath* paths, const i32 npaths)
            {
                GLContext* gl = static_cast<GLContext*>(uptr);
                GLCall* call = alloc_call(gl);
                Vertex* quad;

                if (call == nullptr)
                    return;

                call->type = NVGFill;
                call->triangle_count = 4;
                call->path_offset = alloc_paths(gl, npaths);
                if (call->path_offset != -1)
                {
                    call->path_count = npaths;
                    call->image = paint->image;
                    call->blend_func = blend_composite_operation(composite_operation);

                    if (npaths == 1 && paths[0].convex)
                    {
                        call->type = NVGConvexFill;
                        call->triangle_count = 0;
                    }

                    // Allocate vertices for all the paths.
                    const i32 maxverts = max_vert_count(paths, npaths) + call->triangle_count;
                    i32 offset = alloc_verts(gl, maxverts);
                    if (offset != -1)
                    {
                        for (i32 i = 0; i < npaths; i++)
                        {
                            GLPath* copy = &gl->paths[call->path_offset + i];
                            const NVGpath* path = &paths[i];
                            std::memset(copy, 0, sizeof(GLPath));
                            if (path->nfill > 0)
                            {
                                copy->fill_offset = offset;
                                copy->fill_count = path->nfill;
                                std::memcpy(&gl->verts[offset], path->fill,
                                            sizeof(Vertex) * path->nfill);
                                offset += path->nfill;
                            }
                            if (path->nstroke > 0)
                            {
                                copy->stroke_offset = offset;
                                copy->stroke_count = path->nstroke;
                                std::memcpy(&gl->verts[offset], path->stroke,
                                            sizeof(Vertex) * path->nstroke);
                                offset += path->nstroke;
                            }
                        }

                        // Setup uniforms for draw calls
                        if (call->type == NVGFill)
                        {
                            // Quad
                            call->triangle_offset = offset;
                            quad = &gl->verts[call->triangle_offset];
                            vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
                            vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
                            vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
                            vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

                            call->uniform_offset = alloc_frag_uniforms(gl, 2);
                            if (call->uniform_offset != -1)
                            {
                                // Simple shader for stencil
                                GLFragUniforms* frag = frag_uniform_ptr(gl, call->uniform_offset);
                                std::memset(frag, 0, sizeof(*frag));
                                frag->stroke_thr = -1.0f;
                                frag->type = SVGShaderSimple;

                                // Fill shader
                                convert_paint(
                                    gl, frag_uniform_ptr(gl, call->uniform_offset + gl->frag_size),
                                    paint, scissor, fringe, fringe, -1.0f);
                            }
                        }
                        else
                        {
                            call->uniform_offset = alloc_frag_uniforms(gl, 1);
                            if (call->uniform_offset != -1)
                            {
                                // Fill shader
                                convert_paint(gl, frag_uniform_ptr(gl, call->uniform_offset), paint,
                                              scissor, fringe, fringe, -1.0f);
                            }
                        }

                        return;
                    }
                }

                // error:
                //  We get here if call alloc was ok, but something else is not.
                //  Roll back the last call to prevent drawing it.
                if (gl->ncalls > 0)
                    gl->ncalls--;
            }

            void render_stroke(void* uptr, const PaintStyle* paint,
                               const CompositeOperationState compositeOperation,
                               const ScissorParams* scissor, const f32 fringe,
                               const f32 strokeWidth, const NVGpath* paths, const i32 npaths)
            {
                auto gl = static_cast<GLContext*>(uptr);
                GLCall* call = alloc_call(gl);

                if (call == nullptr)
                    return;

                call->type = NVGStroke;
                call->path_offset = alloc_paths(gl, npaths);
                if (call->path_offset != -1)
                {
                    call->path_count = npaths;
                    call->image = paint->image;
                    call->blend_func = blend_composite_operation(compositeOperation);

                    // Allocate vertices for all the paths.
                    const i32 maxverts = max_vert_count(paths, npaths);
                    i32 offset = alloc_verts(gl, maxverts);
                    if (offset != -1)
                    {
                        for (i32 i = 0; i < npaths; i++)
                        {
                            GLPath* copy = &gl->paths[call->path_offset + i];
                            const NVGpath* path = &paths[i];
                            std::memset(copy, 0, sizeof(GLPath));
                            if (path->nstroke)
                            {
                                copy->stroke_offset = offset;
                                copy->stroke_count = path->nstroke;
                                std::memcpy(&gl->verts[offset], path->stroke,
                                            sizeof(Vertex) * path->nstroke);
                                offset += path->nstroke;
                            }
                        }

                        if ((gl->flags & CreateFlags::StencilStrokes) != 0)
                        {
                            // Fill shader
                            call->uniform_offset = alloc_frag_uniforms(gl, 2);
                            if (call->uniform_offset != -1)
                            {
                                convert_paint(gl, frag_uniform_ptr(gl, call->uniform_offset), paint,
                                              scissor, strokeWidth, fringe, -1.0f);
                                convert_paint(
                                    gl, frag_uniform_ptr(gl, call->uniform_offset + gl->frag_size),
                                    paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
                                return;
                            }
                        }
                        else
                        {
                            // Fill shader
                            call->uniform_offset = alloc_frag_uniforms(gl, 1);
                            if (call->uniform_offset != -1)
                                convert_paint(gl, frag_uniform_ptr(gl, call->uniform_offset), paint,
                                              scissor, strokeWidth, fringe, -1.0f);
                            return;
                        }
                    }
                }
                // error:
                //  We get here if call alloc was ok, but something else is not.
                //  Roll back the last call to prevent drawing it.
                if (gl->ncalls > 0)
                    gl->ncalls--;
            }

            void render_triangles(void* uptr, const PaintStyle* paint,
                                  const CompositeOperationState composite_operation,
                                  const ScissorParams* scissor, const Vertex* verts,
                                  const i32 nverts, const f32 fringe)
            {
                auto gl = static_cast<GLContext*>(uptr);
                GLCall* call = alloc_call(gl);

                if (call == nullptr)
                    return;

                call->type = NVGTriangles;
                call->image = paint->image;
                call->blend_func = blend_composite_operation(composite_operation);

                // Allocate vertices for all the paths.
                call->triangle_offset = alloc_verts(gl, nverts);
                if (call->triangle_offset != -1)
                {
                    call->triangle_count = nverts;

                    std::memcpy(&gl->verts[call->triangle_offset], verts, sizeof(Vertex) * nverts);

                    // Fill shader
                    call->uniform_offset = alloc_frag_uniforms(gl, 1);
                    if (call->uniform_offset != -1)
                    {
                        GLFragUniforms* frag = frag_uniform_ptr(gl, call->uniform_offset);
                        convert_paint(gl, frag, paint, scissor, 1.0f, fringe, -1.0f);
                        frag->type = SVGShaderImg;

                        return;
                    }
                }
                // error:
                // We get here if call alloc was ok, but something else is not.
                // Roll back the last call to prevent drawing it.
                if (gl->ncalls > 0)
                    gl->ncalls--;
            }

            void render_delete(void* uptr)
            {
                auto gl{ static_cast<GLContext*>(uptr) };
                if (gl == nullptr)
                    return;

                delete_shader(&gl->shader);

                if (gl->frag_buf != 0)
                    glDeleteBuffers(1, &gl->frag_buf);
                if (gl->vert_arr != 0)
                    glDeleteVertexArrays(1, &gl->vert_arr);
                if (gl->vert_buf != 0)
                    glDeleteBuffers(1, &gl->vert_buf);

                for (i32 i = 0; i < gl->ntextures; i++)
                    if (gl->textures[i].tex != 0 &&
                        (gl->textures[i].flags & ImageFlags::NoDelete) == 0)
                        glDeleteTextures(1, &gl->textures[i].tex);

                std::free(gl->textures);
                std::free(gl->paths);
                std::free(gl->verts);
                std::free(gl->uniforms);
                std::free(gl->calls);
                std::free(gl);
            }
        }
    }

    Context* create_gl_context(const CreateFlags flags)
    {
        auto gl = static_cast<GLContext*>(std::malloc(sizeof(GLContext)));
        if (gl != nullptr)
        {
            Params params{};
            // TODO: MEMSET HERE
            // std::memset(gl, 0, sizeof(GLContext));
            *gl = GLContext{};

            params = {
                .user_ptr = gl,
                .edge_anti_alias = (flags & CreateFlags::AntiAlias) != 0,
                .render_create = detail::render_create,
                .render_create_texture = detail::render_create_texture,
                .render_delete_texture = detail::render_delete_texture,
                .render_update_texture = detail::render_update_texture,
                .render_get_texture_size = detail::render_get_texture_size,
                .render_viewport = detail::render_viewport,
                .render_cancel = detail::render_cancel,
                .render_flush = detail::render_flush,
                .render_fill = detail::render_fill,
                .render_stroke = detail::render_stroke,
                .render_triangles = detail::render_triangles,
                .render_delete = detail::render_delete,
            };

            gl->flags = flags;

            Context* ctx{ create_internal(&params) };
            if (ctx != nullptr)
                return ctx;
        }

        // error:
        //  'gl' is freed by nvg::gl::delete_internal().
        return nullptr;
    }

    void delete_gl_context(Context* ctx)
    {
        delete_internal(ctx);
    }

    i32 create_image_from_handle(Context* ctx, const u32 texture_id, const i32 w, const i32 h,
                                 const ImageFlags image_flags)
    {
        auto gl = static_cast<GLContext*>(internal_params(ctx)->user_ptr);
        GLTexture* tex = detail::alloc_texture(gl);

        if (tex == nullptr)
            return 0;

        tex->type = TextureProperty::RGBA;
        tex->tex = texture_id;
        tex->flags = image_flags;
        tex->width = w;
        tex->height = h;

        return tex->id;
    }

    u32 image_handle(Context* ctx, const i32 image)
    {
        auto gl{ static_cast<GLContext*>(internal_params(ctx)->user_ptr) };
        const GLTexture* tex = detail::find_texture(gl, image);
        return tex->tex;
    }

}
