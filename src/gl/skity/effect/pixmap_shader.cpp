#include "gl/skity/effect/pixmap_shader.hpp"
#include "gl/skity/io/pixmap.hpp"

namespace skity {

    PixmapShader::PixmapShader(std::shared_ptr<Pixmap> pixmap)
        : Shader()
        , pixmap_(std::move(pixmap))
    {
    }

    std::shared_ptr<Pixmap> PixmapShader::asImage() const
    {
        return pixmap_;
    }

}  // namespace skity
