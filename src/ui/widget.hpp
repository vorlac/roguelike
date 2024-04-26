#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"
#include "graphics/nvg_renderer.hpp"
#include "utils/time.hpp"

namespace rl::ui {
    class ScrollableDialog;
    class Canvas;
    class Layout;

    class Widget
    {
    private:
        friend class Canvas;
        explicit Widget(Widget* parent, const std::unique_ptr<NVGRenderer>& vg_renderer);

    public:
        Widget() = delete;
        Widget(Widget&& other) = delete;
        Widget(const Widget& other) = delete;
        Widget& operator=(const Widget& other) = delete;
        Widget& operator=(Widget&& other) noexcept = delete;

    public:
        explicit Widget(Widget* parent);
        virtual ~Widget();

        void show();
        void hide();
        bool visible() const;
        bool visible_recursive() const;
        bool has_font_size() const;
        bool enabled() const;
        bool focused() const;
        bool resizable() const;
        bool recalc_needed() const;

        f32 width() const;
        f32 height() const;
        f32 fixed_width() const;
        f32 fixed_height() const;
        f32 font_size() const;
        f32 icon_extra_scale() const;
        u64 child_index(const Widget* widget) const;
        u64 child_count() const;

        Canvas* canvas();
        Widget* parent();
        Layout* layout() const;
        ScrollableDialog* dialog();

        Widget* child_at(u64 index);
        Mouse::Cursor::ID cursor() const;
        ds::rect<f32> resize_rect() const;
        ds::point<f32> abs_position() const;

        ds::point<f32> position() const;
        ds::dims<f32> fixed_size() const;
        ds::dims<f32> min_size() const;
        ds::dims<f32> max_size() const;
        ds::dims<f32> size() const;
        const ds::rect<f32>& rect() const;
        const Theme* theme() const;
        const Canvas* canvas() const;
        const ScrollableDialog* dialog() const;
        const Widget* parent() const;
        const Widget* child_at(u64 index) const;
        const std::vector<Widget*>& children() const;
        const std::string& tooltip() const;
        const std::string& name() const;

        void assign_layout(Layout* layout);
        void set_parent(Widget* parent);
        void set_position(ds::point<f32> pos);
        void set_rect(const ds::rect<f32>& rect);
        void set_width(f32 width);
        void set_height(f32 height);
        void set_fixed_size(ds::dims<f32> fixed_size);
        void set_fixed_width(f32 width);
        void set_fixed_height(f32 height);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(const std::string& tooltip);
        void set_name(const std::string& name);
        void set_font_size(f32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);
        void set_max_size(ds::dims<f32> size);
        void set_recalc_needed(bool size_recalc_needed, bool recursive = true);
        void request_focus();
        void remove_child_at(u64 index);
        void remove_child(const Widget* widget);

        // TODO: get rid of this
        static nvg::Context* context()
        {
            return m_renderer->context();
        }

        static NVGRenderer* renderer()
        {
            return m_renderer;
        }

    public:
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
        virtual bool on_focus_gained();
        virtual bool on_focus_lost();

        virtual void set_size(ds::dims<f32> size);
        virtual void set_min_size(ds::dims<f32> size);
        virtual void set_visible(bool visible);
        virtual void set_theme(Theme* theme);
        virtual void add_child(Widget* widget);
        virtual void add_child(u64 index, Widget* widget);
        virtual void perform_layout();
        virtual void draw();

        virtual bool contains(ds::point<f32> pt);
        virtual bool draw_mouse_intersection(ds::point<f32> pt);

        virtual Widget* find_widget(ds::point<f32> pt);
        virtual ds::dims<f32> preferred_size() const;

    protected:
        [[nodiscard]] f32 icon_scale() const;

    protected:
        static inline rl::NVGRenderer* m_renderer{ nullptr };
        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };

        Widget* m_parent{ nullptr };
        Layout* m_layout{ nullptr };
        Theme* m_theme{ nullptr };

        bool m_enabled{ true };
        bool m_visible{ true };
        bool m_focused{ false };
        bool m_resizable{ false };
        bool m_mouse_focus{ false };
        bool m_size_recalc_needed{ false };

        // TODO: move to theme
        f32 m_icon_extra_scale{ 1.0f };
        f32 m_font_size{ -1.0f };

        std::vector<Widget*> m_children{};
        ds::rect<f32> m_rect{ ds::rect<f32>::zero() };
        ds::dims<f32> m_fixed_size{ ds::dims<f32>::zero() };
        ds::dims<f32> m_min_size{ ds::dims<f32>::null() };
        ds::dims<f32> m_max_size{ ds::dims<f32>::null() };
        std::string m_tooltip{};
        std::string m_name{};
        Timer<f32> m_timer{};

    protected:
        constexpr static inline f32 RESIZE_GRAB_BUFFER{ 5.0f };
        constexpr static bool DiagnosticsEnabled{ true };
        constinit static inline Theme DEFAULT_THEME{ Theme{} };
    };
}
