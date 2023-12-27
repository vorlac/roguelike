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
        /// @brief
        ///     Create a new ui::widget.
        /// @brief
        ///     The parent should only ever be nullptr for the root widget
        ///
        /// @param  parent
        ///     The parent of the widget being created
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
        void add_child(ui::widget* widget);
        void remove_child_at(i32 index);
        void remove_child(const ui::widget* widget);
        void set_enabled(bool enabled);
        void set_focused(bool focused);
        void request_focus();
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
        virtual ds::dims<i32> preferred_size(NVGcontext* nvg_context) const;
        virtual void perform_layout(NVGcontext* nvg_context);
        virtual void draw(NVGcontext* nvg_context);
        virtual void set_theme(ui::theme* theme);
        virtual void add_child(int index, widget* widget);

    protected:
        virtual bool on_kb_character_input(const WindowID id);
        virtual bool on_kb_key_pressed(const Keyboard::Button::type key);
        virtual bool on_kb_focus_gained(const WindowID id);
        virtual bool on_kb_focus_lost(const WindowID id);
        virtual bool on_mouse_enter(const WindowID id);
        virtual bool on_mouse_leave(const WindowID id);
        virtual bool on_mouse_click(const WindowID id);
        virtual bool on_mouse_drag(const WindowID id);
        virtual bool on_mouse_scroll(const WindowID id);

    protected:
        f32 icon_scale() const;

    protected:
        ui::widget* m_parent{ nullptr };
        ds::shared<ui::theme> m_theme{ nullptr };
        ds::shared<ui::layout> m_layout{ nullptr };
        NVGcontext* m_nvg_context{ nullptr };
        Mouse::Cursor::ID m_cursor{ Mouse::Cursor::Arrow };

        ds::point<i32> m_pos{ 0, 0 };
        ds::dims<i32> m_size{ 0, 0 };
        ds::dims<i32> m_fb_size{ 0, 0 };
        ds::dims<i32> m_fixed_size{ 0, 0 };
        std::vector<widget*> m_children{};
        std::string m_tooltip{};

        i32 m_font_size{ 16 };
        f32 m_icon_extra_scale{ 1.0f };

        bool m_enabled{ true };
        bool m_focused{ false };
        bool m_mouse_focus{ false };
        bool m_visible{ true };

    private:
        constexpr static bool DiagnosticsEnabled{ true };
    };

}
