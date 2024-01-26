#include "core/ui/progressbar.hpp"

namespace rl::ui {

    ProgressBar::ProgressBar(Widget* parent)
        : Widget{ parent }
    {
    }

    f32 ProgressBar::value()
    {
        return m_value;
    }

    void ProgressBar::set_value(f32 value)
    {
        m_value = value;
    }

    ds::dims<f32> ProgressBar::preferred_size() const
    {
        return ds::dims{ 70.0f, 12.0f };
    }

    void ProgressBar::draw()
    {
        auto&& context{ m_renderer->context() };
        Widget::draw();

        nvg::NVGpaint paint{ nvg::BoxGradient(
            context, m_pos.x + 1.0f, m_pos.y + 1.0f, m_size.width - 2.0f, m_size.height, 3.0f, 4.0f,
            ds::color<f32>{ 0, 0, 0, 32 }, ds::color<f32>{ 0, 0, 0, 92 }) };

        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x, m_pos.y, m_size.width, m_size.height, 3.0f);
        nvg::FillPaint(context, paint);
        nvg::Fill(context);

        f32 value{ std::min(std::max(0.0f, m_value), 1.0f) };
        f32 bar_pos{ std::round((m_size.width - 2.0f) * value) };

        paint = nvg::BoxGradient(context, m_pos.x, m_pos.y, bar_pos + 1.5f, m_size.height - 1.0f,
                                 3.0f, 4.0f, ds::color<f32>{ 220, 220, 220, 100 },
                                 ds::color<f32>{ 128, 128, 128, 100 });

        nvg::BeginPath(context);
        nvg::RoundedRect(context, m_pos.x + 1.0f, m_pos.y + 1.0f, bar_pos, m_size.height - 2.0f,
                         3.0f);
        nvg::FillPaint(context, paint);
        nvg::Fill(context);
    }
}