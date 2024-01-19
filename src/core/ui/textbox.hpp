#pragma once

#include <cstdio>
#include <functional>
#include <sstream>
#include <string>

#include "core/keyboard.hpp"
#include "core/mouse.hpp"
#include "core/ui/widget.hpp"
#include "ds/dims.hpp"
#include "ds/point.hpp"
#include "utils/concepts.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {

    // Fancy text box with builtin regular expression-based validation.
    // This class overrides widget::m_icon_extra_scale to be 0.8f, which
    // affects all subclasses of this widget. Subclasses must explicitly set
    // a different value if needed (e.g., in their constructor).
    class TextBox : public ui::Widget
    {
    public:
        enum class Alignment {
            Left,
            Center,
            Right
        };

        TextBox(ui::Widget* parent, const std::string& value = "Untitled");

        bool editable() const;
        bool spinnable() const;
        const std::string& value() const;
        const std::string& default_value() const;
        Alignment alignment() const;
        const std::string& units() const;
        i32 units_image() const;
        const std::string& format() const;
        const std::string& placeholder() const;
        const std::function<bool(const std::string& str)>& callback() const;

        void set_editable(bool editable);
        void set_spinnable(bool spinnable);
        void set_value(const std::string& value);
        void set_default_value(const std::string& default_value);
        void set_alignment(Alignment align);
        void set_units(const std::string& units);
        void set_units_image(int image);
        void set_format(const std::string& format);
        void set_placeholder(const std::string& placeholder);
        void set_callback(const std::function<bool(const std::string& str)>& callback);

    public:
        virtual bool on_mouse_entered(const Mouse& mouse) override;
        virtual bool on_mouse_exited(const Mouse& mouse) override;

        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override;

        virtual bool on_mouse_move(const Mouse& mouse, const Keyboard& kb) override;
        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override;

        virtual bool on_focus_gained() override;
        virtual bool on_focus_lost() override;

        virtual bool on_key_pressed(const Keyboard& kb) override;
        virtual bool on_key_released(const Keyboard& kb) override;
        virtual bool on_character_input(const Keyboard& kb) override;

    public:
        virtual void set_theme(ui::Theme* theme) override;
        virtual ds::dims<f32> preferred_size(vg::NVGcontext* nvg_context) const override;
        virtual void draw(vg::NVGcontext* nvg_context) override;

    protected:
        bool check_format(const std::string& input, const std::string& format);
        bool copy_selection();
        bool delete_selection();

        void paste_from_clipboard();
        void update_cursor(vg::NVGcontext* nvg_context, f32 last_x,
                           const vg::NVGglyphPosition* glyphs, i32 size);

        f32 cursor_index_to_position(i32 index, f32 last_x, const vg::NVGglyphPosition* glyphs,
                                     i32 size);
        i32 position_to_cursor_index(i32 pos_x, f32 last_x, const vg::NVGglyphPosition* glyphs,
                                     i32 size);

        // The location (if any) for the spin area.
        enum class SpinArea {
            None,
            Top,
            Bottom
        };

        SpinArea spin_area(ds::point<i32> pos) const;

    protected:
        bool m_editable{ false };
        bool m_spinnable{ false };
        bool m_committed{ false };
        bool m_valid_format{ false };

        std::string m_value{};
        std::string m_default_value{};
        std::string m_units{};
        std::string m_format{};
        std::string m_value_temp{};
        std::string m_placeholder{};

        i32 m_cursor_pos{ -1 };
        i32 m_selection_pos{ -1 };
        i32 m_mouse_down_modifier{ 0 };
        i32 m_units_image{ -1 };
        f32 m_text_offset{ 0.0f };
        f32 m_last_click{ 0.0f };

        ds::point<i32> m_mouse_pos{ -1, -1 };
        ds::point<i32> m_mouse_down_pos{ -1, -1 };
        ds::point<i32> m_mouse_drag_pos{ -1, -1 };

        Alignment m_alignment{};

        std::function<bool(const std::string& str)> m_callback;
    };

    // A specialization of TextBox for representing integral values.
    template <constraint::integer Scalar>
    class IntBox : public ui::TextBox
    {
    public:
        IntBox(ui::Widget* parent, Scalar value = (Scalar)0)
            : TextBox(parent)
        {
            this->set_default_value("0");
            this->set_format(std::is_signed<Scalar>::value ? "[-]?[0-9]*" : "[0-9]*");
            this->set_value_increment(1);
            this->set_min_max_values(std::numeric_limits<Scalar>::lowest(),
                                     std::numeric_limits<Scalar>::max());
            this->set_value(value);
            this->set_spinnable(false);
        }

        Scalar value() const
        {
            std::istringstream iss(TextBox::value());
            Scalar value = 0;
            iss >> value;
            return value;
        }

        void set_value(Scalar value)
        {
            Scalar clamped_value = std::min(std::max(value, m_min_value), m_max_value);
            TextBox::set_value(std::to_string(clamped_value));
        }

        void set_callback(const std::function<void(Scalar)>& cb)
        {
            TextBox::set_callback([cb, this](const std::string& str) {
                std::istringstream iss(str);
                Scalar value = 0;
                iss >> value;
                set_value(value);
                cb(value);
                return true;
            });
        }

        void set_value_increment(Scalar incr)
        {
            m_value_increment = incr;
        }

        void set_min_value(Scalar min_value)
        {
            m_min_value = min_value;
        }

        void set_max_value(Scalar max_value)
        {
            m_max_value = max_value;
        }

        void set_min_max_values(Scalar min_value, Scalar max_value)
        {
            set_min_value(min_value);
            set_max_value(max_value);
        }

        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override
        {
            if (m_editable || m_spinnable)
                m_mouse_down_value = this->value();

            SpinArea area{ this->spin_area(mouse.pos()) };
            if (m_spinnable && area != SpinArea::None && !this->focused())
            {
                if (area == SpinArea::Top)
                {
                    this->set_value(value() + m_value_increment);
                    if (m_callback != nullptr)
                        m_callback(m_value);
                }
                else if (area == SpinArea::Bottom)
                {
                    this->set_value(this->value() - m_value_increment);
                    if (m_callback != nullptr)
                        m_callback(m_value);
                }

                return true;
            }

            return TextBox::on_mouse_button_pressed(mouse, kb);
        }

        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override
        {
            return TextBox::on_mouse_button_released(mouse, kb);
        }

        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override
        {
            if (TextBox::on_mouse_drag(mouse, kb))
                return true;

            if (m_spinnable && !this->focused() && mouse.is_button_held(Mouse::Button::Right) &&
                m_mouse_down_pos.x != -1)
            {
                i32 value_delta{ static_cast<int>((mouse.pos().x - m_mouse_down_pos.x) / 10.0f) };
                this->set_value(m_mouse_down_value + value_delta * m_value_increment);

                if (m_callback != nullptr)
                    m_callback(m_value);

                return true;
            }

            return false;
        }

        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override
        {
            if (ui::Widget::on_mouse_scroll(mouse, kb))
                return true;

            if (m_spinnable && !this->focused())
            {
                i32 value_delta{ (mouse.wheel().y > 0) ? 1 : -1 };
                this->set_value(this->value() + value_delta * m_value_increment);

                if (m_callback != nullptr)
                    m_callback(m_value);

                return true;
            }

            return false;
        }

    private:
        Scalar m_mouse_down_value{};
        Scalar m_value_increment{};
        Scalar m_min_value{};
        Scalar m_max_value{};
    };

    // A specialization of TextBox representing floating point values. The
    // emplate parametersshould a be floating point type, e.g. `float` or
    // `double`.
    template <constraint::floating_point Scalar>
    class FloatBox : public ui::TextBox
    {
    public:
        FloatBox(ui::Widget* parent, Scalar value = (Scalar)0.f)
            : ui::TextBox(parent)
        {
            m_number_format = sizeof(Scalar) == sizeof(float) ? "%.4g" : "%.7g";
            this->set_default_value("0");
            this->set_format("[-+]?[0-9]*\\.?[0-9]+([e_e][-+]?[0-9]+)?");
            this->set_value_increment((Scalar)0.1);
            this->set_min_max_values(std::numeric_limits<Scalar>::lowest(),
                                     std::numeric_limits<Scalar>::max());
            this->set_value(value);
            this->set_spinnable(false);
        }

        std::string number_format() const
        {
            return m_number_format;
        }

        void number_format(const std::string& format)
        {
            m_number_format = format;
        }

        Scalar value() const
        {
            return (Scalar)std::stod(TextBox::value());
        }

        void set_value(Scalar value)
        {
            Scalar clamped_value = std::min(std::max(value, m_min_value), m_max_value);
            char buffer[50];
            std::snprintf(buffer, 50, m_number_format.c_str(), clamped_value);
            TextBox::set_value(buffer);
        }

        void set_callback(const std::function<void(Scalar)>& cb)
        {
            TextBox::set_callback([cb, this](const std::string& str) {
                Scalar scalar = (Scalar)std::stod(str);
                set_value(scalar);
                cb(scalar);
                return true;
            });
        }

        void set_value_increment(Scalar incr)
        {
            m_value_increment = incr;
        }

        void set_min_value(Scalar min_value)
        {
            m_min_value = min_value;
        }

        void set_max_value(Scalar max_value)
        {
            m_max_value = max_value;
        }

        void set_min_max_values(Scalar min_value, Scalar max_value)
        {
            set_min_value(min_value);
            set_max_value(max_value);
        }

        virtual bool on_mouse_button_pressed(const Mouse& mouse, const Keyboard& kb) override
        {
            if (m_editable || m_spinnable)
                m_mouse_down_value = this->value();

            SpinArea area{ this->spin_area(mouse.pos()) };
            if (m_spinnable && area != SpinArea::None && !this->focused())
            {
                if (area == SpinArea::Top)
                {
                    this->set_value(value() + m_value_increment);
                    if (m_callback != nullptr)
                        m_callback(m_value);
                }
                else if (area == SpinArea::Bottom)
                {
                    this->set_value(this->value() - m_value_increment);
                    if (m_callback != nullptr)
                        m_callback(m_value);
                }

                return true;
            }

            return TextBox::on_mouse_button_pressed(mouse, kb);
        }

        virtual bool on_mouse_button_released(const Mouse& mouse, const Keyboard& kb) override
        {
            SpinArea area{ this->spin_area(mouse.pos()) };
            return TextBox::on_mouse_button_released(mouse, kb);
        }

        virtual bool on_mouse_drag(const Mouse& mouse, const Keyboard& kb) override
        {
            if (TextBox::on_mouse_drag(mouse, kb))
                return true;

            if (m_spinnable && !this->focused() && mouse.is_button_held(Mouse::Button::Right) &&
                m_mouse_down_pos.x != -1)
            {
                i32 value_delta{ static_cast<i32>((mouse.pos().x - m_mouse_down_pos.x) / 10.0f) };
                this->set_value(m_mouse_down_value + value_delta * m_value_increment);

                if (m_callback != nullptr)
                    m_callback(m_value);

                return true;
            }
            return false;
        }

        virtual bool on_mouse_scroll(const Mouse& mouse, const Keyboard& kb) override
        {
            if (ui::Widget::on_mouse_scroll(mouse, kb))
                return true;

            if (m_spinnable && !this->focused())
            {
                i32 value_delta{ (mouse.wheel().y > 0) ? 1 : -1 };
                this->set_value(this->value() + value_delta * m_value_increment);

                if (m_callback != nullptr)
                    m_callback(m_value);

                return true;
            }

            return false;
        }

    private:
        std::string m_number_format{};
        Scalar m_mouse_down_value{};
        Scalar m_value_increment{};
        Scalar m_min_value{};
        Scalar m_max_value{};
    };
}
