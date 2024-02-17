#include <ranges>

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/dialog.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/vg/nanovg_state.hpp"
#include "utils/logging.hpp"
#include "utils/math.hpp"

namespace rl::ui {

    Widget::Widget(Widget* parent)
        : m_parent{ parent }
    {
        scoped_log();

        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::Widget(Widget* parent, const std::unique_ptr<NVGRenderer>& vg_renderer)
        : m_parent{ parent }
    {
        scoped_log();
        runtime_assert(m_renderer == nullptr, "widget vectorized renderer already set");
        if (m_renderer == nullptr)
            m_renderer = vg_renderer.get();

        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::~Widget()
    {
        scoped_log();
        for (const auto child : m_children)
            if (child != nullptr)
                child->release_ref();
    }

    Widget* Widget::parent()
    {
        scoped_log();
        return m_parent;
    }

    const Widget* Widget::parent() const
    {
        scoped_log();
        return m_parent;
    }

    void Widget::set_parent(Widget* parent)
    {
        scoped_log();
        m_parent = parent;
    }

    Layout* Widget::layout()
    {
        scoped_log();
        return m_layout;
    }

    const Layout* Widget::layout() const
    {
        scoped_log();
        return m_layout.get();
    }

    void Widget::set_layout(Layout* layout)
    {
        scoped_log();
        m_layout = layout;
    }

    Theme* Widget::theme()
    {
        scoped_log();
        return m_theme;
    }

    const Theme* Widget::theme() const
    {
        scoped_log();
        return m_theme.get();
    }

    void Widget::set_theme(Theme* theme)
    {
        scoped_log();
        if (m_theme.get() == theme)
            return;

        m_theme = theme;
        for (const auto child : m_children)
            child->set_theme(theme);
    }

    ds::point<f32> Widget::position() const
    {
        return m_pos;
    }

    void Widget::set_position(const ds::point<f32>& pos)
    {
        scoped_log();
        m_pos = pos;
    }

    ds::point<f32> Widget::abs_position() const
    {
        scoped_log();
        return m_parent != nullptr                   //
                 ? m_parent->abs_position() + m_pos  //
                 : m_pos;                            //
    }

    ds::dims<f32> Widget::size() const
    {
        return m_size;
    }

    void Widget::set_size(const ds::dims<f32>& size)
    {
        m_size = size;
    }

    f32 Widget::width() const
    {
        return m_size.width;
    }

    void Widget::set_width(const f32 width)
    {
        scoped_log();
        m_size.width = width;
    }

    f32 Widget::height() const
    {
        return m_size.height;
    }

    void Widget::set_height(const f32 height)
    {
        scoped_log();
        m_size.height = height;
    }

    void Widget::set_fixed_size(const ds::dims<f32>& fixed_size)
    {
        scoped_log();
        m_fixed_size = fixed_size;
    }

    ds::dims<f32> Widget::fixed_size() const
    {
        return m_fixed_size;
    }

    f32 Widget::fixed_width() const
    {
        return m_fixed_size.width;
    }

    f32 Widget::fixed_height() const
    {
        return m_fixed_size.height;
    }

    void Widget::set_fixed_width(const f32 width)
    {
        scoped_log();
        m_fixed_size.width = width;
    }

    void Widget::set_fixed_height(f32 height)
    {
        scoped_log("height={}", height);
        m_fixed_size.height = height;
    }

    bool Widget::visible() const
    {
        scoped_log("m_visible={}", m_visible);
        return m_visible;
    }

    void Widget::set_visible(bool visible)
    {
        scoped_log("visible={}", visible);
        m_visible = visible;
    }

    bool Widget::show()
    {
        this->set_visible(true);
        return true;
    }

    bool Widget::hide()
    {
        this->set_visible(false);
        return true;
    }

    bool Widget::visible_recursive() const
    {
        scoped_log();
        bool visible{ true };

        const Widget* widget{ this };
        while (widget != nullptr)
        {
            visible &= widget->visible();
            widget = widget->parent();
        }

        return visible;
    }

    i32 Widget::child_count() const
    {
        scoped_log("{}", m_children.size());
        return static_cast<i32>(m_children.size());
    }

    Widget* Widget::child_at(i32 index)
    {
        scoped_log("idx={} cnt={}", index, m_children.size());
        return m_children[static_cast<size_t>(index)];
    }

    const Widget* Widget::child_at(i32 index) const
    {
        scoped_log("idx={} cnt={}", index, m_children.size());
        return m_children[static_cast<size_t>(index)];
    }

    const std::vector<Widget*>& Widget::children() const
    {
        return m_children;
    }

    f32 Widget::font_size() const
    {
        return (m_font_size < 0.0f && m_theme != nullptr) ? m_theme->standard_font_size
                                                          : m_font_size;
    }

    ds::dims<f32> Widget::preferred_size() const
    {
        auto&& context{ m_renderer->context() };
        auto&& ret{ (m_layout != nullptr ? m_layout->preferred_size(context, this) : m_size) };
        scoped_logger(log_level::trace, "{}", ret);
        return ret;
    }

    void Widget::perform_layout()
    {
        scoped_trace(log_level::trace);
        auto&& context{ m_renderer->context() };
        if (m_layout != nullptr)
            m_layout->perform_layout(context, this);
        else
        {
            for (const auto child : m_children)
            {
                auto&& pref{ child->preferred_size() };
                auto&& fix{ child->fixed_size() };

                child->set_size({
                    math::is_equal(fix.width, 0.0f) ? fix.width : pref.width,
                    math::is_equal(fix.height, 0.0f) ? fix.height : pref.height,
                });

                child->perform_layout();
            }
        }
    }

    Widget* Widget::find_widget(const ds::point<f32>& pt)
    {
        scoped_trace(log_level::debug);

        {
            LocalTransform transform{ this };
            for (const auto child : std::ranges::reverse_view{ m_children })
            {
                if (!child->visible())
                    continue;
                if (child->contains(pt - m_pos))
                    return child->find_widget(pt - m_pos);
            }
        }

        return this->contains(pt) ? this : nullptr;
    }

    const Widget* Widget::find_widget(const ds::point<f32>& pt) const
    {
        scoped_trace(log_level::debug);

        LocalTransform transform{ this };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (child->contains(pt - m_pos))
                return child->find_widget(pt - m_pos);
        }

        return this->contains(pt) ? this : nullptr;
    }

    bool Widget::on_mouse_entered(const Mouse& mouse)
    {
        scoped_logger(log_level::warn, "enter_pos={}", mouse.pos());
        m_mouse_focus = true;
        return false;
    }

    bool Widget::on_mouse_exited(const Mouse& mouse)
    {
        scoped_logger(log_level::warn, "exit_pos={}", mouse.pos());
        m_mouse_focus = false;
        return false;
    }

    bool Widget::on_focus_gained()
    {
        scoped_trace(log_level::debug);
        m_focused = true;
        return false;
    }

    bool Widget::on_focus_lost()
    {
        scoped_trace(log_level::debug);
        m_focused = false;
        return false;
    }

    bool Widget::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_pressed());

