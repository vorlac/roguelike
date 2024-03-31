#pragma once
#include <memory>
#include <string>

#include "core/assert.hpp"
#include "core/ui/canvas.hpp"
#include "core/ui/layouts/advanced_grid_layout.hpp"
#include "core/ui/layouts/box_layout.hpp"
#include "core/ui/theme.hpp"
#include "core/ui/widgets/checkbox.hpp"
#include "core/ui/widgets/combobox.hpp"
#include "core/ui/widgets/label.hpp"
#include "core/ui/widgets/scroll_dialog.hpp"
#include "core/ui/widgets/textbox.hpp"
#include "ds/shared.hpp"
#include "ds/vector2d.hpp"

namespace rl::ui {
    namespace detail {
        template <typename T, typename U = std::true_type>
        class FormWidget
        {
        };
    }

    class FormHelper
    {
    public:
        explicit FormHelper(Canvas* screen)
            : m_ui_canvas{ screen }
        {
        }

        ScrollableDialog* add_dialog(ds::point<f32>&& pos, const std::string& title = "Untitled")
        {
            runtime_assert(m_ui_canvas != nullptr, "invalid dialog");

            m_dialog = new ScrollableDialog{ m_ui_canvas, title };
            // m_dialog->set_layout(new BoxLayout{ Orientation::Horizontal });

            m_layout = new AdvancedGridLayout({ 0, 0, 0, 0 }, {}, 10.0f);
            // m_layout->set_margin(10.0f);

            m_dialog->set_position(std::move(pos));
            m_dialog->set_visible(true);

            return m_dialog;
        }

        Label* add_group(const std::string& caption)
        {
            const Theme* theme{ m_dialog->theme() };
            const auto label{ new Label{ m_dialog, caption } };

            m_layout->append_row(theme->form_pre_group_spacing);
            m_layout->append_row(0);
            m_layout->set_anchor(label, Anchor{ 0, m_layout->row_count() - 1, 4, 1 });
            m_layout->append_row(theme->form_post_group_spacing);

            return label;
        }

        template <typename T>
        detail::FormWidget<T>* add_variable(const std::string& label_text,
                                            const std::function<void(const T&)>& setter,
                                            const std::function<T()>& getter, bool editable = true)
        {
            const Theme* theme{ m_dialog->theme() };
            const Label* label = new Label{ m_dialog, label_text };

            auto widget{ new detail::FormWidget<T>{ m_dialog } };
            auto refresh = [widget, getter] {
                T value{ getter() };
                T current{ widget->value() };

                if constexpr (std::floating_point<T>)
                {
                    if (std::fabs(value - current) <=
                        std::numeric_limits<decltype(value - current)>::epsilon())
                        widget->set_value(value);
                }
                else
                {
                    if (value != current)
                        widget->set_value(value);
                }
            };

            refresh();
            widget->set_callback(setter);
            widget->set_editable(editable);

            ds::dims<f32> fs{ widget->fixed_size() };
            widget->set_fixed_size(ds::dims{
                math::equal(fs.width, 0.0f) ? m_fixed_size.width : fs.width,
                math::equal(fs.height, 0.0f) ? m_fixed_size.height : fs.height,
            });

            m_refresh_callbacks.push_back(refresh);

            if (m_layout->row_count() > 0)
                m_layout->append_row(theme->form_variable_spacing);

            m_layout->append_row(0);
            m_layout->set_anchor(label, Anchor{ 1, m_layout->row_count() - 1 });
            m_layout->set_anchor(widget, Anchor{ 3, m_layout->row_count() - 1 });

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

        Button* add_button(std::string&& label, const std::function<void()>& cb)
        {
            const Theme* theme{ m_dialog->theme() };
            const auto button{ new Button{ m_dialog, std::move(label) } };

            button->set_callback(cb);
            if (m_layout->row_count() > 0)
                m_layout->append_row(theme->form_variable_spacing);

            m_layout->append_row(0);
            m_layout->set_anchor(button, Anchor{ 1, m_layout->row_count() - 1, 3, 1 });

            return button;
        }

        void add_widget(const std::string& label_text, const Widget* widget)
        {
            const Theme* theme{ m_dialog->theme() };
            m_layout->append_row(0);

            if (label_text.empty())
                m_layout->set_anchor(widget, Anchor{ 1, m_layout->row_count() - 1, 3, 1 });
            else
            {
                const Label* label = new Label{ m_dialog, label_text };
                m_layout->set_anchor(label, Anchor{ 1, m_layout->row_count() - 1 });
                m_layout->set_anchor(widget, Anchor{ 3, m_layout->row_count() - 1 });
            }
        }

        void refresh() const
        {
            for (const auto& callback : m_refresh_callbacks)
                callback();
        }

        ScrollableDialog* dialog()
        {
            return m_dialog;
        }

        void set_dialog(ScrollableDialog* dialog)
        {
            m_dialog = dialog;
            m_layout = dynamic_cast<AdvancedGridLayout*>(dialog->layout());
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

    protected:
        ds::shared<Canvas> m_ui_canvas{ nullptr };
        ds::shared<ScrollableDialog> m_dialog{ nullptr };
        // ds::shared<VerticalScrollPanel> m_dialog{ nullptr };
        ds::shared<Widget> m_container{ nullptr };
        ds::shared<AdvancedGridLayout> m_layout{ nullptr };
        std::vector<std::function<void()>> m_refresh_callbacks;
        ds::dims<f32> m_fixed_size{ 0.0f, 0.0f };
    };

    namespace detail {
        template <>
        class FormWidget<bool, std::true_type> final : public CheckBox
        {
        public:
            explicit FormWidget(Widget* p)
                : CheckBox{ p, "" }
            {
                this->set_fixed_width(20);
            }

            void set_value(const bool v)
            {
                this->set_checked(v);
            }

            void set_editable(const bool e)
            {
                this->set_enabled(e);
            }

            bool value() const
            {
                return this->checked();
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_enum<T>::type> final : public ComboBox
        {
        public:
            explicit FormWidget(Widget* p)
                : ComboBox{ p }
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
                ComboBox::set_callback([cb](u32 v) {
                    cb(static_cast<T>(v));
                });
            }

            void set_editable(const bool e)
            {
                this->set_enabled(e);
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_integral<T>::type> final : public IntBox<T>
        {
        public:
            explicit FormWidget(Widget* p)
                : IntBox<T>{ p }
            {
                this->set_alignment(TextBox::Alignment::Right);
            }
        };

        template <typename T>
        class FormWidget<T, typename std::is_floating_point<T>::type> final : public FloatBox<T>
        {
        public:
            explicit FormWidget(Widget* p)
                : FloatBox<T>{ p }
            {
                this->set_alignment(TextBox::Alignment::Right);
            }
        };

        template <>
        class FormWidget<std::string, std::true_type> final : public TextBox
        {
        public:
            explicit FormWidget(Widget* p)
                : TextBox{ p }
            {
                this->set_alignment(TextBox::Alignment::Left);
            }

            void set_callback(const std::function<void(const std::string&)>& cb)
            {
                TextBox::set_callback([cb](const std::string& str) {
                    cb(str);
                    return true;
                });
            }
        };
    }
}
