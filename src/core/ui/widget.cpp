#include <ranges>

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/layouts/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widgets/dialog.hpp"
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
        scoped_trace(log_level::debug);
        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::Widget(Widget* parent, const std::unique_ptr<NVGRenderer>& vg_renderer)
        : m_parent{ parent }
    {
        scoped_trace(log_level::debug);
        runtime_assert(m_renderer == nullptr, "widget vectorized renderer already set");
        if (m_renderer == nullptr)
            m_renderer = vg_renderer.get();

        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::~Widget()
    {
        scoped_trace(log_level::debug);
        for (const auto child : m_children)
            if (child != nullptr)
                child->release_ref();
    }

    Widget* Widget::parent()
    {
        scoped_trace(log_level::trace);
        return m_parent;
    }

    const Widget* Widget::parent() const
    {
        scoped_trace(log_level::debug);
        return m_parent;
    }

    void Widget::set_parent(Widget* parent)
    {
        scoped_trace(log_level::debug);
        m_parent = parent;
    }

    Layout* Widget::layout()
    {
        scoped_trace(log_level::debug);
        return m_layout;
    }

    const Layout* Widget::layout() const
    {
        scoped_trace(log_level::debug);
        return m_layout.get();
    }

    void Widget::set_layout(Layout* layout)
    {
        scoped_trace(log_level::debug);
        m_layout = layout;
    }

    Theme* Widget::theme()
    {
        scoped_trace(log_level::trace);
        return m_theme;
    }

    const Theme* Widget::theme() const
    {
        scoped_trace(log_level::trace);
        return m_theme.get();
    }

    void Widget::set_theme(Theme* theme)
    {
        scoped_trace(log_level::debug);
        if (m_theme.get() == theme)
            return;

        m_theme = theme;
        for (const auto child : m_children)
            child->set_theme(theme);
    }

    const ds::point<f32>& Widget::position() const
    {
        return m_rect.pt;
    }

    void Widget::set_position(ds::point<f32>&& pos) noexcept
    {
        scoped_trace(log_level::debug);
        m_rect.pt = std::move(pos);
    }

    void Widget::set_rect(ds::rect<f32>&& rect) noexcept
    {
        scoped_trace(log_level::debug);
        m_rect = std::move(rect);
    }

    ds::point<f32> Widget::abs_position() const
    {
        scoped_trace(log_level::debug);
        return m_parent != nullptr  //
                 ? m_parent->abs_position() + m_rect.pt
                 : m_rect.pt;
    }

    const ds::dims<f32>& Widget::size() const
    {
        return m_rect.size;
    }

    const ds::rect<f32>& Widget::rect() const
    {
        return m_rect;
    }

    void Widget::set_size(ds::dims<f32>&& size)
    {
        m_rect.size = std::move(size);
    }

    f32 Widget::width() const
    {
        return m_rect.size.width;
    }

    void Widget::set_width(const f32 width)
    {
        scoped_log("width={}", width);
        m_rect.size.width = width;
    }

    f32 Widget::height() const
    {
        return m_rect.size.height;
    }

    void Widget::set_height(const f32 height)
    {
        scoped_log("height={}", height);
        m_rect.size.height = height;
    }

    void Widget::set_fixed_size(const ds::dims<f32>& fixed_size)
    {
        scoped_log("fixed_size={}", fixed_size);
        m_fixed_size = fixed_size;
    }

    const ds::dims<f32>& Widget::fixed_size() const
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
        scoped_log("fixed_width={}", width);
        m_fixed_size.width = width;
    }

    void Widget::set_fixed_height(f32 height)
    {
        scoped_log("fixed_height={}", height);
        m_fixed_size.height = height;
    }

    bool Widget::visible() const
    {
        scoped_logger(log_level::trace, "m_visible={}", m_visible);
        return m_visible;
    }

    void Widget::set_visible(bool visible)
    {
        scoped_logger(log_level::trace, "visible={}", visible);
        m_visible = visible;
    }

    void Widget::show()
    {
        this->set_visible(true);
    }

    void Widget::hide()
    {
        this->set_visible(false);
    }

    bool Widget::visible_recursive() const
    {
        scoped_trace(log_level::debug);
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
        return static_cast<i32>(m_children.size());
    }

    Widget* Widget::child_at(const i32 index)
    {
        return m_children[index];
    }

    const Widget* Widget::child_at(const i32 index) const
    {
        return m_children[index];
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
        const auto context{ m_renderer->context() };
        auto ret{ (m_layout != nullptr ? m_layout->preferred_size(context, this) : m_rect.size) };
        scoped_logger(log_level::trace, "{}", ret);
        return ret;
    }

    void Widget::perform_layout()
    {
        scoped_trace(log_level::trace);

        for (const auto child : m_children)
        {
            auto ps{ child->preferred_size() };
            auto fs{ child->fixed_size() };

            child->set_size({
                fs.width == 0.0f ? ps.width : fs.width,
                fs.height == 0.0f ? ps.height : fs.height,
            });

            child->perform_layout();
        }

        if (m_layout != nullptr)
        {
            const auto context{ m_renderer->context() };
            m_layout->perform_layout(context, this);
        }
    }

    Widget* Widget::find_widget(const ds::point<f32>& pt)
    {
        {
            LocalTransform transform{ this };
            const ds::point local_mouse_pos{ pt - m_rect.pt };
            for (auto&& child : std::ranges::reverse_view{ m_children })
            {
                if (!child->visible())
                    continue;
                if (child->resizable() && child->resize_rect().contains(pt - m_rect.pt))
                {
                    // if the child is resizable and the larger resize rect (for grab points)
                    // contains the mouse, but the smaller inner rect doesn't then favor resizing
                    // over recursively going deeper into the tree of widgets for more children
                    if (!child->rect().expanded(-RESIZE_GRAB_BUFFER).contains(pt - m_rect.pt))
                        return child;

                    // otherwise continue searching for a better match
                    return child->find_widget(local_mouse_pos);
                }

                // recurse deeper if the child rect contains the cursor
                if (child->contains(pt - m_rect.pt))
                    return child->find_widget(local_mouse_pos);
            }
        }

        return this->contains(pt) ? this : nullptr;
    }

    bool Widget::on_mouse_entered(const Mouse& mouse)
    {
        m_mouse_focus = true;
        return false;
    }

    bool Widget::on_mouse_exited(const Mouse& mouse)
    {
        m_mouse_focus = false;
        return false;
    }

    bool Widget::on_focus_gained()
    {
        m_focused = true;
        return false;
    }

    bool Widget::on_focus_lost()
    {
        m_focused = false;
        return false;
    }

    bool Widget::on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb)
    {
        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(local_mouse_pos))
                continue;
            if (!child->on_mouse_button_pressed(mouse, kb))
                continue;

            return true;
        }

        if (!m_focused && mouse.is_button_pressed(Mouse::Button::Left))
            this->request_focus();

        return false;
    }

    bool Widget::on_mouse_button_released(const Mouse& mouse, const Keyboard& kb)
    {
        scoped_log("btn={}", mouse.button_released());

        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(local_mouse_pos))
                continue;
            if (!child->on_mouse_button_released(mouse, kb))
                continue;

            return true;
        }

        return false;
    }

    bool Widget::on_mouse_scroll(const Mouse& mouse, const Keyboard& kb)
    {
        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
        for (auto&& child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(local_mouse_pos))
                continue;
            if (!child->on_mouse_scroll(mouse, kb))
                continue;

            return true;
        }

        return false;
    }

    bool Widget::on_mouse_move(const Mouse& mouse, const Keyboard& kb)
    {
        bool handled{ false };

        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
        for (auto&& child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;

            const bool contained{ child->contains(local_mouse_pos) };
            const bool prev_contained{ child->contains(local_mouse_pos - mouse.pos_delta()) };

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
        // do nothing, derived classess
        // should implement/override
        return false;
    }

    bool Widget::on_key_pressed(const Keyboard& kb)
    {
        // do nothing, derived classess
        // should implement/override
        return false;
    }

    bool Widget::on_key_released(const Keyboard& kb)
    {
        // do nothing, derived classess
        // should implement/override
        return false;
    }

    bool Widget::on_character_input(const Keyboard& kb)
    {
        // do nothing, derived classess
        // should implement/override
        return false;
    }

    void Widget::add_child(const i32 index, Widget* widget)
    {
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
        const std::size_t child_count{ m_children.size() };
        std::erase(m_children, widget);

        runtime_assert(m_children.size() != child_count, "didn't find widget to delete");
        widget->release_ref();
    }

    void Widget::remove_child_at(const i32 index)
    {
        runtime_assert(index >= 0 && index < m_children.size(),
                       "widget child remove idx out of bounds");

        const Widget* widget{ m_children[index] };
        m_children.erase(m_children.begin() + index);
        widget->release_ref();
    }

    i32 Widget::child_index(const Widget* widget) const
    {
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

    bool Widget::resizable() const
    {
        return m_resizable;
    }

    void Widget::set_focused(bool focused)
    {
        scoped_log("focused={}", focused);
        m_focused = focused;
    }

    const std::string& Widget::tooltip() const
    {
        scoped_logger(log_level::debug, "m_tooltip={}", m_tooltip);
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
        scoped_logger(log_level::debug, "{}", std::to_underlying(m_cursor));
        return m_cursor;
    }

    ds::rect<f32> Widget::resize_rect() const
    {
        return m_rect.expanded(RESIZE_GRAB_BUFFER);
    }

    void Widget::set_cursor(const Mouse::Cursor::ID cursor)
    {
        scoped_log("{}", static_cast<i32>(cursor));
        m_cursor = cursor;
    }

    bool Widget::contains(const ds::point<f32>& pt)
    {
        // Check if the widget contains a certain position
        const ds::rect widget_rect{ m_rect.pt, m_rect.size };
        return widget_rect.contains(pt);
    }

    Canvas* Widget::canvas()
    {
        scoped_trace(log_level::trace);
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

    ScrollableDialog* Widget::dialog()
    {
        scoped_trace(log_level::trace);

        Widget* widget{ this };
        while (widget != nullptr)
        {
            auto dialog{ dynamic_cast<ScrollableDialog*>(widget) };
            if (dialog != nullptr)
                return dialog;

            widget = widget->parent();
        }

        runtime_assert(false, "failed to get dialog that owns widget");
        return nullptr;
    }

    const Canvas* Widget::canvas() const
    {
        scoped_trace(log_level::trace);
        return const_cast<Widget*>(this)->canvas();
    }

    const ScrollableDialog* Widget::dialog() const
    {
        scoped_trace(log_level::trace);
        return const_cast<Widget*>(this)->dialog();
    }

    void Widget::request_focus()
    {
        Widget* widget{ this };
        while (widget->parent() != nullptr)
            widget = widget->parent();

        Canvas* canvas{ dynamic_cast<Canvas*>(widget) };
        runtime_assert(canvas != nullptr, "failed to get top level UI canvas");
        canvas->update_focus(this);
    }

    bool Widget::draw_mouse_intersection(const ds::point<f32>& pt)
    {
        scoped_trace(log_level::trace);

        if (this->contains(pt))
        {
            const ds::rect widget_rect{ m_rect.pt, m_rect.size };
            m_renderer->draw_rect_outline(widget_rect, 1.0f, rl::Colors::Yellow, Outline::Inner);
        }

        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ pt - m_rect.pt };
        for (const auto child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(local_mouse_pos))
                continue;
            if (!child->draw_mouse_intersection(local_mouse_pos))
                continue;

            m_renderer->draw_rect_outline(this->rect(), 1.0f, rl::Colors::Yellow, Outline::Inner);

            diag_log("dgb outline: handled={}", this->name());
            diag_log("dgb outline: rel_pos={}", ds::rect{ m_rect.pt, m_rect.size });
            diag_log("dgb outline: abs_pos={}", ds::rect{ this->abs_position(), m_rect.size });
            return true;
        }

        return false;
    }

    void Widget::draw()
    {
        scoped_trace(log_level::trace);

        if constexpr (Widget::DiagnosticsEnabled)
        {
            m_renderer->draw_rect_outline(ds::rect{ m_rect.pt, m_rect.size }, 1.0f,
                                          rl::Colors::Grey, Outline::Outer);
        }

        if (m_children.empty())
            return;

        LocalTransform transform{ this };
        for (auto child : m_children)
        {
            if (!child->visible())
                continue;

            m_renderer->scoped_draw([&] {
                // TODO: put this back after fixing popup window
                // nvg::intersect_scissor(context, child->m_rect.pt.x, child->m_rect.pt.y,
                // child->m_rect.size.width, child->m_rect.size.height);
                child->draw();
            });
        }
    }

    f32 Widget::icon_scale() const
    {
        runtime_assert(m_theme != nullptr, "theme not set");
        return m_theme->icon_scale * m_icon_extra_scale;
    }

    std::string_view Widget::name() const
    {
        return typeid(*this).name();
    }
}
