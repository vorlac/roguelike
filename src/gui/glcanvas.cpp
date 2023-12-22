#include "gui/glcanvas.hpp"
#include "gui/opengl.hpp"
#include "gui/screen.hpp"
#include "gui/theme.hpp"
#include "gui/window.hpp"

namespace rl::gui {
    GLCanvas::GLCanvas(Widget* parent)
        : Widget(parent)
        , mBackgroundColor(gui::Color(128, 128, 128, 255))
        , mDrawBorder(true)
    {
        m_size = gui::Vector2i(250, 250);
    }

    void GLCanvas::drawWidgetBorder(NVGcontext* ctx) const
    {
        nvgBeginPath(ctx);
        nvgStrokeWidth(ctx, 1.0f);
        nvgRoundedRect(ctx, m_pos.x() - 0.5f, m_pos.y() - 0.5f, m_size.x() + 1, m_size.y() + 1,
                       m_theme->m_window_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_light);
        nvgRoundedRect(ctx, m_pos.x() - 1.0f, m_pos.y() - 1.0f, m_size.x() + 2, m_size.y() + 2,
                       m_theme->m_window_corner_radius);
        nvgStrokeColor(ctx, m_theme->m_border_dark);
        nvgStroke(ctx);
    }

    void GLCanvas::draw(NVGcontext* ctx)
    {
        Widget::draw(ctx);
        nvgEndFrame(ctx);

        if (mDrawBorder)
            this->drawWidgetBorder(ctx);

        const Screen* screen = this->screen();
        assert(screen);

        float pixelRatio = screen->pixel_ratio();
        Eigen::Vector2f screenSize{ (float)screen->size().x(), (float)screen->size().y() };
        Eigen::Vector2i positionInScreen{ absolute_position().x(), absolute_position().y() };

        auto resize = m_size * pixelRatio;
        Eigen::Vector2i size{ resize.x(), resize.y() };
        auto img_pos{ (gui::Vector2i(positionInScreen[0],
                                     screenSize[1] - positionInScreen[1] - (float)m_size[1]) *
                       pixelRatio) };
        Eigen::Vector2i imagePosition{ (int)img_pos.x(), (int)img_pos.y() };

        GLint storedViewport[4];
        glGetIntegerv(GL_VIEWPORT, storedViewport);

        glViewport(imagePosition[0], imagePosition[1], size[0], size[1]);

        glEnable(GL_SCISSOR_TEST);
        glScissor(imagePosition[0], imagePosition[1], size[0], size[1]);
        glClearColor(mBackgroundColor[0], mBackgroundColor[1], mBackgroundColor[2],
                     mBackgroundColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        drawGL();

        glDisable(GL_SCISSOR_TEST);
        glViewport(storedViewport[0], storedViewport[1], storedViewport[2], storedViewport[3]);
    }

    // void GLCanvas::save(Serializer& s) const
    //{
    //     // if (!Widget::save(s))
    //     //     return false;
    //     s.set("backgroundColor", mBackgroundColor);
    //     s.set("drawBorder", mDrawBorder);
    // }

    // bool GLCanvas::load(Serializer& s) const
    //{
    //     // if (!Widget::load(s))
    //     //     return false;
    //     if (!s.get("backgroundColor", mBackgroundColor))
    //         return false;
    //     if (!s.get("drawBorder", mDrawBorder))
    //         return false;
    //     return true;
    // }
}
