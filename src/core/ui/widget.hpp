#pragma once

#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/layouts/layout.hpp"
#include "core/ui/theme.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"
#include "graphics/nvg_renderer.hpp"
#include "utils/numeric.hpp"
#include "utils/time.hpp"

namespace rl::ui {
    class Dialog;
    class Canvas;

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

        using refcounted::operator=;

        void show();
        void hide();
        bool visible() const;
        bool visible_recursive() const;
        bool has_font_size() const;
        bool enabled() const;
        bool focused() const;
        bool contains(const ds::point<f32>& pt) const;

        f32 width() const;
        f32 height() const;
        f32 fixed_width() const;
        f32 fixed_height() const;
        f32 font_size() const;
        f32 icon_extra_scale() const;
        i32 child_index(const Widget* widget) const;
        i32 child_count() const;

        Canvas* canvas();
        Dialog* dialog();
        Widget* parent();
        Layout* layout();
        Theme* theme();
        Widget* child_at(i32 index);
        Widget* find_widget(const ds::point<f32>& pt);
        Mouse::Cursor::ID cursor() const;
        ds::point<f32> position() const;
        ds::point<f32> abs_position() const;
        ds::dims<f32> fixed_size() const;
        ds::dims<f32> size() const;

        const Canvas* canvas() const;
        const Dialog* dialog() const;
        const Widget* parent() const;
        const Layout* layout() const;
        const Theme* theme() const;
        const Widget* child_at(i32 index) const;
        const Widget* find_widget(const ds::point<f32>& pt) const;
        const std::vector<Widget*>& children() const;
        const std::string& tooltip() const;

        void set_parent(Widget* parent);
        void set_layout(Layout* layout);
        void set_position(ds::point<f32>&& pos);
        void set_size(ds::dims<f32>&& size);
        void set_width(f32 width);
        void set_height(f32 height);
        void set_fixed_size(const ds::dims<f32>& fixed_size);
        void set_fixed_width(f32 width);
        void set_fixed_height(f32 height);
        virtual void set_visible(bool visible);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(const std::string& tooltip);
        void set_font_size(f32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);

        void request_focus();
        ds::rect<f32> bounding_rect() const;
        void remove_child_at(i32 index);
        void remove_child(const Widget* widget);
        virtual void add_child(Widget* widget);

    public:
        // TODO: get rid of this
        static nvg::Context* context()
        {
            return m_renderer->context();
        }

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
        virtual bool draw_mouse_intersection(const ds::point<f32>& pt);

    protected:
        f32 icon_scale() const;
        virtual std::string_view name() const;

    protected:
        Widget* m_parent{ nullptr };
        ds::shared<Theme> m_theme{ nullptr };
        ds::shared<Layout> m_layout{ nullptr };
        static inline rl::NVGRenderer* m_renderer{ nullptr };

        bool m_enabled{ true };
        bool m_visible{ true };
        bool m_focused{ false };
        bool m_mouse_focus{ false };

        f32 m_font_size{ -1.0f };
        f32 m_icon_extra_scale{ 1.0f };

        ds::point<f32> m_pos{ 0.0f, 0.0f };
        ds::dims<f32> m_size{ 0.0f, 0.0f };
        ds::dims<f32> m_fixed_size{ 0.0f, 0.0f };

        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };
        std::vector<Widget*> m_children{};
        std::string m_tooltip{};

        Timer<f32> m_timer{};

    private:
        constexpr static bool DiagnosticsEnabled{ false };
    };
}
