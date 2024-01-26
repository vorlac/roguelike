#pragma once

#include <algorithm>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"
#include "graphics/nvg_renderer.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

namespace rl::ui {
    class Dialog;
    class UICanvas;

    class Widget : public ds::refcounted
    {
    private:
        friend class UICanvas;
        Widget(Widget* parent, const std::unique_ptr<NVGRenderer>& vg_renderer);

    public:
        Widget(Widget* parent);

        virtual ~Widget();

        bool show();
        bool hide();
        bool visible() const;
        bool visible_recursive() const;
        bool has_font_size() const;
        bool enabled() const;
        bool focused() const;
        bool contains(ds::point<f32> pt) const;

        f32 width() const;
        f32 height() const;
        f32 fixed_width() const;
        f32 fixed_height() const;
        f32 font_size() const;
        f32 icon_extra_scale() const;
        i32 child_index(Widget* widget) const;
        i32 child_count() const;

        UICanvas* canvas();
        Dialog* dialog();
        Widget* parent();
        Layout* layout();
        Theme* theme();
        Widget* child_at(i32 index);
        rl::Mouse::Cursor::ID cursor() const;
        Widget* find_widget(ds::point<f32> pt);
        ds::point<f32> position() const;
        ds::point<f32> abs_position() const;
        const ds::dims<f32>& fixed_size() const;
        const ds::dims<f32>& size() const;

        const UICanvas* canvas() const;
        const Dialog* dialog() const;
        const Widget* parent() const;
        const Layout* layout() const;
        const Theme* theme() const;
        const Widget* child_at(i32 index) const;
        const Widget* find_widget(ds::point<f32> pt) const;
        const std::vector<Widget*>& children() const;
        const std::string& tooltip() const;

        void request_focus();
        void remove_child_at(i32 index);
        void remove_child(const Widget* widget);
        void add_child(Widget* widget);

        void set_parent(Widget* parent);
        void set_layout(Layout* layout);
        void set_position(ds::point<f32> pos);
        void set_size(ds::dims<f32> size);
        void set_width(f32 width);
        void set_height(f32 height);
        void set_fixed_size(ds::dims<f32> fixed_size);
        void set_fixed_width(f32 width);
        void set_fixed_height(f32 height);
        void set_visible(bool visible);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(const std::string& tooltip);
        void set_font_size(f32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);

    public:
        virtual bool on_focus_gained();
        virtual bool on_focus_lost();

        virtual bool on_key_pressed(const Keyboard& kb);
        virtual bool on_key_released(const Keyboard& kb);
        virtual bool on_character_input(const Keyboard& kb);

        virtual bool on_mouse_entered(const Mouse& mouse);
        virtual bool on_mouse_exited(const Mouse& mouse);
        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb);

        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb);
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb);

        virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb);
        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb);

    public:
        virtual void draw();
        virtual void set_theme(Theme* theme);
        virtual void add_child(i32 index, Widget* widget);
        virtual void perform_layout();
        virtual ds::dims<f32> preferred_size() const;

    public:
        virtual void draw_mouse_intersection( ds::point<f32> pt);

    protected:
        f32 icon_scale() const;

    protected:
        Widget* m_parent{ nullptr };
        ds::shared<Theme> m_theme{ nullptr };
        ds::shared<Layout> m_layout{ nullptr };
        static inline rl::NVGRenderer* m_renderer{ nullptr };

        bool m_enabled{ true };
        bool m_visible{ true };
        bool m_focused{ false };
        bool m_mouse_focus{ false };

        f32 m_font_size{ 16.0 };
        f32 m_icon_extra_scale{ 1.0f };

        ds::point<f32> m_pos{ 0.0f, 0.0f };
        ds::dims<f32> m_size{ 0.0f, 0.0f };
        ds::dims<f32> m_fixed_size{ 0.0f, 0.0f };
        ds::dims<f32> m_framebuf_size{ 0.0f, 0.0f };

        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };
        std::vector<Widget*> m_children{};
        std::string m_tooltip{ "" };

        mutable Timer<f32> m_timer{};

    private:
        constexpr static bool DiagnosticsEnabled{ false };
    };
}
