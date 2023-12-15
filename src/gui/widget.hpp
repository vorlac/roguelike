#pragma once

#include <type_traits>
#include <vector>

#include "gui/common.hpp"
#include "gui/layout.hpp"
#include "gui/theme.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

namespace rl::gui {
    class Window;
    class Label;
    class ToolButton;
    class MessageDialog;
    class PopupButton;
    class Button;
    class ComboBox;
    class CheckBox;
    class VScrollPanel;
    class ProgressBar;
    class Slider;
    class ImagePanel;
    class DropdownBox;
    class TextBox;

    /**
     * @brief Base class of all widgets.
     *
     * Widget is the base class of all widgets. It can also be used as an panel
     * to arrange an arbitrary number of child widgets using a layout generator.
     */
    class Widget : public Object
    {
    public:
        /**
         * @brief Construct a new widget with the given parent widget
         **/
        Widget(Widget* parent);

        /**
         * @brief Gets the parent widget
         **/
        Widget* parent()
        {
            return m_parent;
        }

        /**
         * @brief Gets the parent widget
         **/
        const Widget* parent() const
        {
            return m_parent;
        }

        /**
         * @brief Sets the parent widget
         **/
        void set_parent(Widget* parent)
        {
            m_parent = parent;
        }

        /**
         * @brief Return the used layout generator
         **/
        Layout* layout()
        {
            return m_layout;
        }

        /**
         * @brief Gets the used layout generator
         **/
        const Layout* layout() const
        {
            return m_layout.get();
        }

        /**
         * @brief Sets the used layout generator
         **/
        void set_layout(Layout* layout)
        {
            m_layout = layout;
        }

        /**
         * @brief Gets the theme used render the widget
         **/
        Theme* theme()
        {
            return m_theme;
        }

        /**
         * @brief Gets the theme used render the widget
         **/
        const Theme* theme() const
        {
            return m_theme.get();
        }

        /**
         * @brief Sets the theme used to render the widget
         **/
        virtual void set_theme(Theme* theme);

        /**
         * @brief Gets the position relative to the parent widget
         **/
        const Vector2i& relative_position() const
        {
            return m_pos;
        }

        /**
         * @brief Sets the position relative to the parent widget
         **/
        void set_relative_position(const Vector2i& pos)
        {
            m_pos = pos;
        }

        /**
         * @brief Sets the position relative to the parent widget
         **/
        void set_relative_position(int x, int y)
        {
            m_pos = { x, y };
        }

        /**
         * @brief Gets the absolute screen coordinates
         **/
        Vector2i absolute_position() const
        {
            return m_parent ? (m_parent->absolute_position() + m_pos) : m_pos;
        }

        /**
         * @brief Gets the widget dimensions
         **/
        const Vector2i& size() const
        {
            return m_size;
        }

        /**
         * @brief Sets the widget dimensions
         **/
        void set_size(const Vector2i& size)
        {
            m_size = size;
        }

        /**
         * @brief Gets the widget's width
         **/
        int width() const
        {
            return m_size.x;
        }

        /**
         * @brief Sets the widget's width
         **/
        void set_width(int width)
        {
            m_size.x = width;
        }

        /**
         * @brief Gets the widget's height
         **/
        int height() const
        {
            return m_size.y;
        }

        /**
         * @brief Sets the widget's height
         **/
        void set_height(int height)
        {
            m_size.y = height;
        }

        /**
         * @brief Sets the fixed size of this widget
         *
         * If nonzero, components of the fixed size attribute override any values
         * computed by a layout generator associated with this widget. Note that
         * just setting the fixed size alone is not enough to actually change its
         * size; this is done with a call to set_size() or a call to perform_layout()
         * in the parent widget.
         */
        void set_fixed_size(const Vector2i& fixedSize)
        {
            m_fixed_size = fixedSize;
        }

