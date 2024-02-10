#include <glad/gl.h>

#include <cstring>
#include <print>

#include "graphics/vg/nanovg_gl.hpp"

#define NANOVG_GL3                  1
#define NANOVG_GL_USE_UNIFORMBUFFER 1

namespace rl::nvg {

    enum GLNVGuniformLoc {
        NVGLocViewsize,
        NVGLocTex,
        NVGLocFrag,
        NVGMaxLocs
    };

    enum GLNVGshaderType {
        SVGShaderFillgrad,
        SVGShaderFillimg,
        SVGShaderSimple,
        SVGShaderImg
    };

    enum GLNVGuniformBindings {
        NVGFragBinding = 0,
    };

    struct GLNVGshader
    {
        GLuint prog;
        GLuint frag;
        GLuint vert;
        GLint loc[NVGMaxLocs];
    };

    struct GLNVGtexture
    {
        int32_t id;
        GLuint tex;
        int32_t width;
        int32_t height;
        int32_t type;
        int32_t flags;
    };

    struct GLNVGblend
    {
        GLenum src_rgb;
        GLenum dst_rgb;
        GLenum src_alpha;
        GLenum dst_alpha;
    };

    enum GLNVGcallType {
        NVGNone = 0,
        NVGFill,
        NVGConvexFill,
        NVGStroke,
        NVGTriangles,
    };

    struct GLNVGcall
    {
        int32_t type;
        int32_t image;
        int32_t path_offset;
        int32_t path_count;
        int32_t triangle_offset;
        int32_t triangle_count;
        int32_t uniform_offset;
        GLNVGblend blend_func;
    };

    struct GLNVGpath
    {
        int32_t fill_offset;
        int32_t fill_count;
        int32_t stroke_offset;
        int32_t stroke_count;
    };

    struct GLNVGfragUniforms
    {
        float scissor_mat[12];  // matrices are actually 3 vec4s
        float paint_mat[12];
        NVGcolor inner_col;
        NVGcolor outer_col;
        float scissor_ext[2];
        float scissor_scale[2];
        float extent[2];
        float radius;
        float feather;
        float stroke_mult;
        float stroke_thr;
        int32_t tex_type;
        int32_t type;
    };

    struct GLNVGcontext
    {
        GLNVGshader shader;
        GLNVGtexture* textures;
        float view[2];
        int32_t ntextures;
        int32_t ctextures;
        int32_t texture_id;
        GLuint vert_buf;
        GLuint vert_arr;
        GLuint frag_buf;
        int32_t frag_size;
        int32_t flags;

        // Per frame buffers
        GLNVGcall* calls;
        int32_t ccalls;
        int32_t ncalls;
        GLNVGpath* paths;
        int32_t cpaths;
        int32_t npaths;
        struct NVGvertex* verts;
        int32_t cverts;
        int32_t nverts;
        uint8_t* uniforms;
        int32_t cuniforms;
        int32_t nuniforms;

        // cached state
        GLuint bound_texture;
        GLuint stencil_mask;
        GLenum stencil_func;
        GLint stencil_func_ref;
        GLuint stencil_func_mask;
        GLNVGblend blend_func;

        int32_t dummy_tex;
    };

    using GLNVGcall = GLNVGcall;
    using GLNVGpath = GLNVGpath;
    using GLNVGfragUniforms = GLNVGfragUniforms;
    using GLNVGcontext = GLNVGcontext;

    namespace {
        int32_t glnvg_maxi(const int32_t a, const int32_t b)
        {
            return a > b ? a : b;
        }

        void glnvg_bind_texture(GLNVGcontext* gl, const GLuint tex)
        {
            if (gl->bound_texture != tex)
            {
                gl->bound_texture = tex;
                glBindTexture(GL_TEXTURE_2D, tex);
            }
        }

        void glnvg_stencil_mask(GLNVGcontext* gl, const GLuint mask)
        {
            if (gl->stencil_mask != mask)
            {
                gl->stencil_mask = mask;
                glStencilMask(mask);
            }
        }

        void glnvg_stencil_func(GLNVGcontext* gl, const GLenum func, const GLint ref,
                                const GLuint mask)
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

        void glnvg_blend_func_separate(GLNVGcontext* gl, const GLNVGblend* blend)
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

        GLNVGtexture* glnvg_alloc_texture(GLNVGcontext* gl)
        {
            GLNVGtexture* tex = nullptr;

            for (int32_t i = 0; i < gl->ntextures; i++)
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
                    const int32_t ctextures = glnvg_maxi(gl->ntextures + 1, 4) +
                                              gl->ctextures / 2;  // 1.5x
                                                                  // Overallocate
                    const auto textures = static_cast<GLNVGtexture*>(
                        realloc(gl->textures, sizeof(GLNVGtexture) * ctextures));
                    if (textures == nullptr)
                        return nullptr;
                    gl->textures = textures;
                    gl->ctextures = ctextures;
                }
                tex = &gl->textures[gl->ntextures++];
            }

            memset(tex, 0, sizeof(*tex));
            tex->id = ++gl->texture_id;

