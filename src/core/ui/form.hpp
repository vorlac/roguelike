#pragma once
#include <string>

#include "core/ui/checkbox.hpp"
// #include "core/ui/colorpicker.hpp"
#include "core/assert.hpp"
#include "core/ui/combobox.hpp"
#include "core/ui/dialog.hpp"
#include "core/ui/gui_canvas.hpp"
#include "core/ui/label.hpp"
#include "core/ui/layout.hpp"
#include "core/ui/textbox.hpp"
#include "ds/color.hpp"
#include "ds/dims.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"

namespace rl::ui {
    namespace detail {

        template <typename T, typename sfinae = std::true_type>
        class FormWidget
        {
        };
    }

    class FormHelper
    {
    public:
        FormHelper(ui::UICanvas* screen)
            : m_ui_canvas{ screen }
        {
        }

        ui::Dialog* add_dialog(const ds::point<i32>& pos, const std::string& title = "Untitled")
        {
            runtime_assert(m_ui_canvas != nullptr, "invalid dialog");

            m_window = new ui::Dialog{ m_ui_canvas, title };
            m_layout = new AdvancedGridLayout{ { 10, 0, 10, 0 }, {} };

            m_layout->set_margin(10);
            m_layout->set_col_stretch(2, 1);
            m_window->set_position(pos);
            m_window->set_layout(m_layout);
            m_window->set_visible(true);

            return m_window;
        }

        /// Add a new group that may contain several sub-widgets
        ui::Label* add_group(const std::string& caption)
        {
            ui::Label* label{ new ui::Label{
                m_window,
                caption,
                m_group_font_name,
                m_group_font_size,
            } };

            m_layout->append_row(          // add spacing to account for the header if it's
                m_layout->row_count() > 0  // the first group insert otherwise just offset downw
                    ? m_pre_group_spacing  // by offsetting by the default pre group spacing
                    : m_window->header_height());

            m_layout->append_row(0);
            m_layout->set_anchor(label, ui::Anchor{ 0, m_layout->row_count() - 1, 4, 1 });
            m_layout->append_row(m_post_group_spacing);

            return label;
        }

        /// Add a new data widget controlled using custom getter/setter functions
        template <typename T>
        detail::FormWidget<T>* add_variable(const std::string& label,
                                            const std::function<void(const T&)>& setter,
                                            const std::function<T()>& getter, bool editable = true)
        {
            ui::Label* label_w{ new ui::Label{
                m_window,
                label,
                m_label_font_name,
                m_label_font_size,
            } };

            auto widget{ new detail::FormWidget<T>{ m_window } };
            auto refresh = [widget, getter] {
                T value{ getter() };
                T current{ widget->value() };

                if (value != current)
                    widget->set_value(value);
            };

            refresh();

            widget->set_callback(setter);
            widget->set_editable(editable);
            widget->set_font_size(m_widget_font_size);

            ds::dims<f32> fs{ widget->fixed_size() };
            widget->set_fixed_size(ds::dims<f32>{
                fs.width != 0.0f ? fs.width : m_fixed_size.width,
                fs.height != 0.0f ? fs.height : m_fixed_size.height,
            });

            m_refresh_callbacks.push_back(refresh);

            if (m_layout->row_count() > 0)
                m_layout->append_row(m_variable_spacing);

            m_layout->append_row(0);
            m_layout->set_anchor(label_w, ui::Anchor{ 1, m_layout->row_count() - 1 });
            m_layout->set_anchor(widget, ui::Anchor{ 3, m_layout->row_count() - 1 });

            return widget;
        }

        template <typename T>
        detail::FormWidget<T>* add_variable(const std::string& label, T& value, bool editable = true)
        {
            return this->add_variable<T>(
                label,
                [&](const T& v) {
                    value = v;
                },
                [&]() -> T {
                    return value;
                },
                editable);
        }

        ui::Button* add_button(const std::string& label, const std::function<void()>& cb)
        {
            ui::Button* button{ new Button{ m_window, label } };

            button->set_callback(cb);
            button->set_fixed_height(25);

            if (m_layout->row_count() > 0)
                m_layout->append_row(m_variable_spacing);

            m_layout->append_row(0);
            m_layout->set_anchor(button, ui::Anchor{ 1, m_layout->row_count() - 1, 3, 1 });

            return button;
        }

