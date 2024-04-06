#include <memory>
#include <ranges>
#include <utility>

#include "core/keyboard.hpp"
#include "core/main_window.hpp"
#include "core/mouse.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/layouts/layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widget.hpp"
#include "core/ui/widgets/scroll_dialog.hpp"
#include "ds/shared.hpp"
#include "graphics/nvg_renderer.hpp"
#include "graphics/vg/nanovg_state.hpp"

namespace rl::ui {

    Widget::Widget(Widget* parent)
        : m_parent{ parent }
    {
        if (m_theme == nullptr)
            m_theme = new Theme{};
        if (parent != nullptr)
            parent->add_child(this);
    }

    Widget::Widget(Widget* parent, const std::unique_ptr<NVGRenderer>& vg_renderer)
        : Widget{ parent }
    {
        runtime_assert(m_renderer == nullptr, "widget vectorized renderer already set");
        if (m_renderer == nullptr)
            m_renderer = vg_renderer.get();
    }

    Widget::~Widget()
    {
        for (const auto child : m_children)
            if (child != nullptr)
                child->release_ref();

        // if (m_layout != nullptr)
        //     m_layout->release_ref();
    }

    Widget* Widget::parent()
    {
        return m_parent;
    }

    const Widget* Widget::parent() const
    {
        return m_parent;
    }

    void Widget::set_parent(Widget* parent)
    {
        m_parent = parent;
    }

    Layout* Widget::layout()
    {
        return m_layout;
    }

    void Widget::assign_layout(Layout* layout)
    {
        runtime_assert(m_layout == nullptr, "overwriting existing layout");
        this->add_child(layout);
    }

    const Theme* Widget::theme() const
    {
        return m_theme;
    }

    void Widget::set_theme(Theme* theme)
    {
        if (m_theme == theme)
            return;

        m_theme = theme;
        for (Widget* child : m_children)
            child->set_theme(theme);
    }

    ds::point<f32> Widget::position() const
    {
        return m_rect.pt;
    }

    void Widget::set_position(const ds::point<f32> pos)
    {
        m_rect.pt = pos;
    }

    void Widget::set_rect(ds::rect<f32> rect)
    {
        m_rect = std::move(rect);
    }

    ds::point<f32> Widget::abs_position() const
    {
        return m_parent != nullptr  //
                 ? m_parent->abs_position() + m_rect.pt
                 : m_rect.pt;
    }

    ds::dims<f32> Widget::size() const
    {
        return m_rect.size;
    }

    const ds::rect<f32>& Widget::rect() const
    {
        return m_rect;
    }

    f32 Widget::width() const
    {
        return m_rect.size.width;
    }

    void Widget::set_width(const f32 width)
    {
        m_rect.size.width = width;
    }

    f32 Widget::height() const
    {
        return m_rect.size.height;
    }

    void Widget::set_height(const f32 height)
    {
        m_rect.size.height = height;
    }

    void Widget::set_fixed_size(const ds::dims<f32> fixed_size)
    {
        m_fixed_size = fixed_size;
    }

    ds::dims<f32> Widget::fixed_size() const
    {
        return m_fixed_size;
    }

    ds::dims<f32> Widget::min_size() const
    {
        return m_min_size;
    }

    ds::dims<f32> Widget::max_size() const
    {
        return m_max_size;
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
        m_fixed_size.width = width;
    }

    void Widget::set_fixed_height(const f32 height)
    {
        m_fixed_size.height = height;
    }

    void Widget::set_size(const ds::dims<f32> size)
    {
        m_rect.size = size;
    }

    bool Widget::visible() const
    {
        return m_visible;
    }

    void Widget::set_visible(const bool visible)
    {
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
        bool visible{ true };

        const Widget* widget{ this };
        while (widget != nullptr)
        {
            visible &= widget->visible();
            widget = widget->parent();
        }

        return visible;
    }

    u64 Widget::child_count() const
    {
        return m_children.size();
    }

    Widget* Widget::child_at(const u64 index)
    {
        return m_children[index];
    }

    const Widget* Widget::child_at(const u64 index) const
    {
        return m_children[index];
    }

    const std::vector<Widget*>& Widget::children() const
    {
        return m_children;
    }

    f32 Widget::font_size() const
    {
        return m_theme != nullptr && math::equal(m_font_size, -1.0f)  //
                 ? m_theme->standard_font_size                        //
                 : m_font_size;
    }

    ds::dims<f32> Widget::preferred_size() const
    {
        return m_layout != nullptr            //
                 ? m_layout->computed_size()  //
                 : m_rect.size;
    }

    void Widget::perform_layout()
    {
        if (m_parent != nullptr)
        {
            const ds::dims<f32>& widget_upper_limit_size{
                m_parent->max_size() == ds::dims<f32>::null()  //
                    ? m_parent->size()                         //
                    : m_parent->max_size()                     //
            };

            ds::dims max_size{ widget_upper_limit_size - m_outer_margin };
            if (m_layout != nullptr && m_layout->size_policy() == SizePolicy::Prefered)
            {
                Alignment alignment{ m_layout->alignment() };
                if (alignment == Alignment::Horizontal)
                    max_size.width /= static_cast<f32>(m_children.size());
                else if (alignment == Alignment::Vertical)
                    max_size.height /= static_cast<f32>(m_children.size());
            }
            this->set_max_size(max_size);
        }

        for (Widget* child : m_children)
            child->perform_layout();

        if (m_layout != nullptr)
            m_layout->apply_layout();
    }

