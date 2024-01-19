#pragma once

#include "core/ui/icons.hpp"
#include "ds/color.hpp"
#include "ds/refcounted.hpp"
#include "ds/shared.hpp"
#include "graphics/vg/nanovg.hpp"
#include "resources/fonts.hpp"
#include "utils/numeric.hpp"

namespace rl::ui {
    enum class Outline {
        Inner,
        Outer
    };

    struct Text
    {
        enum Alignment {
            HorizLeft = nvg::NVG_ALIGN_LEFT,         // Default, align text horizontally to left.
            HorizCenter = nvg::NVG_ALIGN_CENTER,     // Align text horizontally to center.
            HorizRight = nvg::NVG_ALIGN_RIGHT,       // Align text horizontally to right.
            VertTop = nvg::NVG_ALIGN_TOP,            // Align text vertically to top.
            VertCenter = nvg::NVG_ALIGN_MIDDLE,      // Align text vertically to middle.
            VertBottom = nvg::NVG_ALIGN_BOTTOM,      // Align text vertically to bottom.
            VertBaseline = nvg::NVG_ALIGN_BASELINE,  // Align text vertically to baseline.

            Centered = HorizCenter | VertCenter,
            TopLeft = HorizLeft | VertTop,
            TopMiddle = HorizCenter | VertTop,
            CenteredLeft = HorizLeft | VertCenter,
            CenteredRight = HorizRight | VertCenter,
        };
    };

    namespace font {
        constexpr i32 uninitialized{ -1 };

        namespace name {
            constexpr const char* const sans{ "sans" };
            constexpr const char* const sans_bold{ "sans_bold" };
            constexpr const char* const icons{ "icons" };
            constexpr const char* const mono{ "mono" };
        }
    }

    class Theme : public ds::refcounted
    {
    public:
        Theme(nvg::NVGcontext* nvg_context)
            : m_font_sans_regular{
                nvg::CreateFontMem(nvg_context, ui::font::name::sans,
                                 roboto_regular_ttf, roboto_regular_ttf_size, 0),
            }
            , m_font_sans_bold{
                nvg::CreateFontMem(nvg_context, ui::font::name::sans_bold,
                                 roboto_bold_ttf, roboto_bold_ttf_size, 0),
            }
            , m_font_icons{
                nvg::CreateFontMem(nvg_context, ui::font::name::icons,
                                 fontawesome_solid_ttf, fontawesome_solid_ttf_size, 0),
            }
            , m_font_mono_regular{
                nvg::CreateFontMem(nvg_context, ui::font::name::mono, fira_code_regular_ttf,
                                 fira_code_regular_ttf_size, 0),
            }
        {
            bool font_load_success{ true };
            font_load_success &= m_font_sans_regular != font::uninitialized;
            font_load_success &= m_font_sans_bold != font::uninitialized;
            font_load_success &= m_font_icons != font::uninitialized;
            font_load_success &= m_font_mono_regular != font::uninitialized;
            runtime_assert(font_load_success, "Failed to load fonts");
        }

        i32 m_font_sans_regular{ font::uninitialized };
        i32 m_font_sans_bold{ font::uninitialized };
        i32 m_font_icons{ font::uninitialized };
        i32 m_font_mono_regular{ font::uninitialized };

        f32 m_icon_scale{ 1.0f };
        f32 m_tab_border_width{ 0.75f };

        f32 m_standard_font_size{ 16.0f };
        f32 m_button_font_size{ 20.0f };
        f32 m_text_box_font_size{ 20.0f };
        f32 m_window_corner_radius{ 2.0f };
        f32 m_window_header_height{ 30.0f };
        f32 m_window_drop_shadow_size{ 10.0f };
        f32 m_button_corner_radius{ 2.0f };
        i32 m_tab_inner_margin{ 5 };
        i32 m_tab_min_button_width{ 20 };
        i32 m_tab_max_button_width{ 160 };
        i32 m_tab_control_width{ 20 };
        i32 m_tab_button_horizontal_padding{ 10 };
        i32 m_tab_button_vertical_padding{ 2 };

        ds::color<f32> m_drop_shadow{ 0, 0, 0, 128 };
        ds::color<f32> m_transparent{ 0, 0, 0, 0 };
        ds::color<f32> m_border_dark{ 29, 29, 29, 255 };
        ds::color<f32> m_border_light{ 92, 92, 92, 255 };
        ds::color<f32> m_border_medium{ 35, 35, 35, 255 };
        ds::color<f32> m_text_color{ 255, 255, 255, 160 };
        ds::color<f32> m_disabled_text_color{ 255, 255, 255, 80 };
        ds::color<f32> m_text_shadow_color{ 0, 0, 0, 0 };
        ds::color<f32> m_icon_color{ m_text_color };

        ds::color<f32> m_button_gradient_top_focused{ 64, 64, 64, 255 };
        ds::color<f32> m_button_gradient_bot_focused{ 48, 48, 48, 255 };
        ds::color<f32> m_button_gradient_top_unfocused{ 74, 74, 74, 255 };
        ds::color<f32> m_button_gradient_bot_unfocused{ 58, 58, 58, 255 };
        ds::color<f32> m_button_gradient_top_pushed{ 41, 41, 41, 255 };
        ds::color<f32> m_button_gradient_bot_pushed{ 29, 29, 29, 255 };

        ds::color<f32> m_window_fill_unfocused{ 43, 43, 43, 230 };
        ds::color<f32> m_window_fill_focused{ 45, 45, 45, 230 };
        ds::color<f32> m_window_title_unfocused{ 220, 220, 220, 160 };
        ds::color<f32> m_window_title_focused{ 255, 255, 255, 190 };
        ds::color<f32> m_window_header_gradient_top{ m_button_gradient_top_unfocused };
        ds::color<f32> m_window_header_gradient_bot{ m_button_gradient_bot_unfocused };
        ds::color<f32> m_window_header_sep_top{ m_border_light };
        ds::color<f32> m_window_header_sep_bot{ m_border_dark };
        ds::color<f32> m_window_popup{ 50, 50, 50, 255 };
        ds::color<f32> m_window_popup_transparent{ 50, 50, 50, 0 };

        ui::Icon::ID m_check_box_icon{ ui::Icon::Check };
        ui::Icon::ID m_message_information_icon{ ui::Icon::InfoCircle };
        ui::Icon::ID m_message_question_icon{ ui::Icon::QuestionCircle };
        ui::Icon::ID m_message_warning_icon{ ui::Icon::ExclamationTriangle };
        ui::Icon::ID m_message_alt_button_icon{ ui::Icon::PlusCircle };
        ui::Icon::ID m_message_primary_button_icon{ ui::Icon::Check };
        ui::Icon::ID m_popup_chevron_right_icon{ ui::Icon::ChevronRight };
        ui::Icon::ID m_popup_chevron_left_icon{ ui::Icon::ChevronLeft };
        ui::Icon::ID m_text_box_up_icon{ ui::Icon::ChevronUp };
        ui::Icon::ID m_text_box_down_icon{ ui::Icon::ChevronDown };
    };
}
