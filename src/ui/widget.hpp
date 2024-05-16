#pragma once

#include <memory>
#include <string>
#include <vector>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "ds/rect.hpp"
#include "ds/refcounted.hpp"
#include "ds/vector2d.hpp"
#include "gfx/nvg_renderer.hpp"
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

        void assign_layout(Layout* layout);
        void set_position(ds::point<f32> pos);
        void set_rect(const ds::rect<f32>& rect);
        void set_width(f32 width);
        void set_height(f32 height);
        void set_fixed_size(ds::dims<f32> fixed_size);
        void set_fixed_width(f32 width);
        void set_fixed_height(f32 height);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(std::string tooltip);
        void set_name(std::string name);
        void set_expansion(f32 stretch);
        void set_font_size(f32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);
        void remove_child_at(u64 index);
        void request_focus();
        void show();
        void hide();

        [[nodiscard]] bool enabled() const;
        [[nodiscard]] bool focused() const;
        [[nodiscard]] bool resizable() const;
        [[nodiscard]] bool visible(bool recursive = false) const;
        [[nodiscard]] bool has_font_size_override() const;

        [[nodiscard]] Mouse::Cursor::ID cursor() const;
        [[nodiscard]] std::string_view tooltip() const;
        [[nodiscard]] std::string_view name() const;

        [[nodiscard]] ds::point<f32> position() const;
        [[nodiscard]] ds::point<f32> abs_position() const;
        [[nodiscard]] ds::dims<f32> size() const;
        [[nodiscard]] ds::dims<f32> min_size() const;
        [[nodiscard]] ds::dims<f32> max_size() const;
        [[nodiscard]] ds::dims<f32> fixed_size() const;
        [[nodiscard]] ds::rect<f32> resize_rect() const;

        [[nodiscard]] u64 child_count() const;
        [[nodiscard]] f32 icon_extra_scale() const;
        [[nodiscard]] f32 width() const;
        [[nodiscard]] f32 height() const;
        [[nodiscard]] f32 fixed_width() const;
        [[nodiscard]] f32 fixed_height() const;
        [[nodiscard]] f32 expansion() const;
        [[nodiscard]] f32 font_size() const;

        [[nodiscard]] Widget* parent();
        [[nodiscard]] Canvas* canvas();
        [[nodiscard]] Layout* layout() const;
        [[nodiscard]] Widget* parent() const;
        [[nodiscard]] Theme* theme() const;
        [[nodiscard]] ScrollableDialog* dialog();
        [[nodiscard]] Widget* child_at(u64 index) const;

        [[nodiscard]] const Canvas* canvas() const;
        [[nodiscard]] const ScrollableDialog* dialog() const;
        [[nodiscard]] const std::vector<Widget*>& children() const;
        [[nodiscard]] const ds::rect<f32>& rect() const;

    private:
        void set_parent(Widget* parent);
        void remove_child(const Widget* widget);

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
        virtual void set_max_size(ds::dims<f32> size);
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
        static inline NVGRenderer* m_renderer{ nullptr };
        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };

        Widget* m_parent{ nullptr };
        Layout* m_layout{ nullptr };
        Theme* m_theme{ nullptr };

        bool m_enabled{ true };
        bool m_visible{ true };
        bool m_focused{ false };
        bool m_resizable{ false };
        bool m_mouse_focus{ false };

        // TODO: move to theme
        f32 m_icon_extra_scale{ 1.0f };
        f32 m_font_size{ text::font::InvalidSize };
        f32 m_stretch_factor{ 1.0f };

        std::vector<Widget*> m_children{};
        ds::rect<f32> m_rect{};
        ds::dims<f32> m_fixed_size{};
        ds::dims<f32> m_min_size{};
        ds::dims<f32> m_max_size{};
        std::string m_tooltip{};
        std::string m_name{};
        Timer<f32> m_timer{};

    protected:
        constexpr static f32 RESIZE_GRAB_BUFFER{ 5.0f };
        constexpr static bool DiagnosticsEnabled{ true };
        constinit static inline Theme m_default_theme{};

    public:
        // TODO: get rid of this stuff
        static nvg::Context* context()
        {
            return m_renderer->context();
        }

        static NVGRenderer* renderer()
        {
            return m_renderer;
        }
    };
}