    Widget* Widget::find_widget(const ds::point<f32> pt)
    {
        {
            LocalTransform transform{ this };
            const ds::point local_mouse_pos{ pt - m_rect.pt };
            for (Widget* child : std::ranges::reverse_view{ m_children })
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
        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ mouse.pos() - LocalTransform::absolute_pos };
        for (Widget* child : std::ranges::reverse_view{ m_children })
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
        for (Widget* child : std::ranges::reverse_view{ m_children })
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
        for (Widget* child : std::ranges::reverse_view{ m_children })
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

    void Widget::add_child(const u64 index, Widget* widget)
    {
        runtime_assert(index <= this->child_count(), "child widget index out of bounds");
        m_children.insert(m_children.begin() + static_cast<i32>(index), widget);

        widget->acquire_ref();
        widget->set_parent(this);
        widget->set_theme(m_theme);
    }

    void Widget::add_child(Widget* widget)
    {
        this->add_child(this->child_count(), widget);
    }

    void Widget::remove_child(const Widget* widget)
    {
        const std::size_t child_count{ m_children.size() };
        std::erase(m_children, widget);

        runtime_assert(m_children.size() != child_count, "didn't find widget to delete");
        widget->release_ref();
    }

    void Widget::remove_child_at(const u64 index)
    {
        runtime_assert(index < m_children.size(), "widget child remove idx out of bounds");

        Widget* widget{ m_children[index] };
        m_children.erase(m_children.begin() + static_cast<ptrdiff_t>(index));
        widget->release_ref();
    }

    u64 Widget::child_index(const Widget* widget) const
    {
        auto w{ std::ranges::find(m_children, widget) };
        if (w == m_children.end())
            return std::numeric_limits<u64>::max();

        return static_cast<u64>(w - m_children.begin());
    }

    bool Widget::enabled() const
    {
        return m_enabled;
    }

    void Widget::set_enabled(const bool enabled)
    {
        m_enabled = enabled;
    }

    bool Widget::focused() const
    {
        return m_focused;
    }

    bool Widget::resizable() const
    {
        return m_resizable;
    }

    void Widget::set_focused(const bool focused)
    {
        m_focused = focused;
    }

    const std::string& Widget::tooltip() const
    {
        return m_tooltip;
    }

    void Widget::set_tooltip(const std::string& tooltip)
    {
        m_tooltip = tooltip;
    }

    const std::string& Widget::name() const
    {
        return m_name;
    }

    void Widget::set_name(const std::string& name)
    {
        m_name = name;
    }

    void Widget::set_font_size(const f32 font_size)
    {
        m_font_size = font_size;
    }

    bool Widget::has_font_size() const
    {
        return m_font_size > 0;
    }

    f32 Widget::icon_extra_scale() const
    {
        return m_icon_extra_scale;
    }

    void Widget::set_icon_extra_scale(const f32 scale)
    {
        m_icon_extra_scale = scale;
    }

    Mouse::Cursor::ID Widget::cursor() const
    {
        return m_cursor;
    }

    ds::rect<f32> Widget::resize_rect() const
    {
        return m_rect.expanded(RESIZE_GRAB_BUFFER);
    }

    void Widget::set_cursor(const Mouse::Cursor::ID cursor)
    {
        m_cursor = cursor;
    }

    void Widget::set_min_size(const ds::dims<f32> min_size)
    {
        m_min_size = min_size;
    }

    void Widget::set_max_size(const ds::dims<f32> max_size)
    {
        m_max_size = max_size;
    }

    bool Widget::contains(const ds::point<f32> pt)
    {
        // Check if the widget contains a certain position
        const ds::rect widget_rect{ m_rect.pt, m_rect.size };
        return widget_rect.contains(pt);
    }

    Canvas* Widget::canvas()
    {
        Widget* widget{ this };
        while (widget != nullptr)
        {
            Canvas* canvas{ dynamic_cast<Canvas*>(widget) };
            if (canvas != nullptr)
                return canvas;

            widget = widget->parent();
        }

        return nullptr;
    }

    ScrollableDialog* Widget::dialog()
    {
        Widget* widget{ this };
        while (widget != nullptr)
        {
            auto dialog{ dynamic_cast<ScrollableDialog*>(widget) };
            if (dialog != nullptr)
                return dialog;

            widget = widget->parent();
        }

        return nullptr;
    }

    const Canvas* Widget::canvas() const
    {
        return const_cast<Widget*>(this)->canvas();
    }

    const ScrollableDialog* Widget::dialog() const
    {
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

    bool Widget::draw_mouse_intersection(const ds::point<f32> pt)
    {
        if (this->contains(pt))
        {
            const ds::rect widget_rect{ m_rect.pt, m_rect.size };
            m_renderer->draw_rect_outline(widget_rect, 1.0f, rl::Colors::Yellow, Outline::Inner);
        }

        LocalTransform transform{ this };
        const ds::point local_mouse_pos{ pt - m_rect.pt };
        for (Widget* child : std::ranges::reverse_view{ m_children })
        {
            if (!child->visible())
                continue;
            if (!child->contains(local_mouse_pos))
                continue;
            if (!child->draw_mouse_intersection(local_mouse_pos))
                continue;

            m_renderer->draw_rect_outline(m_rect, 1.0f, rl::Colors::Yellow, Outline::Inner);
            return true;
        }

        return false;
    }

    void Widget::draw()
    {
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

            m_renderer->scoped_draw([child] {
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
}
