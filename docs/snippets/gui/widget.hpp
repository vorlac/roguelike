#pragma once

#include <algorithm>
#include <vector>

#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "ds/rect.hpp"
#include "ds/vector2d.hpp"
#include "gui/object.hpp"
#include "gui/theme.hpp"

namespace rl::gui {

    enum class Cursor;

    class widget
    {
    public:
        // Construct a new widget
        // with the given parent
        widget(widget* parent);

        // Free all resources used by
        // the widget and any children
        virtual ~widget();

        inline widget* parent()
        {
            return m_parent;
        }

        const inline widget* parent() const
        {
            return m_parent;
        }

        inline void set_parent(widget* parent)
        {
            m_parent = parent;
        }

        inline Layout* layout() const
        {
            return m_layout;
        }

        inline Layout* layout() const
        {
            return m_layout.get();
        }

        inline void set_layout(Layout* layout)
        {
            m_layout = layout;
        }

        inline Theme* theme()
        {
            return m_theme;
        }

        const inline Theme* theme() const
        {
            return m_theme.get();
        }

        virtual void set_theme(Theme* theme)
        {
            if (m_theme.get() == theme)
                return;

            m_theme = theme;
            for (auto&& child : m_children)
                child->set_theme(theme);
        }

        inline ds::point<i32> position() const
        {
            return m_pos;
        }

        inline void set_position(ds::point<i32> pos)
        {
            m_pos = pos;
        }

        inline ds::point<i32> abs_position() const
        {
            // Return the position in absolute screen coords
            return m_parent != nullptr                   //
                     ? m_parent->abs_position() + m_pos  //
                     : m_pos;                            //
        }

        inline ds::dims<i32> size() const
        {
            return m_size;
        }

        inline void set_size(ds::dims<i32> size)
        {
            m_size = size;
        }

        inline i32 width() const
        {
            return m_size.width;
        }

        inline void set_width(i32 width)
        {
            m_size.width = width;
        }

        inline i32 height() const
        {
            return m_size.height;
        }

        inline void set_height(i32 height)
        {
            m_size.height = height;
        }

        inline void set_fixed_size(ds::dims<i32> fixed_size)
        {
            m_fixed_size = fixed_size;
        }

        inline ds::dims<i32> fixed_size() const
        {
            return m_fixed_size;
        }

        inline i32 fixed_width() const
        {
            return m_fixed_size.width;
        }

        inline i32 fixed_height() const
        {
            return m_fixed_size.height;
        }

        inline void set_fixed_width(i32 width)
        {
            m_fixed_size.width = width;
        }

        inline void set_fixed_height(i32 height)
        {
            m_fixed_size.height = height;
        }

        inline bool visible() const
        {
            return m_visible;
        }

        inline void set_visible(bool visible)
        {
            m_visible = visible;
        }

        inline void show()
        {
            this->set_visible(true);
        }

        inline void hide()
        {
            this->set_visible(false);
        }

        // Check if this widget is currently visible, taking parent widgets into account
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

        // Return the number of child widgets
        int child_count() const
        {
            return (int)m_children.size();
        }

        // Return the list of child widgets of the current widget
        const std::vector<widget*>& children() const
        {
            return m_children;
        }

        /**
         * \brief Add a child widget to the current widget at
         * the specified index.
         *
         * This function almost never needs to be called by hand,
         * since the constructor of widget automatically
         * adds the current widget to its parent
         */
        virtual void add_child(int index, widget* widget);

        // Convenience function which appends a widget at the end
        void add_child(widget* widget);

        // Remove a child widget by index
        void remove_child_at(int index);

        // Remove a child widget by value
        void remove_child(const widget* widget);

        // Retrieves the child at the specific position
        const widget* child_at(int index) const
        {
            return m_children[(size_t)index];
        }

        // Retrieves the child at the specific position
        widget* child_at(int index)
        {
            return m_children[(size_t)index];
        }

        // Returns the index of a specific child or -1 if not found
        int child_index(widget* widget) const;

        // Variadic shorthand notation to construct and add a child widget
        template <typename widgetClass, typename... Args>
        widgetClass* add(const Args&... args)
        {
            return new widgetClass(this, args...);
        }

        // Walk up the hierarchy and return the parent window
        Window* window();
        // Walk up the hierarchy and return the parent window (const version)
        const Window* window() const;

        // Walk up the hierarchy and return the parent screen
        Screen* screen();
        // Walk up the hierarchy and return the parent screen (const version)
        const Screen* screen() const;

        // Return whether or not this widget is currently enabled
        bool enabled() const
        {
            return m_enabled;
        }

        // Set whether or not this widget is currently enabled
        void set_enabled(bool enabled)
        {
            m_enabled = enabled;
        }

        // Return whether or not this widget is currently focused
        bool focused() const
        {
            return m_focused;
        }

        // Set whether or not this widget is currently focused
        void set_focused(bool focused)
        {
            m_focused = focused;
        }

        // Request the focus to be moved to this widget
        void request_focus();

        const std::string& tooltip() const
        {
            return m_tooltip;
        }

        void set_tooltip(const std::string& tooltip)
        {
            m_tooltip = tooltip;
        }

        // Return current font size. If not set the default of the current theme will be returned
        int font_size() const;

        // Set the font size of this widget
        void set_font_size(int font_size)
        {
            m_font_size = font_size;
        }

        // Return whether the font size is explicitly specified for this widget
        bool has_font_size() const
        {
            return m_font_size > 0;
        }

