#pragma once

namespace rl::inline utils {
    template <typename T>
    struct iterator {
        auto& operator*(this auto&& self) {
            return *self.m_ptr;
        }

        auto& operator->(this auto&& self) {
            return self.m_ptr;
        }

        auto& operator++(this auto&& self) {
            self.m_ptr++;
            return self;
        }

        auto& operator++(this auto&& self, int) {
            auto ret{ self };
            self.m_ptr++;
            return ret;
        }

        auto& operator--(this auto&& self) {
            self.m_ptr--;
            return self;
        }

        auto& operator--(this auto&& self, int) {
            auto ret{ self };
            --self.m_ptr;
            return ret;
        }

        bool operator==(const iterator& b) const {
            return m_ptr == b.m_ptr;
        }

        bool operator!=(const iterator& b) const {
            return m_ptr != b.m_ptr;
        }

        T* m_ptr{ nullptr };
    };
}
