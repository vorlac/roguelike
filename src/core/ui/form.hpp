// #pragma once
// #include <string>
//
// #include "core/ui/checkbox.hpp"
//// #include "core/ui/colorpicker.hpp"
// #include "core/assert.hpp"
// #include "core/ui/combobox.hpp"
// #include "core/ui/dialog.hpp"
// #include "core/ui/gui_canvas.hpp"
// #include "core/ui/label.hpp"
// #include "core/ui/layout.hpp"
// #include "core/ui/textbox.hpp"
// #include "ds/dims.hpp"
// #include "ds/shared.hpp"
//
// namespace rl::ui::detail {
//
//     /**
//      * \class FormWidget formhelper.h nanogui/formhelper.h
//      *
//      * \brief A template wrapper class for assisting in the creation of various form widgets.
//      *
//      * \rst
//      * The partial template specializations are:
//      *
//      * - Inheritance from :class:`nanogui::ComboBox` for ``enum`` types:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <typename T>
//      *      class FormWidget<T, typename std::is_enum<T>::type> : public ComboBox
//      *
//      * - Inheritance from :class:`nanogui::IntBox` for integral types:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <typename T>
//      *      class FormWidget<T, typename std::is_integral<T>::type> : public IntBox<T>
//      *
//      * - Inheritance from :class:`nanogui::FloatBox` for floating point types:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <typename T>
//      *      class FormWidget<T, typename std::is_floating_point<T>::type> : public FloatBox<T>
//      *
//      * The full template specializations are:
//      *
//      * - Inheritance from :class:`nanogui::CheckBox` for booleans:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <>
//      *      class FormWidget<bool, std::true_type> : public CheckBox
//      *
//      * - Inheritance from :class:`nanogui::TextBox` for strings:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <>
//      *      class FormWidget<std::string, std::true_type> : public TextBox
//      *
//      * - Inheritance from :class:`nanogui::ColorPicker` for :class:`nanogui::Color` types:
//      *
//      *   .. code-block:: cpp
//      *
//      *      template <>
//      *      class FormWidget<Color, std::true_type> : public ColorPicker
//      *
//      * Please refer to the bottom of :ref:`program_listing_file_nanogui_formhelper.h`
//      * for the implementation details.
//      * \endrst
//      */
//     template <typename T, typename sfinae = std::true_type>
//     class FormWidget
//     {
//     };
// }
//
///**
// * \class FormHelper formhelper.h nanogui/formhelper.h
// *
// * \brief Convenience class to create simple AntTweakBar-style layouts that
// *        expose variables of various types using NanoGUI widgets
// *
// * **Example**:
// *
// * \rst
// * .. code-block:: cpp
// *
// *    // [ ... initialize NanoGUI, construct screen ... ]
// *
// *    FormHelper* h = new FormHelper(screen);
// *
// *    // Add a new windows widget
// *    h->add_window(ds::dims<i32>(10,10),"Menu");
// *
// *    // Start a new group
// *    h->add_group("Group 1");
// *
// *    // Expose an integer variable by reference
// *    h->add_variable("integer variable", a_int);
// *
// *    // Expose a float variable via setter/getter functions
// *    h->add_variable(
// *      [&](float value) { a_float = value; },
// *      [&]() { return *a_float; },
// *      "float variable");
// *
// *    // add a new button
// *    h->add_button("Button", [&]() { std::cout << "Button pressed" << std::endl; });
// *
// * \endrst
// */
// class FormHelper
//{
// public:
//    /// Create a helper class to construct NanoGUI widgets on the given screen
//    FormHelper(ui::UICanvas* screen)
//        : m_screen(screen)
//    {
//    }
//
//    /// Add a new top-level window
//    ui::Dialog* add_window(const ds::dims<i32>& pos, const std::string& title = "Untitled")
//    {
//        runtime_assert(m_screen != nullptr, "invalid dialog");
//        m_window = new ui::Dialog(m_screen, title);
//        m_layout = new AdvancedGridLayout({ 10, 0, 10, 0 }, {});
//        m_layout->set_margin(10);
//        m_layout->set_col_stretch(2, 1);
//        m_window->set_position(pos);
//        m_window->set_layout(m_layout);
//        m_window->set_visible(true);
//        return m_window;
//    }
//
//    /// Add a new group that may contain several sub-widgets
//    ui::Label* add_group(const std::string& caption)
//    {
//        ui::Label* label = new ui::Label(m_window, caption, m_group_font_name, m_group_font_size);
//        if (m_layout->row_count() > 0)
//            m_layout->append_row(m_pre_group_spacing); /* Spacing */
//        m_layout->append_row(0);
//        m_layout->set_anchor(label, AdvancedGridLayout::Anchor(0, m_layout->row_count() - 1, 4,
//        1)); m_layout->append_row(m_post_group_spacing); return label;
//    }
//
//    /// Add a new data widget controlled using custom getter/setter functions
//    template <typename Type>
//    detail::FormWidget<Type>* add_variable(
//        const std::string& label, const std::function<void(const Type&)>& setter,
//        const std::function<Type()>& getter, bool editable = true)
//    {
//        ui::Label* label_w = new ui::Label(m_window, label, m_label_font_name, m_label_font_size);
//        auto widget = new detail::FormWidget<Type>(m_window);
//        auto refresh = [widget, getter] {
//            Type value = getter(), current = widget->value();
//            if (value != current)
//                widget->set_value(value);
//        };
//        refresh();
//        widget->set_callback(setter);
//        widget->set_editable(editable);
//        widget->set_font_size(m_widget_font_size);
//        ds::dims<i32> fs = widget->fixed_size();
//        widget->set_fixed_size(ds::dims<i32>(fs.x() != 0 ? fs.x() : m_fixed_size.x(),
//                                             fs.y() != 0 ? fs.y() : m_fixed_size.y()));
//        m_refresh_callbacks.push_back(refresh);
//        if (m_layout->row_count() > 0)
//            m_layout->append_row(m_variable_spacing);
//        m_layout->append_row(0);
//        m_layout->set_anchor(label_w, AdvancedGridLayout::Anchor(1, m_layout->row_count() - 1));
//        m_layout->set_anchor(widget, AdvancedGridLayout::Anchor(3, m_layout->row_count() - 1));
//        return widget;
//    }
//
//    /// Add a new data widget that exposes a raw variable in memory
//    template <typename Type>
//    detail::FormWidget<Type>* add_variable(const std::string& label, Type& value,
//                                           bool editable = true)
//    {
//        return add_variable<Type>(
//            label,
//            [&](const Type& v) {
//                value = v;
//            },
//            [&]() -> Type {
//                return value;
//            },
//            editable);
//    }
//
//    /// Add a button with a custom callback
//    Button* add_button(const std::string& label, const std::function<void()>& cb)
//    {
//        Button* button = new Button(m_window, label);
//        button->set_callback(cb);
//        button->set_fixed_height(25);
//        if (m_layout->row_count() > 0)
//            m_layout->append_row(m_variable_spacing);
//        m_layout->append_row(0);
//        m_layout->set_anchor(button, AdvancedGridLayout::Anchor(1, m_layout->row_count() - 1, 3,
//        1)); return button;
//    }
//
//    /// Add an arbitrary (optionally labeled) widget to the layout
//    void add_widget(const std::string& label, Widget* widget)
//    {
//        m_layout->append_row(0);
//        if (label == "")
//        {
//            m_layout->set_anchor(widget,
//                                 AdvancedGridLayout::Anchor(1, m_layout->row_count() - 1, 3, 1));
//        }
//        else
//        {
//            ui::Label* label_w = new ui::Label(m_window, label, m_label_font_name,
//                                               m_label_font_size);
//            m_layout->set_anchor(label_w, AdvancedGridLayout::Anchor(1, m_layout->row_count() -
//            1)); m_layout->set_anchor(widget, AdvancedGridLayout::Anchor(3, m_layout->row_count()
//            - 1));
//        }
//    }
//
//    /// Cause all widgets to re-synchronize with the underlying variable state
//    void refresh()
//    {
//        for (const auto& callback : m_refresh_callbacks)
//            callback();
//    }
//
//    /// Access the currently active \ref ui::Dialog instance
//    ui::Dialog* window()
//    {
//        return m_window;
//    }
//
//    /// Set the active \ref ui::Dialog instance.
//    void set_window(ui::Dialog* window)
//    {
//        m_window = window;
//        m_layout = dynamic_cast<AdvancedGridLayout*>(window->layout());
//        if (m_layout == nullptr)
//            throw std::runtime_error("Internal error: window has an incompatible layout!");
//    }
//
//    /// Specify a fixed size for newly added widgets
//    void set_fixed_size(const ds::dims<i32>& fw)
//    {
//        m_fixed_size = fw;
//    }
//
//    /// The current fixed size being used for newly added widgets.
//    ds::dims<i32> fixed_size()
//    {
//        return m_fixed_size;
//    }
//
//    /// The font name being used for group headers.
//    const std::string& group_font_name() const
//    {
//        return m_group_font_name;
//    }
//
//    /// Sets the font name to be used for group headers.
//    void set_group_font_name(const std::string& name)
//    {
//        m_group_font_name = name;
//    }
//
//    /// The font name being used for labels.
//    const std::string& label_font_name() const
//    {
//        return m_label_font_name;
//    }
//
//    /// Sets the font name being used for labels.
//    void set_label_font_name(const std::string& name)
//    {
//        m_label_font_name = name;
//    }
//
//    /// The size of the font being used for group headers.
//    int group_font_size() const
//    {
//        return m_group_font_size;
//    }
//
//    /// Sets the size of the font being used for group headers.
//    void set_group_font_size(int value)
//    {
//        m_group_font_size = value;
//    }
//
//    /// The size of the font being used for labels.
//    int label_font_size() const
//    {
//        return m_label_font_size;
//    }
//
//    /// Sets the size of the font being used for labels.
//    void set_label_font_size(int value)
//    {
//        m_label_font_size = value;
//    }
//
//    /// The size of the font being used for non-group / non-label widgets.
//    int widget_font_size() const
//    {
//        return m_widget_font_size;
//    }
//
//    /// Sets the size of the font being used for non-group / non-label widgets.
//    void set_widget_font_size(int value)
//    {
//        m_widget_font_size = value;
//    }
//
// protected:
//    /// A reference to the \ref nanogui::ui::UICanvas this FormHelper is assisting.
//    ds::shared<ui::UICanvas> m_screen;
//    /// A reference to the \ref nanogui::ui::Dialog this FormHelper is controlling.
//    ds::shared<ui::Dialog> m_window;
//    /// A reference to the \ref nanogui::AdvancedGridLayout this FormHelper is using.
//    ds::shared<AdvancedGridLayout> m_layout;
//    /// The callbacks associated with all widgets this FormHelper is managing.
//    std::vector<std::function<void()>> m_refresh_callbacks;
//    /// The group header font name.
//    std::string m_group_font_name = "sans-bold";
//    /// The label font name.
//    std::string m_label_font_name = "sans";
//    /// The fixed size for newly added widgets.
//    ds::dims<i32> m_fixed_size = ds::dims<i32>(0, 20);
//    /// The font size for group headers.
//    int m_group_font_size = 20;
//    /// The font size for labels.
//    int m_label_font_size = 16;
//    /// The font size for non-group / non-label widgets.
//    int m_widget_font_size = 16;
//    /// The spacing used **before** new groups.
//    int m_pre_group_spacing = 15;
//    /// The spacing used **after** each group.
//    int m_post_group_spacing = 5;
//    /// The spacing between all other widgets.
//    int m_variable_spacing = 5;
//};
//
// NAMESPACE_BEGIN(detail)
//
///**
// * A specialization for adding a CheckBox to a FormHelper.
// */
// template <>
// class FormWidget<bool, std::true_type> : public CheckBox
//{
// public:
//    /// Creates a new FormWidget with underlying type CheckBox.
//    FormWidget(Widget* p)
//        : CheckBox(p, "")
//    {
//        set_fixed_width(20);
//    }
//
//    /// Pass-through function for \ref nanogui::CheckBox::set_checked.
//    void set_value(bool v)
//    {
//        set_checked(v);
//    }
//
//    /// Pass-through function for \ref nanogui::Widget::set-enabled.
//    void set_editable(bool e)
//    {
//        set_enabled(e);
//    }
//
//    /// Returns the value of \ref nanogui::CheckBox::checked.
//    bool value() const
//    {
//        return checked();
//    }
//};
//
///**
// * A specialization for adding a ComboBox to a FormHelper.
// *
// * \tparam T
// *     The type being used inside the ComboBox.
// */
// template <typename T>
// class FormWidget<T, typename std::is_enum<T>::type> : public ComboBox
//{
// public:
//    /// Creates a new FormWidget with underlying type ComboBox.
//    FormWidget(Widget* p)
//        : ComboBox(p)
//    {
//    }
//
//    /// Pass-through function for \ref nanogui::ComboBox::selected_index.
//    T value() const
//    {
//        return (T)selected_index();
//    }
//
//    /// Pass-through function for \ref nanogui::ComboBox::set_selected_index.
//    void set_value(T value)
//    {
//        set_selected_index((int)value);
//        m_selected_index = (int)value;
//    }
//
//    /// Pass-through function for \ref nanogui::ComboBox::set_callback.
//    void set_callback(const std::function<void(const T&)>& cb)
//    {
//        ComboBox::set_callback([cb](int v) {
//            cb((T)v);
//        });
//    }
//
//    /// Pass-through function for \ref nanogui::Widget::set_enabled.
//    void set_editable(bool e)
//    {
//        set_enabled(e);
//    }
//};
//
///**
// * A specialization for adding an IntBox to a FormHelper.
// *
// * \tparam T
// *     The **integral** type being used for the IntBox.
// */
// template <typename T>
// class FormWidget<T, typename std::is_integral<T>::type> : public IntBox<T>
//{
// public:
//    /// Creates a new FormWidget with underlying type IntBox.
//    FormWidget(Widget* p)
//        : IntBox<T>(p)
//    {
//        this->set_alignment(TextBox::Alignment::Right);
//    }
//};
//
///**
// * A specialization for adding a FloatBox to a FormHelper.
// *
// * \tparam T
// *     The **floating point** type being used for the FloatBox.
// */
// template <typename T>
// class FormWidget<T, typename std::is_floating_point<T>::type> : public FloatBox<T>
//{
// public:
//    /// Creates a new FormWidget with underlying type FloatBox.
//    FormWidget(Widget* p)
//        : FloatBox<T>(p)
//    {
//        this->set_alignment(TextBox::Alignment::Right);
//    }
//};
//
///**
// * A specialization for adding a TextBox to a FormHelper.
// */
// template <>
// class FormWidget<std::string, std::true_type> : public TextBox
//{
// public:
//    /// Creates a new FormWidget with underlying type TextBox.
//    FormWidget(Widget* p)
//        : TextBox(p)
//    {
//        set_alignment(TextBox::Alignment::Left);
//    }
//
//    /// Pass-through function for \ref nanogui::TextBox::set_callback.
//    void set_callback(const std::function<void(const std::string&)>& cb)
//    {
//        TextBox::set_callback([cb](const std::string& str) {
//            cb(str);
//            return true;
//        });
//    }
//};
//
///**
// * A specialization for adding a ColorPicker to a FormHelper.
// */
// template <>
// class FormWidget<Color, std::true_type> : public ColorPicker
//{
// public:
//    /// Creates a new FormWidget with underlying type ColorPicker.
//    FormWidget(Widget* p)
//        : ColorPicker(p)
//    {
//    }
//
//    /// Pass-through function for \ref nanogui::ColorPicker::set_color.
//    void set_value(const Color& c)
//    {
//        set_color(c);
//    }
//
//    /// Pass-through function for \ref nanogui::Widget::set_enabled.
//    void set_editable(bool e)
//    {
//        set_enabled(e);
//    }
//
//    /// Returns the value of \ref nanogui::ColorPicker::color.
//    Color value() const
//    {
//        return color();
//    }
//};
