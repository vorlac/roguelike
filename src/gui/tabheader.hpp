#pragma once

#include <cmath>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "gui/widget.hpp"

namespace rl::gui {
    /**
     * \class TabHeader tabheader.h sdl_gui/tabheader.h
     *
     * \brief A Tab navigable widget.
     */
    class TabHeader : public Widget
    {
    public:
        TabHeader(Widget* parent, const std::string& font = "sans-bold");

        void set_font(const std::string& font)
        {
            m_font = font;
        }

        const std::string& font() const
        {
            return m_font;
        }

        bool overflowing() const
        {
            return mOverflowing;
        }

        /**
         * Sets the callable objects which is invoked when a tab button is pressed.
         * The argument provided to the callback is the index of the tab.
         */
        void set_callback(std::function<void(size_t)>&& callback)
        {
            m_active_header_changed_callback = std::move(callback);
        };

        const std::function<void(size_t)>& callback() const
        {
            return m_active_header_changed_callback;
        }

        void set_active_tab(size_t tab_index);
        size_t active_tab() const;
        bool isTabVisible(size_t index) const;

        size_t tab_count() const
        {
            return mTabButtons.size();
        }

        /// Inserts a tab at the end of the tabs collection.
        void add_tab(const std::string& label);

        /// Inserts a tab into the tabs collection at the specified index.
        void add_tab(size_t index, const std::string& label);

        /**
         * Removes the tab with the specified label and returns the index of the label.
         * Returns -1 if there was no such tab
         */
        int remove_tab(const std::string& label);

        /// Removes the tab with the specified index.
        void remove_tab(size_t index);

        /// Retrieves the label of the tab at a specific index.
        const std::string& tab_label_at(size_t index) const;

        /**
         * Retrieves the index of a specific tab label.
         * Returns the number of tabs (tabsCount) if there is no such tab.
         */
        int tab_index(const std::string& label);

        /**
         * Recalculate the visible range of tabs so that the tab with the specified
         * index is visible. The tab with the specified index will either be the
         * first or last visible one depending on the position relative to the
         * old visible range.
         */
        void ensure_tab_visible(size_t index);

        /**
         * Returns a pair of Vectors describing the top left (pair.first) and the
         * bottom right (pair.second) positions of the rectangle containing the visible tab buttons.
         */
        std::pair<Vector2i, Vector2i> visible_button_area() const;

        /**
         * Returns a pair of Vectors describing the top left (pair.first) and the
         * bottom right (pair.second) positions of the rectangle containing the active tab button.
         * Returns two zero vectors if the active button is not visible.
         */
        std::pair<Vector2i, Vector2i> active_button_area() const;

        void perform_layout(SDL3::SDL_Renderer* ctx) override;
        Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const override;
        bool mouse_button_event(const Vector2i& p, int button, bool down, int modifiers) override;

        void draw(const std::unique_ptr<rl::Renderer>& renderer) override;
        void draw(SDL3::SDL_Renderer* renderer) override;

    private:
        /**
         * @brief Implementation class of the actual tab buttons.
         */
        class TabButton
        {
        public:
            constexpr const static char* dots = "...";

            TabButton(TabHeader& header, const std::string& label);

            void set_label(const std::string& label)
            {
                m_label = label;
            }

            const std::string& label() const
            {
                return m_label;
            }

            void set_size(const Vector2i& size)
            {
                m_size = size;
            }

            const Vector2i& size() const
            {
                return m_size;
            }

            Vector2i preferred_size(SDL3::SDL_Renderer* ctx) const;
            void calculateVisibleString(SDL3::SDL_Renderer* renderer);
            void drawAtPosition(SDL3::SDL_Renderer* renderer, const Vector2i& position, bool active);
            void drawActiveBorderAt(SDL3::SDL_Renderer* renderer, const Vector2i& position,
                                    float offset, const Color& color);
            void drawInactiveBorderAt(SDL3::SDL_Renderer* renderer, const Vector2i& position,
                                      float offset, const Color& color);

        private:
            TabHeader* m_tab_header;
            std::string m_label;
            Vector2i m_size;

            /**
             * @brief Represent a TabButton.
             */
            struct StringView
            {
                const char* first = nullptr;
                const char* last = nullptr;
            };

            StringView m_visible_text;
            int m_visible_width = 0;

            Texture m_label_texture;
        };

        using TabIterator = std::vector<TabButton>::iterator;
        using ConstTabIterator = std::vector<TabButton>::const_iterator;

        // The location in which the Widget will be facing.
        enum class ClickLocation {
            LeftControls,
            RightControls,
            TabButtons
        };

        TabIterator visibleBegin()
        {
            return std::next(mTabButtons.begin(), m_visible_start);
        }

        TabIterator visibleEnd()
        {
            return std::next(mTabButtons.begin(), m_visible_end);
        }

        TabIterator activeIterator()
        {
            return std::next(mTabButtons.begin(), m_active_tab_idx);
        }

        TabIterator tabIterator(size_t index)
        {
            return std::next(mTabButtons.begin(), index);
        }

        ConstTabIterator visibleBegin() const
        {
            return std::next(mTabButtons.begin(), m_visible_start);
        }

        ConstTabIterator visibleEnd() const
        {
            return std::next(mTabButtons.begin(), m_visible_end);
        }

        ConstTabIterator activeIterator() const
        {
            return std::next(mTabButtons.begin(), m_active_tab_idx);
        }

        ConstTabIterator tabIterator(int index) const
        {
            return std::next(mTabButtons.begin(), index);
        }

        /// Given the beginning of the visible tabs, calculate the end.
        void calculateVisibleEnd();

        void drawControls(SDL3::SDL_Renderer* renderer);
        ClickLocation locateClick(const Vector2i& p);
        void onArrowLeft();
        void onArrowRight();

        std::function<void(size_t)> m_active_header_changed_callback;
        std::vector<TabButton> mTabButtons;
        int m_visible_start = 0;
        int m_visible_end = 0;
        size_t m_active_tab_idx = 0;
        bool mOverflowing = false;

        std::string m_font;
        Texture _leftIcon;
        Texture _rightIcon;
        size_t _lastLeftActive = std::numeric_limits<size_t>::max();
        size_t _lastRightActive = std::numeric_limits<size_t>::max();
    };
}
