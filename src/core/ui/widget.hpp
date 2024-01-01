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

#if _MSC_VER
  #pragma warning(disable : 4244)
#endif

namespace rl {
    class Window;
    using WindowID = u32;
    using DisplayID = u32;
}

namespace rl::ui {

    class widget : public ds::refcounted
    {
    public:
        // TODO: remove! only placeholder for Popup
        widget() = default;
        widget(widget* parent);

        /// @brief
        ///     Free all resources used by the widget and any children
        virtual ~widget();

        bool show();
        bool hide();
        bool visible() const;
        bool visible_recursive() const;
        bool has_font_size() const;
        bool enabled() const;
        bool focused() const;
        bool contains(ds::point<i32> pt) const;

        i32 width() const;
        i32 height() const;
        i32 fixed_width() const;
        i32 fixed_height() const;
        i32 font_size() const;
        f32 icon_extra_scale() const;
        i32 child_index(ui::widget* widget) const;
        i32 child_count() const;

        rl::Window* window();
        ui::widget* parent();
        ui::layout* layout();
        ui::theme* theme();
        ui::widget* child_at(int index);
        ui::widget* find_widget(ds::point<i32> pt);
        ds::point<i32> position() const;
        ds::point<i32> abs_position() const;
        ds::dims<i32> fixed_size() const;
        ds::dims<i32> size() const;
        Mouse::Cursor::ID cursor() const;

        const rl::Window* window() const;
        const ui::widget* parent() const;
        const ui::layout* layout() const;
        const ui::theme* theme() const;
        const ui::widget* child_at(i32 index) const;
        const ui::widget* find_widget(ds::point<i32> pt) const;
        const std::vector<ui::widget*>& children() const;
        const std::string& tooltip() const;

        void request_focus();
        void remove_child_at(i32 index);
        void remove_child(const ui::widget* widget);
        void add_child(ui::widget* widget);

        void set_parent(ui::widget* parent);
        void set_layout(ui::layout* layout);
        void set_position(ds::point<i32> pos);
        void set_size(const ds::dims<i32>& size);
        void set_width(i32 width);
        void set_height(i32 height);
        void set_fixed_size(ds::dims<i32> fixed_size);
        void set_fixed_width(i32 width);
        void set_fixed_height(i32 height);
        void set_visible(bool visible);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void set_tooltip(const std::string& tooltip);
        void set_font_size(i32 font_size);
        void set_icon_extra_scale(f32 scale);
        void set_cursor(Mouse::Cursor::ID cursor);

        template <typename TWidget, typename... TArgs>
        TWidget* add(const TArgs&... args)
        {
            return new TWidget{ this, args... };
        }

    public:
        virtual void set_theme(ui::theme* theme);
        virtual void add_child(i32 index, widget* widget);

        virtual bool on_mouse_click(const ds::point<i32>& pt, Mouse::Button::type button, bool down,
                                    i32 modifiers);
        virtual bool on_mouse_move(const ds::point<i32>& pt, const ds::vector2<i32>& rel,
                                   Mouse::Button::type button, i32 modifiers);
        virtual bool on_mouse_drag(const ds::point<i32>& pt, const ds::vector2<i32>& rel,
                                   Mouse::Button::type button, i32 modifiers);
        virtual bool on_mouse_enter(const ds::point<i32>& pt);
        virtual bool on_mouse_exit(const ds::point<i32>& pt);
        virtual bool on_mouse_scroll(const ds::point<i32> pt, const ds::vector2<i32>& rel);
        virtual bool on_kb_focus_gained();
        virtual bool on_kb_focus_lost();
        virtual bool on_kb_key_pressed(const Keyboard::Button::type key);
        virtual bool on_kb_character_input(uint32_t codepoint);

    public:
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const;
        virtual void perform_layout(NVGcontext* nvg_context);
        virtual void draw(NVGcontext* nvg_context);

    protected:
        f32 icon_scale() const;

    protected:
        ui::widget* m_parent{ nullptr };
        ds::shared<ui::theme> m_theme{ nullptr };
        ds::shared<ui::layout> m_layout{ nullptr };
        NVGcontext* m_nvg_context{ nullptr };
        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };
        std::vector<widget*> m_children{};
        std::string m_tooltip{};

        ds::point<i32> m_pos{ 0, 0 };
        ds::dims<i32> m_size{ 0, 0 };
        ds::dims<i32> m_fb_size{ 0, 0 };
        ds::dims<i32> m_fixed_size{ 0, 0 };
        ds::dims<i32> m_framebuf_size{ 0, 0 };

        i32 m_font_size{ 16 };
        f32 m_icon_extra_scale{ 1.0f };

        bool m_enabled{ true };
        bool m_visible{ true };

        bool m_focused{ false };
        bool m_mouse_focus{ false };
        rl::Keyboard m_keyboard{};
        rl::Mouse m_mouse{};

    private:
        constexpr static bool DiagnosticsEnabled{ true };
    };
}
