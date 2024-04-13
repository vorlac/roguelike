#include <memory>

#include "core/ui/theme.hpp"
#include "core/ui/widgets/label.hpp"
#include "ds/dims.hpp"
#include "graphics/vg/nanovg.hpp"

namespace rl::ui {

    Label::Label(std::string text, const f32 font_size /*= -1.0f*/,
                 const nvg::Align alignment /*= nvg::Align::HLeft | nvg::Align::VMiddle*/)
        : Label{ nullptr, std::move(text), font_size, alignment }
    {
    }

    Label::Label(Widget* parent, std::string text, const f32 font_size /*= font::InvalidSize*/,
                 const nvg::Align alignment /*= nvg::Align::HLeft | nvg::Align::VMiddle*/)
        : Widget{ parent }
        , m_text{ std::move(text) }
        , m_text_alignment{ alignment }

    {
        m_text_color = m_theme->label_font_color;
        m_text_font = m_theme->label_font_name;
        m_font_size = m_theme->label_font_size;

        if (math::not_equal(m_font_size, font::InvalidSize))
            m_font_size = font_size;

        if (alignment != nvg::Align::None)
            m_text_alignment = alignment;
    }

    const std::string& Label::text() const
    {
        return m_text;
    }

    const std::string& Label::font() const
    {
        return m_text_font;
    }

    const ds::color<f32>& Label::color() const
    {
        return m_text_color;
    }

    nvg::Align Label::text_alignment() const
    {
        return m_text_alignment;
    }

    void Label::set_text(const std::string& text)
    {
        m_text = text;
    }

    void Label::set_font(const std::string& font)
    {
        m_text_font = font;
    }

    void Label::set_text_alignment(const nvg::Align alignment)
    {
        m_text_alignment = alignment;
    }

    void Label::set_color(const ds::color<f32>& color)
    {
        m_text_color = color;
    }

    void Label::set_callback(const std::function<void()>& callable)
    {
        m_callback = callable;
    }

    void Label::set_theme(ui::Theme* theme)
    {
        Widget::set_theme(theme);
        if (m_theme != nullptr)
        {
            m_font_size = m_theme->label_font_size;
            m_text_color = m_theme->label_font_color;
        }
    }

    ds::dims<f32> Label::preferred_size() const
    {
        if (m_text.empty())
            return ds::dims<f32>::zero();

        const auto context{ m_renderer->context() };
        runtime_assert(nvg::Align::None != m_text_alignment,
                       "invalid text alignment value assigned in label");

        m_renderer->set_text_properties_(m_text_font, m_font_size, m_text_alignment);
        if (math::not_equal(m_fixed_size.width, 0.0f) && m_fixed_size.width > 0.0f)
        {
            std::array<f32, 4> bounds{};  // TODO: clean this up
            // use TL aligntment if nvg has to compute the actual font size from a predefined width
            constexpr static nvg::Align TOP_LEFT_ALIGNMENT{ nvg::Align::HLeft | nvg::Align::VTop };
            nvg::set_text_align(context, TOP_LEFT_ALIGNMENT);
            nvg::text_box_bounds_(context, m_rect.pt.x, m_rect.pt.y, m_fixed_size.width,
                                  m_text.c_str(), nullptr, bounds.data());

            const f32 textbox_height{ bounds[3] - bounds[1] };
            return ds::dims{ m_fixed_size.width, textbox_height };
        }

        nvg::set_text_align(context, m_text_alignment);
        const f32 text_width{ nvg::text_bounds_(context, 0.0f, 0.0f, m_text.c_str()) };
        return ds::dims{ text_width + 2.0f, m_font_size };
    }

    void Label::draw()
    {
        ui::Widget::draw();

        auto context{ m_renderer->context() };
        m_renderer->set_text_properties_(m_text_font, m_font_size, m_text_alignment);

        nvg::fill_color(context, m_text_color);
        if (math::not_equal(m_fixed_size.width, 0.0f) && m_fixed_size.width > 0.0f)
        {
            // TODO: clean this up
            // use TL aligntment if nvg has to compute the actual font size from a predefined width
            constexpr static nvg::Align TOP_LEFT_ALIGNMENT{ nvg::Align::HLeft | nvg::Align::VTop };
            nvg::set_text_align(context, TOP_LEFT_ALIGNMENT);
            nvg::text_box_(context, m_rect.pt.x, m_rect.pt.y, m_fixed_size.width, m_text.c_str());
        }
        else
        {
            nvg::fill_color(context, m_text_color);
            // TODO: set as constexpr defautl valye
            const ds::point<f32>& pos{ m_text_alignment == (nvg::Align::HLeft | nvg::Align::VMiddle)
                                           ? m_rect.pt
                                           : m_rect.centroid() };
            nvg::set_text_align(context, m_text_alignment);
            nvg::text_(context, pos.x, pos.y, m_text.c_str());
        }
    }
}
