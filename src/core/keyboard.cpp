#include "core/keyboard.hpp"

namespace rl {
    Keyboard::Scancode::ID Keyboard::keys_down() const
    {
        return Keyboard::Scancode::ID(m_pressed.all() | m_held.all());
    }

    bool Keyboard::is_button_pressed(const Keyboard::Scancode::ID key) const
    {
        return m_pressed[key];
    }

    bool Keyboard::is_button_released(const Keyboard::Scancode::ID key) const
    {
        const bool ret = m_released[key];
        m_released[key] = false;
        return ret;
    }

    bool Keyboard::is_button_held(const Keyboard::Scancode::ID key) const
    {
        return m_held[key];
    }

    bool Keyboard::is_button_down(const Keyboard::Scancode::ID key) const
    {
        return m_held[key] || m_pressed[key];
    }

    bool Keyboard::all_buttons_down(std::vector<Keyboard::Scancode::ID> keys) const
    {
        bool ret{ true };

        for (auto&& key : keys)
            ret &= this->is_button_down(key);

        return ret;
    }

    bool Keyboard::any_buttons_down(std::vector<Keyboard::Scancode::ID> keys) const
    {
        for (auto&& key : keys)
            if (this->is_button_down(key))
                return true;

        return false;
    }

    std::string Keyboard::get_key_state(const Keyboard::Scancode::ID key) const
    {
        return this->is_button_held(key)     ? "Held"
             : this->is_button_pressed(key)  ? "Pressed"
             : this->is_button_released(key) ? "Released"
                                             : "None";
    }

    std::string Keyboard::get_inputted_text() const
    {
        return m_text;
    }

    std::string Keyboard::get_inputted_text_compisition() const
    {
        return m_composition;
    }

    i32 Keyboard::get_inputted_text_cursor_loc() const
    {
        return m_cursor_pos;
    }

    i32 Keyboard::get_inputted_text_length() const
    {
        return m_text_length;
    }

    void Keyboard::process_button_down(const Scancode::ID key)
    {
        m_released[key] = false;
        auto& keystates = this->is_button_pressed(key)  //
                            ? m_held                    //
                            : m_pressed;                //
        keystates[key] = true;
    }

    void Keyboard::process_button_up(const Scancode::ID key)
    {
        m_held[key] = false;
        m_pressed[key] = false;
        m_released[key] = true;
    }

    void Keyboard::process_text_input(const char* text)
    {
        m_text += std::string{ text };
    }

    void Keyboard::process_text_editing(const char* composition, const i32 start, const i32 length)
    {
        m_composition = std::string{ composition };
        m_cursor_pos = start;
        m_text_length = length;
    }
}