        /**
         * @brief Return the fixed size
         **/
        const Vector2i& fixed_size() const
        {
            return m_fixed_size;
        }

        /**
         * @brief Gets the widget's fixed width
         **/
        int fixed_width() const
        {
            return m_fixed_size.x;
        }

        /**
         * @brief Gets the widget's fixed height
         **/
        int fixed_height() const
        {
            return m_fixed_size.y;
        }

        /**
         * @brief  Sets the widget's fixed width
         **/
        void set_fixed_width(int width)
        {
            m_fixed_size.x = width;
        }

        /**
         * @brief Sets the widget's fixed width
         * @return The resized widget
         **/
        Widget& with_fixed_width(int width)
        {
            set_fixed_width(width);
            return *this;
        }

        /**
         * @brief Set the widget's fixed height
         **/
        void set_fixed_height(int height)
        {
            m_fixed_size.y = height;
        }

        /**
         * @brief Return whether or not the widget is currently
         *        visible, assuming all parents are visible
         **/
        bool visible() const
        {
            return m_visible;
        }

        /**
         * @brief Sets the widget's visbility, assuming
         *        all parents are visible
         **/
        void set_visible(bool visible)
        {
            m_visible = visible;
        }

        /**
         * @brief Checks if this widget is currently visible by traversing
         *        it's ancestors, taking all higher level widgets into account
         **/
        bool visible_recursive() const
        {
            bool visible = true;
            const Widget* widget = this;
            while (widget)
            {
                visible &= widget->visible();
                widget = widget->parent();
            }
            return visible;
        }

        /**
         * @brief Return the number of child widgets
         **/
        size_t child_count() const
        {
            return m_children.size();
        }

        /**
         * @brief  Return the list of child widgets of the current widget
         **/
        const std::vector<Widget*>& children() const
        {
            return m_children;
        }

        /**
         * @brief Add a child widget to the current widget at
         * the specified index.
         *
         * This function almost never needs to be called by hand,
         * since the constructor of Widget automatically
         * adds the current widget to its parent
         */
        virtual void add_child(size_t index, Widget* widget);

        /**
         * @brief Convenience function which appends a widget at the end
         **/
        void add_child(Widget* widget);

        /**
         * @brief Remove a child widget by index
         **/
        void remove_child(size_t index);

        /**
         * @brief Remove a child widget by value
         **/
        void remove_child(const Widget* widget);

        /**
         * @brief Retrieves the child at the specific position
         **/
        const Widget* get_child(size_t index) const
        {
            return m_children[index];
        }

        /**
         * @brief Retrieves the child at the specific position
         **/
        Widget* get_child(size_t index)
        {
            return m_children[index];
        }

        /**
         * @brief Returns the index of a specific child or -1 if not found
         **/
        int get_child_index(Widget* widget) const;

        /**
         * @brief Variadic shorthand notation to construct and add a child widget
         **/
        template <typename TWidget, typename... TArgs>
        TWidget* add(const TArgs&... args)
        {
            return new TWidget(this, args...);
        }

        template <typename TWidget, typename... TArgs>
        TWidget& wdg(const TArgs&... args)
        {
            TWidget* widget = new TWidget(this, args...);
            return *widget;
        }

        /**
         * @brief Walk up the hierarchy and return the parent window
         **/
        Window* window();

        /**
         * @brief Associate this widget with an ID value (optional)
         **/
        void setId(const std::string& id)
        {
            m_id = id;
        }

        /**
         * @brief Return the ID value associated with this widget, if any
         **/
        const std::string& id() const
        {
            return m_id;
        }

        /**
         * @brief Return whether or not this widget is currently enabled
         **/
        bool enabled() const
        {
            return m_enabled;
        }

        /**
         * @brief Set whether or not this widget is currently enabled
         **/
        void setEnabled(bool enabled)
        {
            m_enabled = enabled;
        }

