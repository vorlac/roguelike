#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "ds/margin.hpp"
#include "ds/refcounted.hpp"
#include "ds/vector2d.hpp"
#include "graphics/nvg_renderer.hpp"
#include "utils/time.hpp"

namespace rl::ui {
    class ScrollableDialog;
    class Canvas;
    class Layout;

    class Widget : public ds::refcounted
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
        virtual ~Widget() override;

        void show();
        void hide();
        bool visible() const;
        bool visible_recursive() const;
        bool has_font_size() const;
        bool enabled() const;
        bool focused() const;
        bool resizable() const;

        f32 width() const;
        f32 height() const;
        f32 fixed_width() const;
        f32 fixed_height() const;
        f32 font_size() const;
        f32 icon_extra_scale() const;
        // TODO: change to size_t
        i32 child_index(const Widget* widget) const;
        i32 child_count() const;

        Canvas* canvas();
        Widget* parent();
        Layout* layout();
        ScrollableDialog* dialog();

        //  TODO: change to size_t
        Widget* child_at(i32 index);
        Mouse::Cursor::ID cursor() const;
        ds::rect<f32> resize_rect() const;
        ds::point<f32> abs_position() const;

        const ds::point<f32>& position() const;
        const ds::dims<f32>& fixed_size() const;
        const ds::dims<f32>& min_size() const;
        const ds::dims<f32>& max_size() const;
        const ds::dims<f32>& size() const;
        const ds::rect<f32>& rect() const;
        const Theme* theme() const;
        const Canvas* canvas() const;
        const ScrollableDialog* dialog() const;
        const Widget* parent() const;
        const Widget* child_at(i32 index) const;
        const std::vector<Widget*>& children() const;
        const std::string& tooltip() const;
        const std::string& name() const;

        void assign_layout(Layout* layout);
        void set_parent(Widget* parent);
        void set_position(const ds::point<f32>& pos);
        void set_rect(const ds::rect<f32>& rect);
        void set_size(const ds::dims<f32>& size);
        void set_width(f32 width);
        void set_height(f32 height);
        void set_fixed_size(const ds::dims<f32>& fixed_size);
        void set_fixed_width(f32 width);
        void set_fixed_height(f32 height);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(const std::string& tooltip);
        void set_name(const std::string& name);
        void set_font_size(f32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);
        void set_min_size(ds::dims<f32> min_size);
        void set_max_size(ds::dims<f32> max_size);

        void request_focus();
        // TODO: change to size_t
        void remove_child_at(i32 index);
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

        virtual void set_visible(bool visible);
        virtual void set_theme(Theme* theme);
        virtual void add_child(Widget* widget);
        virtual void add_child(i32 index, Widget* widget);
        virtual void perform_layout();
        virtual void draw();

        virtual ds::dims<f32> preferred_size() const;
        virtual Widget* find_widget(const ds::point<f32>& pt);
        virtual bool contains(const ds::point<f32>& pt);
        virtual bool draw_mouse_intersection(const ds::point<f32>& pt);

    protected:
        f32 icon_scale() const;

    protected:
        Widget* m_parent{};
        Theme* m_theme{ nullptr };
        Layout* m_layout{ nullptr };

        static inline rl::NVGRenderer* m_renderer{ nullptr };

        bool m_enabled{ true };
        bool m_visible{ true };
        bool m_focused{ false };
        bool m_resizable{ false };
        bool m_mouse_focus{ false };

        ds::rect<f32> m_rect{
            ds::point<f32>::zero(),
            ds::dims<f32>::zero(),
        };

        ds::dims<f32> m_fixed_size{ ds::dims<f32>::zero() };
        ds::dims<f32> m_min_size{ ds::dims<f32>::null() };
        ds::dims<f32> m_max_size{ ds::dims<f32>::null() };
        ds::margin<f32> m_outer_margin{ ds::margin<f32>::zero() };
        ds::margin<f32> m_inner_padding{ ds::margin<f32>::zero() };

        f32 m_font_size{ -1.0f };
        f32 m_icon_extra_scale{ 1.0f };

        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };
        std::vector<Widget*> m_children{};
        std::string m_tooltip{};
        Timer<f32> m_timer{};

        std::string m_name{};

    protected:
        constexpr static inline f32 RESIZE_GRAB_BUFFER{ 5.0f };
        constexpr static bool DiagnosticsEnabled{ true };
        constexpr static inline const Theme DEFAULT_THEME{ Theme{} };
    };
}
