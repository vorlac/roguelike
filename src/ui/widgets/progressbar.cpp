#include "ui/widgets/progressbar.hpp"

namespace rl::ui {

    ProgressBar::ProgressBar(Widget* parent)
        : Widget{ parent }
    {
    }

    f32 ProgressBar::value() const
    {
        return m_value;
    }

    void ProgressBar::set_value(const f32 value)
    {
        m_value = value;
    }

    ds::dims<f32> ProgressBar::preferred_size() const
    {
        return ds::dims{ 70.0f, 12.0f };
    }

    void ProgressBar::draw()
    {
        Widget::draw();

        auto&& context{ m_renderer->context() };
        nvg::PaintStyle paint{ nvg::box_gradient(context, m_rect.pt.x + 1.0f, m_rect.pt.y + 1.0f,
                                                 m_rect.size.width - 2.0f, m_rect.size.height, 3.0f,
                                                 4.0f, ds::color<f32>{ 0, 0, 0, 32 },
                                                 ds::color<f32>{ 0, 0, 0, 92 }) };

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_rect.pt.x, m_rect.pt.y, m_rect.size.width, m_rect.size.height,
                          3.0f);
        nvg::fill_paint(context, paint);
        nvg::fill(context);

        const f32 value{ std::min(std::max(0.0f, m_value), 1.0f) };
        const f32 bar_pos{ std::round((m_rect.size.width - 2.0f) * value) };

        paint = nvg::box_gradient(
            context, m_rect.pt.x, m_rect.pt.y, bar_pos + 1.5f, m_rect.size.height - 1.0f, 3.0f,
            4.0f, ds::color<f32>{ 220, 220, 220, 100 }, ds::color<f32>{ 128, 128, 128, 100 });

        nvg::begin_path(context);
        nvg::rounded_rect(context, m_rect.pt.x + 1.0f, m_rect.pt.y + 1.0f, bar_pos,
                          m_rect.size.height - 2.0f, 3.0f);
        nvg::fill_paint(context, paint);
        nvg::fill(context);
    }
}
