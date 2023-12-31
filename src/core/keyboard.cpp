#include "core/keyboard.hpp"

namespace rl {
    bool Keyboard::is_button_pressed(const Keyboard::Button::type key) const
    {
        return m_pressed[key];
    }

    bool Keyboard::is_button_released(const Keyboard::Button::type key) const
    {
        const bool ret = m_released[key];
        m_released[key] = false;
        return ret;
    }

    bool Keyboard::is_button_held(const Keyboard::Button::type key) const
    {
        return m_held[key];
    }

    std::string Keyboard::get_key_state(const Keyboard::Button::type kb_button) const
    {
        return this->is_button_held(kb_button)     ? "Holding"
             : this->is_button_pressed(kb_button)  ? "Pressed"
             : this->is_button_released(kb_button) ? "Released"
                                                   : "None";
    }

    void Keyboard::process_button_down(Keyboard::Button::type key)
    {
        m_released[key] = false;
        auto& keystates = this->is_button_pressed(key)  //
                            ? m_held                    //
                            : m_pressed;                //
        keystates[key] = 1;
    }

    void Keyboard::process_button_up(Keyboard::Button::type key)
    {
        m_held[key] = false;
        m_pressed[key] = false;
        m_released[key] = true;
    }
}
