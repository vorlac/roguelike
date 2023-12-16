#pragma once

#include <functional>
#include <memory>
#include <sstream>

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * \class TextBox textbox.h sdl_gui/textbox.h
     *
     * \brief Fancy text box with builtin regular expression-based validation.
     */
    class TextBox : public Widget
    {
    public:
        /// How to align the text in the text box.
        enum class Alignment {
            Left,
            Center,
            Right
        };

        TextBox(Widget* parent, const std::string& value = "Untitled",
                const std::string& units = "");

        bool editable() const
        {
            return m_editable;
        }

        void set_editable(bool editable);

        bool spinnable() const
        {
            return m_spinnable;
        }

        void set_spinnable(bool spinnable)
        {
            m_spinnable = spinnable;
        }

        const std::string& value() const
        {
            return m_value;
        }

        void set_value(const std::string& value)
        {
            m_value = value;
            m_caption_texture.dirty = true;
        }

        const std::string& default_value() const
        {
            return m_default_value;
        }

        void set_default_value(const std::string& default_value)
        {
            m_default_value = default_value;
        }

        Alignment alignment() const
        {
            return m_alignment;
        }

        void set_alignment(Alignment align)
        {
            m_alignment = align;
        }

        TextBox& with_alignment(Alignment align)
        {
            set_alignment(align);
            return *this;
        }

        const std::string& units() const
        {
            return m_units;
        }

        void set_units(const std::string& units)
        {
            m_units = units;
        }

        int units_image() const
        {
            return m_units_image;
        }

        void set_units_image(int image)
        {
            m_units_image = image;
        }

        /// Return the underlying regular expression specifying valid formats
        const std::string& format() const
        {
            return m_format;
        }

        /// Specify a regular expression specifying valid formats
        void set_format(const std::string& format)
        {
            m_format = format;
        }

        /// Set the \ref Theme used to draw this widget
        void set_theme(Theme* theme) override;

        /// Set the change callback
        std::function<bool(const std::string& str)> callback() const
        {
            return m_callback;
        }

        void set_callback(const std::function<bool(const std::string& str)>& callback)
        {
            m_callback = callback;
        }

        bool mouse_button_event(const Vector2i& p, int button, bool down, int modifiers) override;
        bool mouse_motion_event(const Vector2i& p, const Vector2i& rel, int button,
                                int modifiers) override;
        bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                              int modifiers) override;
        bool focus_event(bool focused) override;
        bool kb_button_event(int key, int scancode, int action, int modifiers) override;
        bool kb_character_event(unsigned int codepoint) override;

        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        void draw(SDL3::SDL_Renderer* renderer) override;
        void draw_body(SDL3::SDL_Renderer* renderer);

    protected:
        bool check_format(const std::string& input, const std::string& format);
        bool copy_selection();
        void paste_from_clipboard();
        bool delete_selection();

        void update_cursor(float lastx, const std::string& str);
        float cursor_idx_to_position(int index, float lastx, const std::string& str);
        int position_to_cursor_idx(float posx, float lastx, const std::string& str);

        /// The location (if any) for the spin area.
        enum class SpinArea {
            None,
            Top,
            Bottom
        };
        SpinArea spin_area(const Vector2i& pos);

    protected:
        bool m_editable;
        bool m_spinnable;
        bool m_committed;
        std::string m_value;
        std::string m_default_value;
        Alignment m_alignment;
        std::string m_units;
        std::string m_format;
        int m_units_image;
        std::function<bool(const std::string& str)> m_callback;
        bool m_valid_format;
        std::string m_value_temp;
        int m_cursor_pos;
        int m_selection_pos;
        Vector2i m_mouse_pos;
        Vector2i m_mouse_down_pos;
        Vector2i m_mouse_drag_pos;
        int m_mouse_down_modifier;
        float m_text_offset;
        double m_last_click;
        int caret_last_tick_count = 0;

        Texture m_caption_texture;
        Texture m_units_texture;
        Texture m_temp_texture;

        struct AsyncTexture;
        using AsyncTexturePtr = std::shared_ptr<TextBox::AsyncTexture>;
        std::vector<TextBox::AsyncTexturePtr> m_textures;

        TextBox::AsyncTexturePtr m_curr_texture = nullptr;

    private:
        void draw_texture(TextBox::AsyncTexturePtr& texture, SDL3::SDL_Renderer* renderer);
    };

    /**
     * @brief A specialization of TextBox for representing integral values.
     *
     * Template parameters should be integral types, e.g. ``int``, ``long``,
     * ``uint32_t``, etc.
     */
    template <typename Scalar>
    class IntBox : public TextBox
    {
    public:
        IntBox(Widget* parent, Scalar value = (Scalar)0)
            : TextBox(parent)
        {
            this->set_default_value("0");
            this->set_format(std::is_signed<Scalar>::value ? "[-]?[0-9]*" : "[0-9]*");
            this->set_value_increment(1);
            this->set_min_max_value(std::numeric_limits<Scalar>::lowest(),
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
            Scalar clamped_val = std::min(std::max(value, m_min_value), m_max_value);
            TextBox::set_value(std::to_string(clamped_val));
        }

        void set_callback(const std::function<void(Scalar)>& cb)
        {
            TextBox::set_callback([cb, this](const std::string& str) {
                std::istringstream iss(str);
                Scalar value = 0;
                iss >> value;
                this->set_value(value);
                cb(value);
                return true;
            });
        }

        void set_value_increment(Scalar incr)
        {
            m_value_increment = incr;
        }

        void set_min_value(Scalar minValue)
        {
            m_min_value = minValue;
        }

        void set_max_value(Scalar maxValue)
        {
            m_max_value = maxValue;
        }

        void set_min_max_value(Scalar minValue, Scalar maxValue)
        {
            this->set_min_value(minValue);
            this->set_max_value(maxValue);
        }

        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override
        {
            if ((m_editable || m_spinnable) && down)
                m_mouse_down_value = value();

            SpinArea area = this->spin_area(p);
            if (m_spinnable && area != SpinArea::None && down && !focused())
            {
                if (area == SpinArea::Top)
                {
                    this->set_value(value() + m_value_increment);
                    if (m_callback)
                        m_callback(m_value);
                }
                else if (area == SpinArea::Bottom)
                {
                    this->set_value(value() - m_value_increment);
                    if (m_callback)
                        m_callback(m_value);
                }
                return true;
            }

            return TextBox::mouse_button_event(p, button, down, modifiers);
        }

        virtual bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                                      int modifiers) override
        {
            if (TextBox::mouse_drag_event(p, rel, button, modifiers))
                return true;
            if (m_spinnable && !this->focused() && button == 2 /* 1 << GLFW_MOUSE_BUTTON_2 */ &&
                m_mouse_down_pos.x != -1)
            {
                int val_delta = static_cast<int>((p.x - m_mouse_down_pos.x) / float(10));
                this->set_value(m_mouse_down_value + val_delta * m_value_increment);
                if (m_callback)
                    m_callback(m_value);
                return true;
            }
            return false;
        }

        virtual bool scroll_event(const Vector2i& p, const Vector2f& rel) override
        {
            if (Widget::scroll_event(p, rel))
                return true;
            if (m_spinnable && !this->focused())
            {
                int val_delta = (rel.y > 0) ? 1 : -1;
                this->set_value(this->value() + val_delta * m_value_increment);
                if (m_callback)
                    m_callback(m_value);
                return true;
            }
            return false;
        }

    private:
        Scalar m_mouse_down_value;
        Scalar m_value_increment;
        Scalar m_min_value, m_max_value;
    };

    /**
     * \brief A specialization of TextBox representing floating point values.
     *
     * Template parameters should be float types, e.g. ``float``, ``double``,
     * ``float64_t``, etc.
     */
    template <typename Scalar>
    class FloatBox : public TextBox
    {
    public:
        FloatBox(Widget* parent, Scalar value = (Scalar)0.f)
            : TextBox(parent)
        {
            m_number_format = sizeof(Scalar) == sizeof(float) ? "%.4g" : "%.7g";
            set_default_value("0");
            set_format("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?");
            set_value_increment((Scalar)0.1);
            set_min_max_value(std::numeric_limits<Scalar>::lowest(),
                              std::numeric_limits<Scalar>::max());
            set_value(value);
            set_spinnable(false);
        }

        std::string numberFormat() const
        {
            return m_number_format;
        }

        void numberFormat(const std::string& format)
        {
            m_number_format = format;
        }

        Scalar value() const
        {
            return (Scalar)std::stod(TextBox::value());
        }

        void set_value(Scalar value)
        {
            Scalar clamped_val = std::min(std::max(value, m_min_value), m_max_value);
            char buffer[50];
            SDLGUI_SNPRINTF(buffer, 50, m_number_format.c_str(), clamped_val);
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

        void set_min_value(Scalar minValue)
        {
            m_min_value = minValue;
        }

        void set_max_value(Scalar maxValue)
        {
            m_max_value = maxValue;
        }

        void set_min_max_value(Scalar minValue, Scalar maxValue)
        {
            set_min_value(minValue);
            set_max_value(maxValue);
        }

        virtual bool mouse_button_event(const Vector2i& p, int button, bool down,
                                        int modifiers) override
        {
            if ((m_editable || m_spinnable) && down)
                m_mouse_down_value = value();

            SpinArea area = spin_area(p);
            if (m_spinnable && area != SpinArea::None && down && !focused())
            {
                if (area == SpinArea::Top)
                {
                    set_value(value() + m_value_increment);
                    if (m_callback)
                        m_callback(m_value);
                }
                else if (area == SpinArea::Bottom)
                {
                    set_value(value() - m_value_increment);
                    if (m_callback)
                        m_callback(m_value);
                }
                return true;
            }

            return TextBox::mouse_button_event(p, button, down, modifiers);
        }

        virtual bool mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                                      int modifiers) override
        {
            if (TextBox::mouse_drag_event(p, rel, button, modifiers))
                return true;
            if (m_spinnable && !focused() && button == 2 /* 1 << GLFW_MOUSE_BUTTON_2 */ &&
                m_mouse_down_pos.x != -1)
            {
                int val_delta = static_cast<int>((p.x - m_mouse_down_pos.x) / float(10));
                set_value(m_mouse_down_value + val_delta * m_value_increment);
                if (m_callback)
                    m_callback(m_value);
                return true;
            }
            return false;
        }

        virtual bool scroll_event(const Vector2i& p, const Vector2f& rel) override
        {
            if (Widget::scroll_event(p, rel))
                return true;
            if (m_spinnable && !focused())
            {
                int val_delta = (rel.y > 0) ? 1 : -1;
                set_value(value() + val_delta * m_value_increment);
                if (m_callback)
                    m_callback(m_value);
                return true;
            }
            return false;
        }

    private:
        std::string m_number_format;
        Scalar m_mouse_down_value;
        Scalar m_value_increment;
        Scalar m_min_value, m_max_value;
    };
}