        bool handled{ false };
        const auto&& mouse_pos{ mouse.pos() };

        LocalTransform transform{ this };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(mouse_pos - m_pos))
                continue;
            if (!child->on_mouse_button_pressed(mouse, kb))
                continue;

            handled = true;
            diag_log("btn pressed handled:{} mouse_pos={} rel={}, abs={}", this->name(),
                     mouse.pos(), ds::rect{ m_pos, m_size },
                     ds::rect{ this->abs_position(), m_size });

            break;
        }

        if (!handled && mouse.is_button_pressed(Mouse::Button::Left) && !m_focused)
            this->request_focus();

        return handled;
    }

    bool Widget::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_released());

        bool handled{ false };
        auto&& mouse_pos{ mouse.pos() };

        LocalTransform transform{ this };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(mouse_pos - m_pos))
                continue;
            if (!child->on_mouse_button_released(mouse, kb))
                continue;

            handled = true;
            diag_log("btn rel handled:{} mouse_pos={} rel={}, abs={}", this->name(), mouse.pos(),
                     ds::rect{ m_pos, m_size }, ds::rect{ this->abs_position(), m_size });
            break;
        }

        return handled;
    }

    bool Widget::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::trace, "pos={} wheel={}", mouse.pos(), mouse.wheel());

        bool handled{ false };
        auto&& mouse_pos{ mouse.pos() };

        LocalTransform transform{ this };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(mouse_pos - m_pos))
                continue;
            if (!child->on_mouse_scroll(mouse, kb))
                continue;

            handled = true;
            diag_log("mouse scroll handled:{} mouse_pos={} rel={}, abs={}", this->name(),
                     mouse.pos(), ds::rect{ m_pos, m_size },
                     ds::rect{ this->abs_position(), m_size });
            break;
        }

        return handled;
    }

    bool Widget::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_logger(log_level::trace, "pos={}", mouse.pos());

        bool handled{ false };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;

            const ds::point mouse_pos{ mouse.pos() };
            const bool contained{ child->contains(mouse_pos - m_pos) };
            const bool prev_contained{ child->contains(mouse_pos - m_pos - mouse.pos_delta()) };

            if (contained && !prev_contained)
                handled |= child->on_mouse_entered(mouse);
            else if (!contained && prev_contained)
                handled |= child->on_mouse_exited(mouse);

            if (contained || prev_contained)
                handled |= child->on_mouse_move(mouse, kb);
        }

        return handled;
    }

    bool Widget::on_mouse_drag(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("noop");
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_key_pressed(const Keyboard& kb)
    {
        scoped_log("noop");
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_key_released(const Keyboard& kb)
    {
        scoped_log("noop");
        // do nothing,
        // derived objects should implement
        return false;
    }

    bool Widget::on_character_input(const Keyboard& kb)
    {
        scoped_log("noop");
        // do nothing,
        // derived objects should implement
        return false;
    }

    void Widget::add_child(const i32 index, Widget* widget)
    {
        scoped_log();
        runtime_assert(index <= child_count(), "child widget index out of bounds");
        m_children.insert(m_children.begin() + index, widget);

        widget->acquire_ref();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void Widget::add_child(Widget* widget)
    {
        scoped_log("{}", widget->name());
        this->add_child(this->child_count(), widget);
    }

    void Widget::remove_child(const Widget* widget)
    {
        scoped_log("{}", widget->name());
        const size_t child_count{ m_children.size() };
        std::erase(m_children, widget);

        runtime_assert(m_children.size() != child_count, "didn't find widget to delete");
        widget->release_ref();
    }

    void Widget::remove_child_at(const i32 index)
    {
        scoped_log();
        runtime_assert(index >= 0 && index < static_cast<i32>(m_children.size()),
                       "widget child remove idx out of bounds");

        const Widget* widget{ m_children[static_cast<std::size_t>(index)] };
        m_children.erase(m_children.begin() + index);
        widget->release_ref();
    }

    i32 Widget::child_index(const Widget* widget) const
    {
        scoped_log();
        const auto w{ std::ranges::find(m_children, widget) };
        if (w == m_children.end())
            return -1;

        return static_cast<i32>(w - m_children.begin());
    }

    bool Widget::enabled() const
    {
        scoped_log("enabled={}", m_enabled);
        return m_enabled;
    }

    void Widget::set_enabled(bool enabled)
    {
        scoped_log("enabled={}", enabled);
        m_enabled = enabled;
    }

    bool Widget::focused() const
    {
        scoped_logger(log_level::trace, "m_focused={}", m_focused);
        return m_focused;
    }

    void Widget::set_focused(bool focused)
    {
        scoped_log("focused={}", focused);
        m_focused = focused;
    }

    const std::string& Widget::tooltip() const
    {
        scoped_log("m_tooltip={}", m_tooltip);
        return m_tooltip;
    }

    void Widget::set_tooltip(const std::string& tooltip)
    {
        scoped_log("tooltip={}", tooltip);
        m_tooltip = tooltip;
    }

    void Widget::set_font_size(f32 font_size)
    {
        scoped_log("font_size={}", font_size);
        m_font_size = font_size;
    }

    bool Widget::has_font_size() const
    {
        scoped_logger(log_level::debug, "m_icon_extra_scale={}", m_icon_extra_scale);
        return m_font_size > 0;
    }

    f32 Widget::icon_extra_scale() const
    {
        scoped_log("m_icon_extra_scale={}", m_icon_extra_scale);
        return m_icon_extra_scale;
    }

    void Widget::set_icon_extra_scale(f32 scale)
    {
        scoped_log("scale={}", scale);
        m_icon_extra_scale = scale;
    }

    Mouse::Cursor::ID Widget::cursor() const
    {
        scoped_logger(log_level::debug, "{}", static_cast<i32>(m_cursor));
        return m_cursor;
    }

    void Widget::set_cursor(const Mouse::Cursor::ID cursor)
    {
        scoped_log("{}", static_cast<i32>(cursor));
        m_cursor = cursor;
    }

    bool Widget::contains(const ds::point<f32>& pt) const
    {
        // Check if the widget contains a certain position
        const ds::rect widget_rect{ m_pos, m_size };
        return widget_rect.contains(pt);
    }

    Canvas* Widget::canvas()
    {
        scoped_log();
        Widget* widget{ this };
        while (widget != nullptr)
        {
            Canvas* canvas{ dynamic_cast<Canvas*>(widget) };
            if (canvas != nullptr)
                return canvas;

            widget = widget->parent();
        }

        assert_msg("failed to get GUI canvas that owns widget");
        return nullptr;
    }

    Dialog* Widget::dialog()
    {
        scoped_log();
        Widget* widget{ this };
        while (widget != nullptr)
        {
            const auto dialog{ dynamic_cast<Dialog*>(widget) };
            if (dialog != nullptr)
                return dialog;

            widget = widget->parent();
        }

        runtime_assert(false, "failed to get dialog that owns widget");
        return nullptr;
    }

    const Canvas* Widget::canvas() const
    {
        scoped_log();
        return const_cast<Widget*>(this)->canvas();
    }

    const Dialog* Widget::dialog() const
    {
        scoped_log();
        return const_cast<Widget*>(this)->dialog();
    }

    void Widget::request_focus()
    {
        scoped_log();
        Widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        dynamic_cast<Canvas*>(widget)->update_focus(this);
    }

    void Widget::draw_mouse_intersection(const ds::point<f32>& pt)
    {
        scoped_trace(log_level::trace);

        {
            LocalTransform transform{ this };
            for (const auto child : std::ranges::reverse_view{ m_children })
            {
                if (!child->visible())
                    continue;

                child->draw_mouse_intersection(pt - m_pos);
            }
        }

        if (this->contains(pt))
        {
            const ds::rect widget_rect{
                m_pos,
                m_size,
            };

            m_renderer->draw_rect_outline(widget_rect, 1.0f, rl::Colors::Yellow, Outline::Inner);
        }
    }

    void Widget::draw()
    {
        scoped_trace(log_level::trace);
        if constexpr (Widget::DiagnosticsEnabled)
        {
            m_renderer->draw_rect_outline(ds::rect{ m_pos, m_size }, 1.0f, rl::Colors::Grey,
                                          Outline::Outer);
        }

        if (m_children.empty())
            return;

        auto&& context{ m_renderer->context() };

        LocalTransform transform{ this };
        for (auto child : m_children)
        {
            if (!child->visible())
                continue;

            m_renderer->scoped_draw([&]() {
                nvg::intersect_scissor(context, child->m_pos.x, child->m_pos.y, child->m_size.width,
                                       child->m_size.height);
                child->draw();
            });
        }
    }

    f32 Widget::icon_scale() const
    {
        runtime_assert(m_theme != nullptr, "theme not set");
        return m_theme->icon_scale * m_icon_extra_scale;
    }

    std::string Widget::name() const
    {
        return typeid(*this).name();
    }
}
