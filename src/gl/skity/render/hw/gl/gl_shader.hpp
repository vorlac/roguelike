#ifndef SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
#define SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP

#include <memory>
#include <vector>

#include "gl/skity/effect/shader.hpp"
#include "gl/skity/geometry/point.hpp"

namespace skity {

    class GLPipelineShader;

    /**
     * @class Shader wrapper for internal use
     */
    class GLShader
    {
    public:
        virtual ~GLShader();
        int32_t GetUniformLocation(const char* name);
        void SetUniform(int32_t location, const glm::vec4& value);
        void SetUniform4i(int32_t location, const glm::ivec4& value);
        void SetUniform2i(int32_t location, const glm::ivec2& value);
        void SetUniform(int32_t location, const glm::vec3& value);
        void SetUniform(int32_t location, const glm::vec2& value);
        void SetUniform(int32_t location, const glm::mat4& value);
        void SetUniform(int32_t location, int32_t value);
        void SetUniform(int32_t location, float value);
        void SetUniform(int32_t location, float* value, int32_t count);
        void SetUniform(int32_t location, glm::vec2* value, int32_t count);
        void SetUniform(int32_t location, glm::vec4* value, int32_t count);
        void SetUniform(int32_t location, glm::mat4* value, int32_t count);
        void Bind();

        void UnBind();
        virtual void InitLocations();

        static std::unique_ptr<GLPipelineShader> CreatePipelineShader();
        static std::unique_ptr<GLPipelineShader> CreateGSPipelineShader();

    protected:
        GLShader() = default;

    protected:
        int32_t program_ = 0;
    };

    class GLPipelineShader : public GLShader
    {
    public:
        GLPipelineShader() = default;
        ~GLPipelineShader() override = default;

        void InitLocations() override;

    public:
        void SetMVP(const Matrix& mvp);
        void SetTransformMatrix(const Matrix& matrix);
        void SetUserTexture(int32_t unit);
        void SetFontTexture(int32_t unit);
        void SetUniformColor(const glm::vec4& color);
        void SetStrokeWidth(float width);
        void SetColorType(int32_t type);
        void SetGradientCountInfo(int32_t color_count, int32_t pos_count);
        void SetGradientBoundInfo(const glm::vec4& info);
        void SetGradientColors(const std::vector<glm::vec4>& colors);
        void SetGradientPostions(const std::vector<float>& pos);
        void SetGlobalAlpha(float alpha);

    private:
        int32_t mvp_location_ = -1;
        int32_t user_transform_locaion_ = -1;
        int32_t user_texture_location_ = -1;
        int32_t font_texture_location_ = -1;
        int32_t uniform_color_location_ = -1;
        int32_t stroke_width_location_ = -1;
        int32_t color_type_location_ = -1;
        int32_t gradient_count_location_ = -1;
        int32_t gradient_bound_location_ = -1;
        int32_t gradient_colors_location_ = -1;
        int32_t gradient_pos_location_ = -1;
        int32_t global_alpha_location_ = -1;
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_GL_GL_SHADER_HPP