        void add_widget(const std::string& label_text, ui::Widget* widget)
        {
            m_layout->append_row(0);
            if (label_text.empty())
                m_layout->set_anchor(widget, ui::Anchor{ 1, m_layout->row_count() - 1, 3, 1 });
            else
            {
                ui::Label* label{ new ui::Label{ m_window, label_text, m_label_font_name,
                                                 m_label_font_size } };
                m_layout->set_anchor(label, ui::Anchor{ 1, m_layout->row_count() - 1 });
                m_layout->set_anchor(widget, ui::Anchor{ 3, m_layout->row_count() - 1 });
            }
        }

        void refresh()
        {
            for (const auto& callback : m_refresh_callbacks)
                callback();
        }

        ui::Dialog* window()
        {
            return m_window;
        }

        void set_window(ui::Dialog* window)
        {
            m_window = window;
            m_layout = dynamic_cast<AdvancedGridLayout*>(window->layout());
            runtime_assert(m_layout == nullptr, "invalid layout");
        }

        void set_fixed_size(const ds::dims<f32>& fw)
        {
            m_fixed_size = fw;
        }

        ds::dims<f32> fixed_size()
        {
            return m_fixed_size;
        }

        const std::string& group_font_name() const
        {
            return m_group_font_name;
        }

        void set_group_font_name(const std::string& name)
        {
            m_group_font_name = name;
        }

        const std::string& label_font_name() const
        {
            return m_label_font_name;
        }

        void set_label_font_name(const std::string& name)
        {
            m_label_font_name = name;
        }

        f32 group_font_size() const
        {
            return m_group_font_size;
        }

        void set_group_font_size(f32 value)
        {
            m_group_font_size = value;
        }

        f32 label_font_size() const
        {
            return m_label_font_size;
        }

        void set_label_font_size(f32 value)
        {
            m_label_font_size = value;
        }

        f32 widget_font_size() const
        {
            return m_widget_font_size;
        }

        void set_widget_font_size(f32 value)
        {
            m_widget_font_size = value;
        }

    protected:
        ds::shared<ui::UICanvas> m_ui_canvas{};
        ds::shared<ui::Dialog> m_window{};
        ds::shared<AdvancedGridLayout> m_layout{};
        std::vector<std::function<void()>> m_refresh_callbacks;
        std::string m_group_font_name{ font::name::sans_bold };
        std::string m_label_font_name{ font::name::sans };
        ds::dims<f32> m_fixed_size{ 0.0f, 20.0f };
        f32 m_group_font_size{ 20.0f };
        f32 m_label_font_size{ 16.0f };
        f32 m_widget_font_size{ 16.0f };
        f32 m_pre_group_spacing{ 15.0f };
        f32 m_post_group_spacing{ 5.0f };
        f32 m_variable_spacing{ 5.0f };
    };

    namespace detail {

        template <>
        class FormWidget<bool, std::true_type> : public ui::CheckBox
        {
        public:
            FormWidget(ui::Widget* p)
                : ui::CheckBox{ p, "" }
            {
                this->set_fixed_width(20);
            }

            void set_value(bool v)
            {
                this->set_checked(v);
            }

            void set_editable(bool e)
            {
                this->set_enabled(e);
            }

            bool value() const
            {
                return this->checked();
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_enum<T>::type> : public ComboBox
        {
        public:
            FormWidget(ui::Widget* p)
                : ui::ComboBox{ p }
            {
            }

            T value() const
            {
                return static_cast<T>(this->selected_index());
            }

            void set_value(T value)
            {
                this->set_selected_index(value);
                m_selected_index = value;
            }

            void set_callback(const std::function<void(const T&)>& cb)
            {
                ui::ComboBox::set_callback([cb](i32 v) {
                    cb(static_cast<T>(v));
                });
            }

            void set_editable(bool e)
            {
                this->set_enabled(e);
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_integral<T>::type> : public IntBox<T>
        {
        public:
            FormWidget(ui::Widget* p)
                : IntBox<T>{ p }
            {
                this->set_alignment(TextBox::Alignment::Right);
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_floating_point<T>::type> : public FloatBox<T>
        {
        public:
            FormWidget(ui::Widget* p)
                : FloatBox<T>{ p }
            {
                this->set_alignment(TextBox::Alignment::Right);
            }
        };

        template <>
        class FormWidget<std::string, std::true_type> : public ui::TextBox
        {
        public:
            FormWidget(ui::Widget* p)
                : ui::TextBox{ p }
            {
                this->set_alignment(TextBox::Alignment::Left);
            }

            void set_callback(const std::function<void(const std::string&)>& cb)
            {
                ui::TextBox::set_callback([cb](const std::string& str) {
                    cb(str);
                    return true;
                });
            }
        };
    }
}