        /**
         * @brief Return whether or not this widget is currently focused
         **/
        bool focused() const
        {
            return m_focused;
        }

        /**
         * @brief Set whether or not this widget is currently focused
         **/
        void setFocused(bool focused)
        {
            m_focused = focused;
        }

        /**
         * @brief Request the focus to be moved to this widget
         **/
        void requestFocus();

        const std::string& tooltip() const
        {
            return m_tooltip;
        }

        void setTooltip(const std::string& tooltip)
        {
            m_tooltip = tooltip;
        }

        /**
         * @brief Return current font size.
         *        If not set the default of the current theme will be returned.
         **/
        int fontSize() const;

        /**
         * @brief Set the font size of this widget
         **/
        virtual void setFontSize(int fontSize)
        {
            m_font_size = fontSize;
        }

        /**
         * @brief Return whether the font size is explicitly specified for this widget
         **/
        bool hasFontSize() const
        {
            return m_font_size > 0;
        }

        /**
         * @brief Return a pointer to the cursor of the widget
         **/
        Cursor cursor() const
        {
            return m_cursor;
        }

        /**
         * @brief Set the cursor of the widget
         **/
        void setCursor(Cursor cursor)
        {
            m_cursor = cursor;
        }

        /**
         * @brief Check if the widget contains a certain position
         **/
        bool contains(const Vector2i& p) const
        {
            Vector2i d = p - m_pos;
            return d.positive() && d.lessOrEq({ m_size.x, m_size.y });
        }

        /**
         * @brief Determine the widget located at the given position value (recursive)
         **/
        Widget* findWidget(const Vector2i& p);
        Widget* find(const std::string& id, bool inchildren = true);

        Widget* gfind(const std::string& id)
        {
            Widget* parent = this;
            while (parent->parent())
                parent = parent->parent();

            return parent->find(id, true);
        }

        template <typename RetClass>
        RetClass* gfind(const std::string& id)
        {
            Widget* f = gfind(id);
            return f ? f->cast<RetClass>() : nullptr;
        }

        /**
         * @brief Handle a mouse button event (default implementation: propagate to children)
         **/
        virtual bool mouseButtonEvent(const Vector2i& p, int button, bool down, int modifiers);

        /**
         * @brief Handle a mouse motion event (default implementation: propagate to children)
         **/
        virtual bool mouseMotionEvent(const Vector2i& p, const Vector2i& rel, int button,
                                      int modifiers);

        /**
         * @brief Handle a mouse drag event (default implementation: do nothing)
         **/
        virtual bool mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button,
                                    int modifiers);

        /**
         * @brief Handle a mouse enter/leave event (default implementation: record this fact, but do
         * nothing)
         **/
        virtual bool mouseEnterEvent(const Vector2i& p, bool enter);

        /**
         * @brief Handle a mouse scroll event (default implementation: propagate to children)
         **/
        virtual bool scrollEvent(const Vector2i& p, const Vector2f& rel);

        /**
         * @brief Handle a focus change event (default implementation: record the focus status, but
         * do nothing)
         **/
        virtual bool focusEvent(bool focused);

        /**
         * @brief Handle a keyboard event (default implementation: do nothing)
         **/
        virtual bool kb_button_event(int key, int scancode, int action, int modifiers);

        /**
         * @brief Handle text input (UTF-32 format) (default implementation: do nothing)
         **/
        virtual bool kb_character_event(unsigned int codepoint);

        /**
         * @brief Compute the preferred size of the widget
         **/
        virtual Vector2i preferredSize(SDL3::SDL_Renderer* ctx) const;

        /**
         * @brief Invoke the associated layout generator to properly place child widgets, if any
         **/
        virtual void perform_layout(SDL3::SDL_Renderer* ctx);

        /**
         * @brief Draw the widget (and all child widgets)
         **/
        virtual void draw(SDL3::SDL_Renderer* renderer);

