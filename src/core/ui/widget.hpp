#pragma once

#include <algorithm>
#include <vector>

#include "core/mouse.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"

#if _MSC_VER
  #pragma warning(disable : 4244)
#endif

namespace rl {
    class Mouse;
    class Window;
    using WindowID = u32;
}

namespace rl::ui {

    class widget : public ds::refcounted
    {
    public:
        // Construct a new widget
        // with the given parent
        widget(widget* parent);

        // Free all resources used by
        // the widget and any children
        virtual ~widget();

        widget* parent()
        {
            return m_parent;
        }

        const widget* parent() const
        {
            return m_parent;
        }

        void set_parent(widget* parent)
        {
            m_parent = parent;
        }

        ui::layout* layout()
        {
            return m_layout;
        }

        const ui::layout* layout() const
        {
            return m_layout.get();
        }

        void set_layout(ui::layout* layout)
        {
            m_layout = layout;
        }

        ui::theme* theme()
        {
            return m_theme;
        }

        const ui::theme* theme() const
        {
            return m_theme.get();
        }

        virtual void set_theme(ui::theme* theme)
        {
            if (m_theme.get() == theme)
                return;

            m_theme = theme;
            for (auto&& child : m_children)
                child->set_theme(theme);
        }

        ds::point<i32> position() const
        {
            return m_pos;
        }

        void set_position(ds::point<i32> pos)
        {
            m_pos = pos;
        }

        ds::point<i32> abs_position() const
        {
            // Return the position in absolute screen coords
            return m_parent != nullptr                   //
                     ? m_parent->abs_position() + m_pos  //
                     : m_pos;                            //
        }

        ds::dims<i32> size() const
        {
            return m_size;
        }

        void set_size(const ds::dims<i32>& size)
        {
            m_size = size;
        }

        i32 width() const
        {
            return m_size.width;
        }

        void set_width(i32 width)
        {
            m_size.width = width;
        }

        i32 height() const
        {
            return m_size.height;
        }

        void set_height(i32 height)
        {
            m_size.height = height;
        }

        void set_fixed_size(ds::dims<i32> fixed_size)
        {
            m_fixed_size = fixed_size;
        }

        ds::dims<i32> fixed_size() const
        {
            return m_fixed_size;
        }

        i32 fixed_width() const
        {
            return m_fixed_size.width;
        }

        i32 fixed_height() const
        {
            return m_fixed_size.height;
        }

        void set_fixed_width(i32 width)
        {
            m_fixed_size.width = width;
        }

        void set_fixed_height(i32 height)
        {
            m_fixed_size.height = height;
        }

        bool visible() const
        {
            return m_visible;
        }

        void set_visible(bool visible)
        {
            m_visible = visible;
        }

        void show()
        {
            this->set_visible(true);
        }

        void hide()
        {
            this->set_visible(false);
        }

        bool visible_recursive() const
        {
            bool visible = true;
            const widget* widget = this;
            while (widget)
            {
                visible &= widget->visible();
                widget = widget->parent();
            }
            return visible;
        }

        int child_count() const
        {
            return (int)m_children.size();
        }

        const std::vector<widget*>& children() const
        {
            return m_children;
        }

        virtual void add_child(int index, widget* widget);

        void add_child(widget* widget);
        void remove_child_at(int index);
        void remove_child(const widget* widget);

        const widget* child_at(int index) const
        {
            return m_children[(size_t)index];
        }

        widget* child_at(int index)
        {
            return m_children[(size_t)index];
        }

        int child_index(widget* widget) const;

        template <typename widgetClass, typename... Args>
        widgetClass* add(const Args&... args)
        {
            return new widgetClass(this, args...);
        }

        rl::Window* window();
        const rl::Window* window() const;

        bool enabled() const
        {
            return m_enabled;
        }

        void set_enabled(bool enabled)
        {
            m_enabled = enabled;
        }

        bool focused() const
        {
            return m_focused;
        }

        void set_focused(bool focused)
        {
            m_focused = focused;
        }

        void request_focus();

        const std::string& tooltip() const
        {
            return m_tooltip;
        }

        void set_tooltip(const std::string& tooltip)
        {
            m_tooltip = tooltip;
        }

        int font_size() const;

        void set_font_size(int font_size)
        {
            m_font_size = font_size;
        }

        bool has_font_size() const
        {
            return m_font_size > 0;
        }

        float icon_extra_scale() const
        {
            return m_icon_extra_scale;
        }

        void set_icon_extra_scale(float scale)
        {
            m_icon_extra_scale = scale;
        }

        Mouse::Cursor::ID cursor() const
        {
            return m_cursor;
        }

        void set_cursor(Mouse::Cursor::ID cursor)
        {
            m_cursor = cursor;
        }

        bool contains(ds::point<i32> pt) const
        {
            // Check if the widget contains a certain position
            const bool contains_pt{ ds::rect<i32>{ m_pos, m_size }.contains(pt) };
            return contains_pt;
        }

        widget* find_widget(ds::point<i32> pt);
        const widget* find_widget(ds::point<i32> pt) const;
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const;
        virtual void perform_layout(NVGcontext* nvg_context);
        virtual void draw(NVGcontext* nvg_context);

        virtual bool on_kb_character_input(const WindowID id);
        virtual bool on_kb_key_pressed(const WindowID id);
        virtual bool on_kb_focus_gained(const WindowID id);
        virtual bool on_kb_focus_lost(const WindowID id);
        virtual bool on_mouse_enter(const WindowID id);
        virtual bool on_mouse_leave(const WindowID id);
        virtual bool on_mouse_click(const WindowID id);
        virtual bool on_mouse_move(const WindowID id);
        virtual bool on_mouse_drag(const WindowID id);
        virtual bool on_mouse_scroll(const WindowID id);

    protected:
        float icon_scale() const
        {
            return m_theme->m_icon_scale * m_icon_extra_scale;
        }

    protected:
        widget* m_parent{ nullptr };
        ds::shared<ui::theme> m_theme{ nullptr };
        ds::shared<ui::layout> m_layout{ nullptr };
        ds::point<i32> m_pos{ 0, 0 };
        ds::dims<i32> m_size{ 0, 0 };
        ds::dims<i32> m_fixed_size{ 0, 0 };
        std::vector<widget*> m_children{};
        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::ID::Arrow };
        std::string m_tooltip{};
        i32 m_font_size{ 16 };

        bool m_enabled{ true };
        bool m_focused{ false };
        bool m_mouse_focus{ false };
        bool m_visible{ true };
        f32 m_icon_extra_scale{ 1.0f };

        NVGcontext* m_nvg_context{ nullptr };

        // Enables diagnostic rendering that displays widget bounds
        constexpr static bool DiagnosticsEnabled = false;
    };

}