        /**
         * The amount of extra scaling applied to *icon* fonts.
         * See widget::m_icon_extra_scale.
         */
        float icon_extra_scale() const
        {
            return m_icon_extra_scale;
        }

        /**
         * Sets the amount of extra scaling applied to *icon* fonts.
         * See widget::m_icon_extra_scale.
         */
        void set_icon_extra_scale(float scale)
        {
            m_icon_extra_scale = scale;
        }

        // Return a pointer to the cursor of the widget
        Cursor cursor() const
        {
            return m_cursor;
        }

        // Set the cursor of the widget
        void set_cursor(Cursor cursor)
        {
            m_cursor = cursor;
        }

        // Check if the widget contains a certain position
        bool contains(const ds::dims<i32>& p) const
        {
            ds::dims<i32> d = p - m_pos;
            return d.x() >= 0 && d.y() >= 0 && d.x() < m_size.x() && d.y() < m_size.y();
        }

        // Determine the widget located at the given position value (recursive)
        widget* find_widget(const ds::dims<i32>& p);
        const widget* find_widget(const ds::dims<i32>& p) const;

        // Handle a mouse button event (default implementation: propagate to children)
        virtual bool mouse_button_event(const ds::dims<i32>& p, int button, bool down,
                                        int modifiers);

        // Handle a mouse motion event (default implementation: propagate to children)
        virtual bool mouse_motion_event(const ds::dims<i32>& p, const ds::dims<i32>& rel,
                                        int button, int modifiers);

        // Handle a mouse drag event (default implementation: do nothing)
        virtual bool mouse_drag_event(const ds::dims<i32>& p, const ds::dims<i32>& rel, int button,
                                      int modifiers);

        // Handle a mouse enter/leave event (default implementation: record this fact, but do
        // nothing)
        virtual bool mouse_enter_event(const ds::dims<i32>& p, bool enter);

        // Handle a mouse scroll event (default implementation: propagate to children)
        virtual bool scroll_event(const ds::dims<i32>& p, const Vector2f& rel);

        // Handle a focus change event (default implementation: record the focus status, but do
        // nothing)
        virtual bool focus_event(bool focused);

        // Handle a keyboard event (default implementation: do nothing)
        virtual bool keyboard_event(int key, int scancode, int action, int modifiers);

        // Handle text input (UTF-32 format) (default implementation: do nothing)
        virtual bool keyboard_character_event(unsigned int codepoint);

        // Compute the preferred size of the widget
        virtual ds::dims<i32> preferred_size(NVGcontext* ctx) const;

        // Invoke the associated layout generator to properly place child widgets, if any
        virtual void perform_layout(NVGcontext* ctx);

        // Draw the widget (and all child widgets)
        virtual void draw(NVGcontext* ctx);

    protected:
        /**
         * Convenience definition for subclasses to get the full icon scale for this
         * class of widget.  It simple returns the value
         * ``m_theme->m_icon_scale * this->m_icon_extra_scale``.
         *
         * \remark
         *     See also: Theme::m_icon_scale and
         *     widget::m_icon_extra_scale.  This tiered scaling
         *     strategy may not be appropriate with fonts other than ``entypo.ttf``.
         */
        float icon_scale() const
        {
            return m_theme->m_icon_scale * m_icon_extra_scale;
        }

    protected:
        widget* m_parent{};
        ref<Theme> m_theme{};
        ref<Layout> m_layout{};
        ds::point<i32> m_pos{};
        ds::dims<i32> m_size{};
        ds::dims<i32> m_fixed_size{};
        std::vector<widget*> m_children;
        std::string m_tooltip{};
        Cursor m_cursor{};

        bool m_enabled{ true };
        bool m_focused{ false };
        bool m_mouse_focus{ false };
        bool m_visible{ true };
        i32 m_font_size{ 16 };

        /// <summary>
        ///   <para>
        ///     The amount of extra icon scaling used in addition the the
        ///     theme's default icon font scale. Default value is 1.0, which implies that
        ///     <code>widget::icon_scale</code> simply returns the value of
        ///     <code>Theme::m_icon_scale</code>.
        ///   </para><para/>
        ///
        ///   <para>
        ///     <para/>
        ///     <para/>
        ///     Most widgets do not need extra scaling, but some (ie. CheckBox, TextBox)
        ///     need to adjust the Theme's default icon scaling
        ///     (Theme::m_icon_scale) to properly display icons within their
        ///     bounds (upscale, or downscale).
        ///   </para><para/>
        ///
        ///   <para>
        ///     When using <c/>nvgFontSize() for icons in
        ///     subclasses, make sure to call <c/>widget::icon_scale(). Expected usage when
        ///     drawing icon fonts is something like:
        ///   </para>
        /// <example>
        ///   <code>
        ///     virtual void draw(NVGcontext *ctx) {
        ///         // fontSize depends on the kind of widget.
        ///         // Search for 'FontSize' in the Theme class.
        ///         float ih = font_size;
        ///         // assuming your widget has a declared 'mIcon'
        ///         if (nvgIsFontIcon(mIcon))
        ///         {
        ///             ih *= icon_scale();
        ///             nvgFontFace(ctx, "icons");
        ///             nvgFontSize(ctx, ih);
        ///             // remaining drawing code (see button.cpp for more)
        ///         }
        ///     }
        ///  </code>
        /// </example>
        /// </summary>
        float m_icon_extra_scale{};

        // Enables diagnostic rendering that displays widget bounds
        constexpr static inline bool DiagnosticsEnabled = false;
    };

}