            return tex;
        }

        GLNVGtexture* glnvg_findTexture(const GLNVGcontext* gl, const int32_t id)
        {
            for (int32_t i = 0; i < gl->ntextures; i++)
                if (gl->textures[i].id == id)
                    return &gl->textures[i];
            return nullptr;
        }

        int32_t glnvg_deleteTexture(const GLNVGcontext* gl, const int32_t id)
        {
            for (int32_t i = 0; i < gl->ntextures; i++)
            {
                if (gl->textures[i].id == id)
                {
                    if (gl->textures[i].tex != 0 &&
                        (gl->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
                        glDeleteTextures(1, &gl->textures[i].tex);
                    memset(&gl->textures[i], 0, sizeof(gl->textures[i]));
                    return 1;
                }
            }
            return 0;
        }

        void glnvg_dump_shader_error(const GLuint shader, const char* name, const char* type)
        {
            GLchar str[512 + 1];
            GLsizei len = 0;
            glGetShaderInfoLog(shader, 512, &len, str);
            if (len > 512)
                len = 512;
            str[len] = '\0';
            std::println("Shader {}/{} error:\n{}", name, type, str);
        }

        void glnvg_dump_program_error(const GLuint prog, const char* name)
        {
            GLchar str[512 + 1];
            GLsizei len = 0;
            glGetProgramInfoLog(prog, 512, &len, str);
            if (len > 512)
                len = 512;
            str[len] = '\0';
            std::println("Program {} error:\n{}", name, str);
        }

        void glnvg_check_error(const GLNVGcontext* gl, const char* str)
        {
            if ((gl->flags & NVG_DEBUG) == 0)
                return;

            const GLenum err = glGetError();
            if (err != GL_NO_ERROR)
                std::print("Error {:08x} after {}", err, str);
        }

        int32_t glnvg_create_shader(GLNVGshader* shader, const char* name, const char* header,
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
                glnvg_dump_shader_error(vert, name, "vert");
                return 0;
            }

            glCompileShader(frag);
            glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
            if (status != GL_TRUE)
            {
                glnvg_dump_shader_error(frag, name, "frag");
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
                glnvg_dump_program_error(prog, name);
                return 0;
            }

            shader->prog = prog;
            shader->vert = vert;
            shader->frag = frag;

            return 1;
        }

        void glnvg_delete_shader(const GLNVGshader* shader)
        {
            if (shader->prog != 0)
                glDeleteProgram(shader->prog);
            if (shader->vert != 0)
                glDeleteShader(shader->vert);
            if (shader->frag != 0)
                glDeleteShader(shader->frag);
        }

        void glnvg_get_uniforms(GLNVGshader* shader)
        {
            shader->loc[NVGLocViewsize] = glGetUniformLocation(shader->prog, "viewSize");
            shader->loc[NVGLocTex] = glGetUniformLocation(shader->prog, "tex");
            shader->loc[NVGLocFrag] = glGetUniformBlockIndex(shader->prog, "frag");
        }

        int32_t glnvg_render_create_texture(void* uptr, int32_t type, int32_t w, int32_t h,
                                            int32_t image_flags, const uint8_t* data);

        int32_t glnvg_render_create(void* uptr)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);

            // TODO: mediump float may not be enough for GLES2 in iOS.
            // see the following discussion: https://github.com/memononen/nanovg/issues/46
            static auto shader_header =
                "#version 150 core\n"
                "#define NANOVG_GL3 1\n"
                "#define USE_UNIFORMBUFFER 1\n"
                "\n";

            static auto fill_vert_shader =
                "uniform vec2 viewSize;\n"
                "in vec2 vertex;\n"
                "in vec2 tcoord;\n"
                "out vec2 ftcoord;\n"
                "out vec2 fpos;\n"
                "\n"
                "void main(void) {\n"
                "	ftcoord = tcoord;\n"
                "	fpos = vertex;\n"
                "	gl_Position = vec4(2.0*vertex.x/viewSize.x - 1.0, 1.0 - 2.0*vertex.y/viewSize.y, 0, 1);\n"
                "}\n";

            static auto fill_frag_shader =
                "#ifdef GL_ES\n"
                "#if defined(GL_FRAGMENT_PRECISION_HIGH) || defined(NANOVG_GL3)\n"
                " precision highp float;\n"
                "#else\n"
                " precision mediump float;\n"
                "#endif\n"
                "#endif\n"
                "#ifdef NANOVG_GL3\n"
                "#ifdef USE_UNIFORMBUFFER\n"
                "	layout(std140) uniform frag {\n"
                "		mat3 scissorMat;\n"
                "		mat3 paintMat;\n"
                "		vec4 innerCol;\n"
                "		vec4 outerCol;\n"
                "		vec2 scissorExt;\n"
                "		vec2 scissorScale;\n"
                "		vec2 extent;\n"
                "		float radius;\n"
                "		float feather;\n"
                "		float strokeMult;\n"
                "		float strokeThr;\n"
                "		int texType;\n"
                "		int type;\n"
                "	};\n"
                "#else\n"  // NANOVG_GL3 && !USE_UNIFORMBUFFER
                "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
                "#endif\n"
                "	uniform sampler2D tex;\n"
                "	in vec2 ftcoord;\n"
                "	in vec2 fpos;\n"
                "	out vec4 outColor;\n"
                "#else\n"  // !NANOVG_GL3
                "	uniform vec4 frag[UNIFORMARRAY_SIZE];\n"
                "	uniform sampler2D tex;\n"
                "	varying vec2 ftcoord;\n"
                "	varying vec2 fpos;\n"
                "#endif\n"
                "#ifndef USE_UNIFORMBUFFER\n"
                "	#define scissorMat mat3(frag[0].xyz, frag[1].xyz, frag[2].xyz)\n"
                "	#define paintMat mat3(frag[3].xyz, frag[4].xyz, frag[5].xyz)\n"
                "	#define innerCol frag[6]\n"
                "	#define outerCol frag[7]\n"
                "	#define scissorExt frag[8].xy\n"
                "	#define scissorScale frag[8].zw\n"
                "	#define extent frag[9].xy\n"
                "	#define radius frag[9].z\n"
                "	#define feather frag[9].w\n"
                "	#define strokeMult frag[10].x\n"
                "	#define strokeThr frag[10].y\n"
                "	#define texType int(frag[10].z)\n"
                "	#define type int(frag[10].w)\n"
                "#endif\n"
                "\n"
                "float sdroundrect(vec2 pt, vec2 ext, float rad) {\n"
                "	vec2 ext2 = ext - vec2(rad,rad);\n"
                "	vec2 d = abs(pt) - ext2;\n"
                "	return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;\n"
                "}\n"
                "\n"
                "// Scissoring\n"
                "float scissorMask(vec2 p) {\n"
                "	vec2 sc = (abs((scissorMat * vec3(p,1.0)).xy) - scissorExt);\n"
                "	sc = vec2(0.5,0.5) - sc * scissorScale;\n"
                "	return clamp(sc.x,0.0,1.0) * clamp(sc.y,0.0,1.0);\n"
                "}\n"
                "#ifdef EDGE_AA\n"
                "// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.\n"
                "float strokeMask() {\n"
                "	return min(1.0, (1.0-abs(ftcoord.x*2.0-1.0))*strokeMult) * min(1.0, ftcoord.y);\n"
                "}\n"
                "#endif\n"
                "\n"
                "void main(void) {\n"
                "   vec4 result;\n"
                "	float scissor = scissorMask(fpos);\n"
                "#ifdef EDGE_AA\n"
                "	float strokeAlpha = strokeMask();\n"
                "	if (strokeAlpha < strokeThr) discard;\n"
                "#else\n"
                "	float strokeAlpha = 1.0;\n"
                "#endif\n"
                "	if (type == 0) {			// Gradient\n"
                "		// Calculate gradient color using box gradient\n"
                "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy;\n"
                "		float d = clamp((sdroundrect(pt, extent, radius) + feather*0.5) / feather, 0.0, 1.0);\n"
                "		vec4 color = mix(innerCol,outerCol,d);\n"
                "		// Combine alpha\n"
                "		color *= strokeAlpha * scissor;\n"
                "		result = color;\n"
                "	} else if (type == 1) {		// Image\n"
                "		// Calculate color fron texture\n"
                "		vec2 pt = (paintMat * vec3(fpos,1.0)).xy / extent;\n"
                "#ifdef NANOVG_GL3\n"
                "		vec4 color = texture(tex, pt);\n"
                "#else\n"
                "		vec4 color = texture2D(tex, pt);\n"
                "#endif\n"
                "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
                "		if (texType == 2) color = vec4(color.x);"
                "		// Apply color tint and alpha.\n"
                "		color *= innerCol;\n"
                "		// Combine alpha\n"
                "		color *= strokeAlpha * scissor;\n"
                "		result = color;\n"
                "	} else if (type == 2) {		// Stencil fill\n"
                "		result = vec4(1,1,1,1);\n"
                "	} else if (type == 3) {		// Textured tris\n"
                "#ifdef NANOVG_GL3\n"
                "		vec4 color = texture(tex, ftcoord);\n"
                "#else\n"
                "		vec4 color = texture2D(tex, ftcoord);\n"
                "#endif\n"
                "		if (texType == 1) color = vec4(color.xyz*color.w,color.w);"
                "		if (texType == 2) color = vec4(color.x);"
                "		color *= scissor;\n"
                "		result = color * innerCol;\n"
                "	}\n"
                "#ifdef NANOVG_GL3\n"
                "	outColor = result;\n"
                "#else\n"
                "	gl_FragColor = result;\n"
                "#endif\n"
                "}\n";

            glnvg_check_error(gl, "init");

            if ((gl->flags & NVG_ANTIALIAS) != 0)
            {
                if (glnvg_create_shader(&gl->shader, "shader", shader_header, "#define EDGE_AA 1\n",
                                        fill_vert_shader, fill_frag_shader) == 0)
                    return 0;
            }
            else
            {
                if (glnvg_create_shader(&gl->shader, "shader", shader_header, nullptr,
                                        fill_vert_shader, fill_frag_shader) == 0)
                    return 0;
            }

            glnvg_check_error(gl, "uniform locations");
            glnvg_get_uniforms(&gl->shader);

            // Create dynamic vertex array
            glGenVertexArrays(1, &gl->vert_arr);
            glGenBuffers(1, &gl->vert_buf);

            // Create UBOs
            int32_t align = 4;
            glUniformBlockBinding(gl->shader.prog, gl->shader.loc[NVGLocFrag], NVGFragBinding);
            glGenBuffers(1, &gl->frag_buf);
            glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &align);

            gl->frag_size = static_cast<int32_t>(
                sizeof(GLNVGfragUniforms) + align - sizeof(GLNVGfragUniforms) % align);

            // Some platforms does not allow to have samples to unset textures.
            // Create empty one which is bound when there's no texture specified.
            gl->dummy_tex = glnvg_render_create_texture(gl, NVGTextureAlpha, 1, 1, 0, nullptr);

            glnvg_check_error(gl, "create done");

            glFinish();

            return 1;
        }

        int32_t glnvg_render_create_texture(void* uptr, const int32_t type, const int32_t w,
                                            const int32_t h, const int32_t image_flags,
                                            const uint8_t* data)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            GLNVGtexture* tex = glnvg_alloc_texture(gl);

            if (tex == nullptr)
                return 0;

            glGenTextures(1, &tex->tex);
            tex->width = w;
            tex->height = h;
            tex->type = type;
            tex->flags = image_flags;
            glnvg_bind_texture(gl, tex->tex);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

            if (type == NVGTextureRgba)
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);

            if (image_flags & NVGImageGenerateMipmaps)
                if (image_flags & NVGImageNearest)
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            else if (image_flags & NVGImageNearest)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            if (image_flags & NVGImageNearest)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            if (image_flags & NVGImageRepeatX)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

            if (image_flags & NVGImageRepeatY)
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

            // The new way to build mipmaps on GLES and GL3
            if (image_flags & NVGImageGenerateMipmaps)
                glGenerateMipmap(GL_TEXTURE_2D);

            glnvg_check_error(gl, "create tex");
            glnvg_bind_texture(gl, 0);

            return tex->id;
        }

        int32_t glnvg_render_delete_texture(void* uptr, const int32_t image)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            return glnvg_deleteTexture(gl, image);
        }

        int32_t glnvg_render_update_texture(void* uptr, const int32_t image, const int32_t x,
                                            const int32_t y, const int32_t w, const int32_t h,
                                            const uint8_t* data)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            const GLNVGtexture* tex = glnvg_findTexture(gl, image);

            if (tex == nullptr)
                return 0;
            glnvg_bind_texture(gl, tex->tex);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, tex->width);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, x);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, y);

            if (tex->type == NVGTextureRgba)
                glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, data);
            else
                glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, GL_RED, GL_UNSIGNED_BYTE, data);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
            glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

            glnvg_bind_texture(gl, 0);

            return 1;
        }

        int32_t glnvg_render_get_texture_size(void* uptr, const int32_t image, float* w, float* h)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            const GLNVGtexture* tex = glnvg_findTexture(gl, image);
            if (tex == nullptr)
                return 0;

            *w = static_cast<float>(tex->width);
            *h = static_cast<float>(tex->height);
            return 1;
        }

        void glnvg_xform_to_mat3_x4(float* m3, const float* t)
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

        NVGcolor glnvg_premul_color(NVGcolor c)
        {
            c.r *= c.a;
            c.g *= c.a;
            c.b *= c.a;
            return c;
        }

        int32_t glnvg_convert_paint(const GLNVGcontext* gl, GLNVGfragUniforms* frag,
                                    const NVGpaint* paint, const NVGscissor* scissor,
                                    const float width, const float fringe, const float stroke_thr)
        {
            float invxform[6];

            memset(frag, 0, sizeof(*frag));

            frag->inner_col = glnvg_premul_color(paint->inner_color);
            frag->outer_col = glnvg_premul_color(paint->outer_color);

            if (scissor->extent[0] < -0.5f || scissor->extent[1] < -0.5f)
            {
                memset(frag->scissor_mat, 0, sizeof(frag->scissor_mat));
                frag->scissor_ext[0] = 1.0f;
                frag->scissor_ext[1] = 1.0f;
                frag->scissor_scale[0] = 1.0f;
                frag->scissor_scale[1] = 1.0f;
            }
            else
            {
                transform_inverse(invxform, scissor->xform);
                glnvg_xform_to_mat3_x4(frag->scissor_mat, invxform);
                frag->scissor_ext[0] = scissor->extent[0];
                frag->scissor_ext[1] = scissor->extent[1];
                frag->scissor_scale[0] = sqrtf(scissor->xform[0] * scissor->xform[0] +
                                               scissor->xform[2] * scissor->xform[2]) /
                                         fringe;
                frag->scissor_scale[1] = sqrtf(scissor->xform[1] * scissor->xform[1] +
                                               scissor->xform[3] * scissor->xform[3]) /
                                         fringe;
            }

            memcpy(frag->extent, paint->extent, sizeof(frag->extent));
            frag->stroke_mult = (width * 0.5f + fringe * 0.5f) / fringe;
            frag->stroke_thr = stroke_thr;

            if (paint->image != 0)
            {
                const GLNVGtexture* tex = glnvg_findTexture(gl, paint->image);
                if (tex == nullptr)
                    return 0;
                if ((tex->flags & NVGImageFlipY) != 0)
                {
                    float m1[6], m2[6];
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

                if (tex->type == NVGTextureRgba)
                    frag->tex_type = (tex->flags & NVGImagePreMultiplied) ? 0 : 1;
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

            glnvg_xform_to_mat3_x4(frag->paint_mat, invxform);

            return 1;
        }

        GLNVGfragUniforms* nvg_frag_uniform_ptr(const GLNVGcontext* gl, const int32_t i)
        {
            return reinterpret_cast<GLNVGfragUniforms*>(&gl->uniforms[i]);
        }

        void glnvg_set_uniforms(GLNVGcontext* gl, const int32_t uniformOffset, const int32_t image)
        {
            const GLNVGtexture* tex = nullptr;
            glBindBufferRange(GL_UNIFORM_BUFFER, NVGFragBinding, gl->frag_buf, uniformOffset,
                              sizeof(GLNVGfragUniforms));

            if (image != 0)
                tex = glnvg_findTexture(gl, image);

            // If no image is set, use empty texture
            if (tex == nullptr)
                tex = glnvg_findTexture(gl, gl->dummy_tex);

            glnvg_bind_texture(gl, tex != nullptr ? tex->tex : 0);
            glnvg_check_error(gl, "tex paint tex");
        }

        void glnvg_render_viewport(void* uptr, const float width, const float height,
                                   float device_pixel_ratio)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            gl->view[0] = width;
            gl->view[1] = height;
        }

        void glnvg_fill(GLNVGcontext* gl, const GLNVGcall* call)
        {
            const GLNVGpath* paths = &gl->paths[call->path_offset];
            const int32_t npaths = call->path_count;

            // Draw shapes
            glEnable(GL_STENCIL_TEST);
            glnvg_stencil_mask(gl, 0xff);
            glnvg_stencil_func(gl, GL_ALWAYS, 0, 0xff);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            // set bindpoint for solid loc
            glnvg_set_uniforms(gl, call->uniform_offset, 0);
            glnvg_check_error(gl, "fill simple");

            glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
            glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);
            glDisable(GL_CULL_FACE);

            for (int32_t i = 0; i < npaths; i++)
                glDrawArrays(GL_TRIANGLE_FAN, paths[i].fill_offset, paths[i].fill_count);

            glEnable(GL_CULL_FACE);

            // Draw anti-aliased pixels
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            glnvg_set_uniforms(gl, call->uniform_offset + gl->frag_size, call->image);
            glnvg_check_error(gl, "fill fill");

            if (gl->flags & NVG_ANTIALIAS)
            {
                glnvg_stencil_func(gl, GL_EQUAL, 0x00, 0xff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
                // Draw fringes
                for (int32_t i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);
            }

            // Draw fill
            glnvg_stencil_func(gl, GL_NOTEQUAL, 0x0, 0xff);
            glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
            glDrawArrays(GL_TRIANGLE_STRIP, call->triangle_offset, call->triangle_count);

            glDisable(GL_STENCIL_TEST);
        }

        void glnvg_convex_fill(GLNVGcontext* gl, const GLNVGcall* call)
        {
            const GLNVGpath* paths = &gl->paths[call->path_offset];
            const int32_t npaths = call->path_count;

            glnvg_set_uniforms(gl, call->uniform_offset, call->image);
            glnvg_check_error(gl, "convex fill");

            for (int32_t i = 0; i < npaths; i++)
            {
                glDrawArrays(GL_TRIANGLE_FAN, paths[i].fill_offset, paths[i].fill_count);
                // Draw fringes
                if (paths[i].stroke_count > 0)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);
            }
        }

        void glnvg_stroke(GLNVGcontext* gl, const GLNVGcall* call)
        {
            const GLNVGpath* paths = &gl->paths[call->path_offset];
            const int32_t npaths = call->path_count;

            if (gl->flags & NVG_STENCIL_STROKES)
            {
                glEnable(GL_STENCIL_TEST);
                glnvg_stencil_mask(gl, 0xff);

                // Fill the stroke base without overlap
                glnvg_stencil_func(gl, GL_EQUAL, 0x0, 0xff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
                glnvg_set_uniforms(gl, call->uniform_offset + gl->frag_size, call->image);
                glnvg_check_error(gl, "stroke fill 0");

                for (int32_t i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);

                // Draw anti-aliased pixels.
                glnvg_set_uniforms(gl, call->uniform_offset, call->image);
                glnvg_stencil_func(gl, GL_EQUAL, 0x00, 0xff);
                glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

                for (int32_t i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);

                // Clear stencil buffer.
                glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
                glnvg_stencil_func(gl, GL_ALWAYS, 0x0, 0xff);
                glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
                glnvg_check_error(gl, "stroke fill 1");

                for (int32_t i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);

                glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
                glDisable(GL_STENCIL_TEST);
            }
            else
            {
                glnvg_set_uniforms(gl, call->uniform_offset, call->image);
                glnvg_check_error(gl, "stroke fill");
                // Draw Strokes
                for (int32_t i = 0; i < npaths; i++)
                    glDrawArrays(GL_TRIANGLE_STRIP, paths[i].stroke_offset, paths[i].stroke_count);
            }
        }

        void glnvg_triangles(GLNVGcontext* gl, const GLNVGcall* call)
        {
            glnvg_set_uniforms(gl, call->uniform_offset, call->image);
            glnvg_check_error(gl, "triangles fill");

            glDrawArrays(GL_TRIANGLES, call->triangle_offset, call->triangle_count);
        }

        void glnvg_render_cancel(void* uptr)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            gl->nverts = 0;
            gl->npaths = 0;
            gl->ncalls = 0;
            gl->nuniforms = 0;
        }

        GLenum glnvg_convert_blend_func_factor(const int32_t factor)
        {
            if (factor == NVGZero)
                return GL_ZERO;
            if (factor == NVGOne)
                return GL_ONE;
            if (factor == NVGSrcColor)
                return GL_SRC_COLOR;
            if (factor == NVGOneMinusSrcColor)
                return GL_ONE_MINUS_SRC_COLOR;
            if (factor == NVGDstColor)
                return GL_DST_COLOR;
            if (factor == NVGOneMinusDstColor)
                return GL_ONE_MINUS_DST_COLOR;
            if (factor == NVGSrcAlpha)
                return GL_SRC_ALPHA;
            if (factor == NVGOneMinusSrcAlpha)
                return GL_ONE_MINUS_SRC_ALPHA;
            if (factor == NVGDstAlpha)
                return GL_DST_ALPHA;
            if (factor == NVGOneMinusDstAlpha)
                return GL_ONE_MINUS_DST_ALPHA;
            if (factor == NVGSrcAlphaSaturate)
                return GL_SRC_ALPHA_SATURATE;
            return GL_INVALID_ENUM;
        }

        GLNVGblend glnvg_blend_composite_operation(const NVGcompositeOperationState op)
        {
            GLNVGblend blend;
            blend.src_rgb = glnvg_convert_blend_func_factor(op.src_rgb);
            blend.dst_rgb = glnvg_convert_blend_func_factor(op.dst_rgb);
            blend.src_alpha = glnvg_convert_blend_func_factor(op.src_alpha);
            blend.dst_alpha = glnvg_convert_blend_func_factor(op.dst_alpha);
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

        void glnvg_render_flush(void* uptr)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);

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
                glBufferData(GL_UNIFORM_BUFFER, static_cast<int32_t>(gl->nuniforms * gl->frag_size),
                             gl->uniforms, GL_STREAM_DRAW);

                // Upload vertex data
                glBindVertexArray(gl->vert_arr);
                glBindBuffer(GL_ARRAY_BUFFER, gl->vert_buf);
                glBufferData(GL_ARRAY_BUFFER, gl->nverts * static_cast<int64_t>(sizeof(NVGvertex)),
                             gl->verts, GL_STREAM_DRAW);
                glEnableVertexAttribArray(0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(NVGvertex),
                                      static_cast<const GLvoid*>(nullptr));
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(NVGvertex),
                                      reinterpret_cast<const void*>(0 + 2 * sizeof(float)));

                // Set view and texture just once per frame.
                glUniform1i(gl->shader.loc[NVGLocTex], 0);
                glUniform2fv(gl->shader.loc[NVGLocViewsize], 1, gl->view);

                glBindBuffer(GL_UNIFORM_BUFFER, gl->frag_buf);

                for (int32_t i = 0; i < gl->ncalls; i++)
                {
                    const GLNVGcall* call = &gl->calls[i];
                    glnvg_blend_func_separate(gl, &call->blend_func);
                    if (call->type == NVGFill)
                        glnvg_fill(gl, call);
                    else if (call->type == NVGConvexFill)
                        glnvg_convex_fill(gl, call);
                    else if (call->type == NVGStroke)
                        glnvg_stroke(gl, call);
                    else if (call->type == NVGTriangles)
                        glnvg_triangles(gl, call);
                }

                glDisableVertexAttribArray(0);
                glDisableVertexAttribArray(1);
                glBindVertexArray(0);
                glDisable(GL_CULL_FACE);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glUseProgram(0);

                glnvg_bind_texture(gl, 0);
            }

            // Reset calls
            gl->nverts = 0;
            gl->npaths = 0;
            gl->ncalls = 0;
            gl->nuniforms = 0;
        }

        int32_t glnvg_max_vert_count(const NVGpath* paths, const int32_t npaths)
        {
            int32_t count = 0;

            for (int32_t i = 0; i < npaths; i++)
            {
                count += paths[i].nfill;
                count += paths[i].nstroke;
            }

            return count;
        }

        GLNVGcall* glnvg_alloc_call(GLNVGcontext* gl)
        {
            if (gl->ncalls + 1 > gl->ccalls)
            {
                const int32_t ccalls = glnvg_maxi(gl->ncalls + 1, 128) +
                                       gl->ccalls / 2;  // 1.5x
                                                        // Overallocate
                const auto calls = static_cast<GLNVGcall*>(
                    realloc(gl->calls, sizeof(GLNVGcall) * ccalls));

                if (calls == nullptr)
                    return nullptr;
                gl->calls = calls;
                gl->ccalls = ccalls;
            }

            GLNVGcall* ret = &gl->calls[gl->ncalls++];
            memset(ret, 0, sizeof(GLNVGcall));
            return ret;
        }

        int32_t glnvg_alloc_paths(GLNVGcontext* gl, const int32_t n)
        {
            if (gl->npaths + n > gl->cpaths)
            {
                const int32_t cpaths = glnvg_maxi(gl->npaths + n, 128) +
                                       gl->cpaths / 2;  // 1.5x
                                                        // Overallocate
                const auto paths = static_cast<GLNVGpath*>(
                    realloc(gl->paths, sizeof(GLNVGpath) * cpaths));
                if (paths == nullptr)
                    return -1;

                gl->paths = paths;
                gl->cpaths = cpaths;
            }

            const int32_t ret = gl->npaths;
            gl->npaths += n;

            return ret;
        }

        int32_t glnvg_alloc_verts(GLNVGcontext* gl, const int32_t n)
        {
            if (gl->nverts + n > gl->cverts)
            {
                // 1.5x Overallocate
                const int32_t cverts = glnvg_maxi(gl->nverts + n, 4096) + gl->cverts / 2;
                const auto verts = static_cast<NVGvertex*>(
                    realloc(gl->verts, sizeof(NVGvertex) * cverts));

                if (verts == nullptr)
                    return -1;

                gl->verts = verts;
                gl->cverts = cverts;
            }

            const int32_t ret = gl->nverts;
            gl->nverts += n;

            return ret;
        }

        int32_t glnvg_alloc_frag_uniforms(GLNVGcontext* gl, const int32_t n)
        {
            const int32_t struct_size = gl->frag_size;
            if (gl->nuniforms + n > gl->cuniforms)
            {
                const int32_t cuniforms = glnvg_maxi(gl->nuniforms + n, 128) +
                                          gl->cuniforms / 2;  // 1.5x
                                                              // Overallocate
                const auto uniforms = static_cast<uint8_t*>(
                    realloc(gl->uniforms, static_cast<int32_t>(struct_size * cuniforms)));
                if (uniforms == nullptr)
                    return -1;

                gl->uniforms = uniforms;
                gl->cuniforms = cuniforms;
            }
            const int32_t ret = gl->nuniforms * struct_size;
            gl->nuniforms += n;
            return ret;
        }

        void glnvg_vset(NVGvertex* vtx, const float x, const float y, const float u, const float v)
        {
            vtx->x = x;
            vtx->y = y;
            vtx->u = u;
            vtx->v = v;
        }

        void glnvg_render_fill(void* uptr, const NVGpaint* paint,
                               const NVGcompositeOperationState composite_operation,
                               const NVGscissor* scissor, const float fringe, const float* bounds,
                               const NVGpath* paths, const int32_t npaths)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            GLNVGcall* call = glnvg_alloc_call(gl);
            NVGvertex* quad;

            if (call == nullptr)
                return;

            call->type = NVGFill;
            call->triangle_count = 4;
            call->path_offset = glnvg_alloc_paths(gl, npaths);
            if (call->path_offset != -1)
            {
                call->path_count = npaths;
                call->image = paint->image;
                call->blend_func = glnvg_blend_composite_operation(composite_operation);

                if (npaths == 1 && paths[0].convex)
                {
                    call->type = NVGConvexFill;
                    call->triangle_count = 0;  // Bounding box fill quad not needed for convex fill
                }

                // Allocate vertices for all the paths.
                const int32_t maxverts = glnvg_max_vert_count(paths, npaths) + call->triangle_count;
                int32_t offset = glnvg_alloc_verts(gl, maxverts);
                if (offset != -1)
                {
                    for (int32_t i = 0; i < npaths; i++)
                    {
                        GLNVGpath* copy = &gl->paths[call->path_offset + i];
                        const NVGpath* path = &paths[i];
                        memset(copy, 0, sizeof(GLNVGpath));
                        if (path->nfill > 0)
                        {
                            copy->fill_offset = offset;
                            copy->fill_count = path->nfill;
                            memcpy(&gl->verts[offset], path->fill, sizeof(NVGvertex) * path->nfill);
                            offset += path->nfill;
                        }
                        if (path->nstroke > 0)
                        {
                            copy->stroke_offset = offset;
                            copy->stroke_count = path->nstroke;
                            memcpy(&gl->verts[offset], path->stroke,
                                   sizeof(NVGvertex) * path->nstroke);
                            offset += path->nstroke;
                        }
                    }

                    // Setup uniforms for draw calls
                    if (call->type == NVGFill)
                    {
                        // Quad
                        call->triangle_offset = offset;
                        quad = &gl->verts[call->triangle_offset];
                        glnvg_vset(&quad[0], bounds[2], bounds[3], 0.5f, 1.0f);
                        glnvg_vset(&quad[1], bounds[2], bounds[1], 0.5f, 1.0f);
                        glnvg_vset(&quad[2], bounds[0], bounds[3], 0.5f, 1.0f);
                        glnvg_vset(&quad[3], bounds[0], bounds[1], 0.5f, 1.0f);

                        call->uniform_offset = glnvg_alloc_frag_uniforms(gl, 2);
                        if (call->uniform_offset != -1)
                        {
                            // Simple shader for stencil
                            GLNVGfragUniforms* frag = nvg_frag_uniform_ptr(gl, call->uniform_offset);
                            memset(frag, 0, sizeof(*frag));
                            frag->stroke_thr = -1.0f;
                            frag->type = SVGShaderSimple;

                            // Fill shader
                            glnvg_convert_paint(
                                gl, nvg_frag_uniform_ptr(gl, call->uniform_offset + gl->frag_size),
                                paint, scissor, fringe, fringe, -1.0f);
                        }
                    }
                    else
                    {
                        call->uniform_offset = glnvg_alloc_frag_uniforms(gl, 1);
                        if (call->uniform_offset != -1)
                        {
                            // Fill shader
                            glnvg_convert_paint(gl, nvg_frag_uniform_ptr(gl, call->uniform_offset),
                                                paint, scissor, fringe, fringe, -1.0f);
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

        void glnvg_render_stroke(
            void* uptr, const NVGpaint* paint, const NVGcompositeOperationState compositeOperation,
            const NVGscissor* scissor, const float fringe, const float strokeWidth,
            const NVGpath* paths, const int32_t npaths)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            GLNVGcall* call = glnvg_alloc_call(gl);

            if (call == nullptr)
                return;

            call->type = NVGStroke;
            call->path_offset = glnvg_alloc_paths(gl, npaths);
            if (call->path_offset != -1)
            {
                call->path_count = npaths;
                call->image = paint->image;
                call->blend_func = glnvg_blend_composite_operation(compositeOperation);

                // Allocate vertices for all the paths.
                const int32_t maxverts = glnvg_max_vert_count(paths, npaths);
                int32_t offset = glnvg_alloc_verts(gl, maxverts);
                if (offset != -1)
                {
                    for (int32_t i = 0; i < npaths; i++)
                    {
                        GLNVGpath* copy = &gl->paths[call->path_offset + i];
                        const NVGpath* path = &paths[i];
                        memset(copy, 0, sizeof(GLNVGpath));
                        if (path->nstroke)
                        {
                            copy->stroke_offset = offset;
                            copy->stroke_count = path->nstroke;
                            memcpy(&gl->verts[offset], path->stroke,
                                   sizeof(NVGvertex) * path->nstroke);
                            offset += path->nstroke;
                        }
                    }

                    if (gl->flags & NVG_STENCIL_STROKES)
                    {
                        // Fill shader
                        call->uniform_offset = glnvg_alloc_frag_uniforms(gl, 2);
                        if (call->uniform_offset != -1)
                        {
                            glnvg_convert_paint(gl, nvg_frag_uniform_ptr(gl, call->uniform_offset),
                                                paint, scissor, strokeWidth, fringe, -1.0f);
                            glnvg_convert_paint(
                                gl, nvg_frag_uniform_ptr(gl, call->uniform_offset + gl->frag_size),
                                paint, scissor, strokeWidth, fringe, 1.0f - 0.5f / 255.0f);
                            return;
                        }
                    }
                    else
                    {
                        // Fill shader
                        call->uniform_offset = glnvg_alloc_frag_uniforms(gl, 1);
                        if (call->uniform_offset != -1)
                            glnvg_convert_paint(gl, nvg_frag_uniform_ptr(gl, call->uniform_offset),
                                                paint, scissor, strokeWidth, fringe, -1.0f);
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

        void glnvg_render_triangles(void* uptr, const NVGpaint* paint,
                                    const NVGcompositeOperationState composite_operation,
                                    const NVGscissor* scissor, const NVGvertex* verts,
                                    const int32_t nverts, const float fringe)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            GLNVGcall* call = glnvg_alloc_call(gl);

            if (call == nullptr)
                return;

            call->type = NVGTriangles;
            call->image = paint->image;
            call->blend_func = glnvg_blend_composite_operation(composite_operation);

            // Allocate vertices for all the paths.
            call->triangle_offset = glnvg_alloc_verts(gl, nverts);
            if (call->triangle_offset != -1)
            {
                call->triangle_count = nverts;

                memcpy(&gl->verts[call->triangle_offset], verts, sizeof(NVGvertex) * nverts);

                // Fill shader
                call->uniform_offset = glnvg_alloc_frag_uniforms(gl, 1);
                if (call->uniform_offset != -1)
                {
                    GLNVGfragUniforms* frag = nvg_frag_uniform_ptr(gl, call->uniform_offset);
                    glnvg_convert_paint(gl, frag, paint, scissor, 1.0f, fringe, -1.0f);
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

        void glnvg_render_delete(void* uptr)
        {
            const auto gl = static_cast<GLNVGcontext*>(uptr);
            if (gl == nullptr)
                return;

            glnvg_delete_shader(&gl->shader);

            if (gl->frag_buf != 0)
                glDeleteBuffers(1, &gl->frag_buf);
            if (gl->vert_arr != 0)
                glDeleteVertexArrays(1, &gl->vert_arr);
            if (gl->vert_buf != 0)
                glDeleteBuffers(1, &gl->vert_buf);

            for (int32_t i = 0; i < gl->ntextures; i++)
                if (gl->textures[i].tex != 0 && (gl->textures[i].flags & NVG_IMAGE_NODELETE) == 0)
                    glDeleteTextures(1, &gl->textures[i].tex);
            free(gl->textures);

            free(gl->paths);
            free(gl->verts);
            free(gl->uniforms);
            free(gl->calls);

            free(gl);
        }
    }

    NVGcontext* nvg_create_gl3(const int32_t flags)
    {
        const auto gl = static_cast<GLNVGcontext*>(malloc(sizeof(GLNVGcontext)));
        if (gl != nullptr)
        {
            NVGparams params{};
            memset(gl, 0, sizeof(GLNVGcontext));

            memset(&params, 0, sizeof(params));
            params.render_create = glnvg_render_create;
            params.render_create_texture = glnvg_render_create_texture;
            params.render_delete_texture = glnvg_render_delete_texture;
            params.render_update_texture = glnvg_render_update_texture;
            params.render_get_texture_size = glnvg_render_get_texture_size;
            params.render_viewport = glnvg_render_viewport;
            params.render_cancel = glnvg_render_cancel;
            params.render_flush = glnvg_render_flush;
            params.render_fill = glnvg_render_fill;
            params.render_stroke = glnvg_render_stroke;
            params.render_triangles = glnvg_render_triangles;
            params.render_delete = glnvg_render_delete;
            params.user_ptr = gl;
            params.edge_anti_alias = flags & NVG_ANTIALIAS ? 1 : 0;

            gl->flags = flags;

            NVGcontext* ctx = create_internal(&params);
            if (ctx != nullptr)
                return ctx;
        }

        // error:
        //  'gl' is freed by nvgDeleteInternal.
        return nullptr;
    }

    void nvg_delete_gl3(NVGcontext* ctx)
    {
        delete_internal(ctx);
    }

    int32_t nvgl_create_image_from_handle_gl3(NVGcontext* ctx, const uint32_t textureId,
                                              const int32_t w, const int32_t h,
                                              const int32_t imageFlags)
    {
        const auto gl = static_cast<GLNVGcontext*>(internal_params(ctx)->user_ptr);
        GLNVGtexture* tex = glnvg_alloc_texture(gl);

        if (tex == nullptr)
            return 0;

        tex->type = NVGTextureRgba;
        tex->tex = textureId;
        tex->flags = imageFlags;
        tex->width = w;
        tex->height = h;

        return tex->id;
    }

    uint32_t nvgl_image_handle_gl3(NVGcontext* ctx, const int32_t image)
    {
        const auto gl = static_cast<GLNVGcontext*>(internal_params(ctx)->user_ptr);
        const GLNVGtexture* tex = glnvg_findTexture(gl, image);
        return tex->tex;
    }

    const NanoVG_GL_Functions_VTable NANO_VG_GL3_FUNCTIONS_V_TABLE = {
        .name = "GL3",
        .createContext = &nvg_create_gl3,
        .deleteContext = &nvg_delete_gl3,
        .createImageFromHandle = &nvgl_create_image_from_handle_gl3,
        .getImageHandle = &nvgl_image_handle_gl3,
    };

}