        virtual int get_absolute_left() const;
        virtual SDL3::SDL_Point get_absolute_pos() const;
        virtual PntRect get_absolute_cliprect() const;
        virtual int get_absolute_top() const;

        Widget& _and()
        {
            return *parent();
        }

        Widget& withId(const std::string& id)
        {
            setId(id);
            return *this;
        }

        Widget& withPosition(const Vector2i& pos)
        {
            set_relative_position(pos);
            return *this;
        }

        Widget& withFontSize(int size)
        {
            setFontSize(size);
            return *this;
        }

        Widget& withFixedSize(const Vector2i& size)
        {
            set_fixed_size(size);
            return *this;
        }

        Widget& withTooltip(const std::string& text)
        {
            setTooltip(text);
            return *this;
        }

        template <typename LayoutClass, typename... TArgs>
        Widget& withLayout(const TArgs&... args)
        {
            set_layout(new LayoutClass(args...));
            return *this;
        }

        template <typename RetClass>
        RetClass* cast()
        {
            return dynamic_cast<RetClass*>(this);
        }

        template <typename... TArgs>
        Widget& boxlayout(const TArgs&... args)
        {
            return withLayout<BoxLayout>(args...);
        }

        template <typename... TArgs>
        ToolButton& toolbutton(const TArgs&... args)
        {
            return wdg<ToolButton>(args...);
        }

        template <typename... TArgs>
        PopupButton& popupbutton(const TArgs&... args)
        {
            return wdg<PopupButton>(args...);
        }

        template <typename... TArgs>
        Label& label(const TArgs&... args)
        {
            return wdg<Label>(args...);
        }

        template <typename... TArgs>
        ProgressBar& progressbar(const TArgs&... args)
        {
            return wdg<ProgressBar>(args...);
        }

        template <typename... TArgs>
        DropdownBox& dropdownbox(const TArgs&... args)
        {
            return wdg<DropdownBox>(args...);
        }

        template <typename... TArgs>
        ComboBox& combobox(const TArgs&... args)
        {
            return wdg<ComboBox>(args...);
        }

        template <typename... TArgs>
        Button& button(const TArgs&... args)
        {
            return wdg<Button>(args...);
        }

        template <typename... TArgs>
        Widget& widget(const TArgs&... args)
        {
            return wdg<Widget>(args...);
        }

        template <typename... TArgs>
        CheckBox& checkbox(const TArgs&... args)
        {
            return wdg<CheckBox>(args...);
        }

        template <typename... TArgs>
        MessageDialog& msgdialog(const TArgs&... args)
        {
            return wdg<MessageDialog>(args...);
        }

        template <typename... TArgs>
        VScrollPanel& vscrollpanel(const TArgs&... args)
        {
            return wdg<VScrollPanel>(args...);
        }

        template <typename... TArgs>
        ImagePanel& imgpanel(const TArgs&... args)
        {
            return wdg<ImagePanel>(args...);
        }

        template <typename... TArgs>
        Slider& slider(const TArgs&... args)
        {
            return wdg<Slider>(args...);
        }

        template <typename... TArgs>
        TextBox& textbox(const TArgs&... args)
        {
            return wdg<TextBox>(args...);
        }

    protected:
        /**
         * @brief Free all resources used by the widget and any children
         **/
        virtual ~Widget() override;

    protected:
        gui::Widget* m_parent{ nullptr };
        gui::ref<Theme> m_theme{};
        gui::ref<Layout> m_layout{};
        std::string m_id{};
        gui::Vector2i m_pos{};
        gui::Vector2i m_size{};
        gui::Vector2i m_fixed_size{};
        std::vector<gui::Widget*> m_children = {};
        bool m_visible{ false };
        bool m_enabled{ false };
        bool m_focused{ false };
        bool m_mouse_focus{ false };
        std::string m_tooltip{};
        int m_font_size{ 0 };
        gui::Cursor m_cursor{};
    };
}
