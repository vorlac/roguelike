#pragma once

#include <string>

#include <parallel_hashmap/phmap.h>

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

    struct LineProperties
    {
    };

    struct Font
    {
        using ID = i32;
        using Map = phmap::flat_hash_map<std::string, ID>;

        constexpr static inline i32 InvalidHandle{ -1 };

        enum class Source {
            Memory,
            Disk,
        };

        struct Name
        {
            constexpr static inline std::string_view Sans{ "sans" };
            constexpr static inline std::string_view SansBold{ "sans_bold" };
            constexpr static inline std::string_view Icons{ "icons" };
            constexpr static inline std::string_view Mono{ "mono" };
        };
    };

    struct Text
    {
        enum Alignment {
            HorizLeft = nvg::NVGAlignLeft,         // Default, align text horizontally to left.
            HorizCenter = nvg::NVGAlignCenter,     // Align text horizontally to center.
            HorizRight = nvg::NVGAlignRight,       // Align text horizontally to right.
            VertTop = nvg::NVGAlignTop,            // Align text vertically to top.
            VertMiddle = nvg::NVGAlignMiddle,      // Align text vertically to middle.
            VertBottom = nvg::NVGAlignBottom,      // Align text vertically to bottom.
            VertBaseline = nvg::NVGAlignBaseline,  // Align text vertically to baseline.

            HCenterVMiddle = HorizCenter | VertMiddle,
            HLeftVTop = HorizLeft | VertTop,
            HMiddleVTop = HorizCenter | VertTop,
            HMiddleVBot = HorizCenter | VertBottom,
            HMiddleVBase = HorizCenter | VertBaseline,
            HLeftVMiddle = HorizLeft | VertMiddle,
            HRightVMiddle = HorizRight | VertMiddle,
        };

        friend Alignment operator|(const Alignment a1, const Alignment a2)
        {
            return static_cast<Alignment>(std::to_underlying(a1) | std::to_underlying(a2));
        }

        struct Properties
        {
            f32 font_size{ 18.0f };
            f32 border_thickness{ 1.0f };
            f32 border_blur{ 2.0f };

            std::string font{ Font::Name::SansBold };

            ds::color<f32> color{ Colors::White };
            ds::color<f32> border_color{ Colors::Transparent };

            ds::vector2<f32> margins{ 10.0f, 10.0f };
            Alignment alignment{ HorizCenter | VertMiddle };
        };
    };

    class Theme final : public ds::refcounted
    {
    public:
        explicit Theme(const nvg::NVGcontext* nvg_context)
            : font_sans_regular{ nvg::create_font_mem(
                  nvg_context, Font::Name::Sans,
                  std::basic_string_view<u8>{ roboto_regular_ttf, roboto_regular_ttf_size }) }
            , font_sans_bold{ nvg::create_font_mem(
                  nvg_context, Font::Name::SansBold,
                  std::basic_string_view<u8>{ roboto_bold_ttf, roboto_bold_ttf_size }) }
            , font_icons{ nvg::create_font_mem(
                  nvg_context, Font::Name::Icons,
                  std::basic_string_view{ fontawesome_solid_ttf, fontawesome_solid_ttf_size }) }
            , font_mono_regular{ nvg::create_font_mem(
                  nvg_context, Font::Name::Mono,
                  std::basic_string_view<u8>{ fira_code_bold_ttf, fira_code_bold_ttf_size }) }
        {
            bool font_load_success{ true };

            font_load_success &= font_sans_regular != Font::InvalidHandle;
            font_load_success &= font_sans_bold != Font::InvalidHandle;
            font_load_success &= font_icons != Font::InvalidHandle;
            font_load_success &= font_mono_regular != Font::InvalidHandle;
            runtime_assert(font_load_success, "Failed to load fonts");
        }

        i32 font_sans_regular{ Font::InvalidHandle };
        i32 font_sans_bold{ Font::InvalidHandle };
        i32 font_icons{ Font::InvalidHandle };
        i32 font_mono_regular{ Font::InvalidHandle };

        std::string form_group_font_name{ Font::Name::Mono };
        std::string form_label_font_name{ Font::Name::Sans };
        std::string tooltip_font_name{ Font::Name::SansBold };
        std::string dialog_title_font_name{ Font::Name::SansBold };

        f32 icon_scale{ 1.0f };
        f32 tab_border_width{ 0.75f };

        f32 standard_font_size{ 16.0f };
        f32 tooltip_font_size{ 18.0f };
        f32 button_font_size{ 20.0f };
        f32 text_box_font_size{ 20.0f };
        f32 dialog_title_font_size{ 24.0f };
        f32 form_group_font_size{ 24.0f };
        f32 form_label_font_size{ 18.0f };
        f32 form_widget_font_size{ 18.0f };
        f32 form_pre_group_spacing{ 15.0f };
        f32 form_post_group_spacing{ 15.0f };
        f32 form_variable_spacing{ 10.0f };

        f32 dialog_corner_radius{ 5.0f };
        f32 dialog_header_height{ 40.0f };
        f32 dialog_drop_shadow_size{ 15.0f };

        f32 button_corner_radius{ 2.5f };

        f32 tab_inner_margin{ 5.0f };
        f32 tab_min_button_width{ 20.0f };
        f32 tab_max_button_width{ 160.0f };
        f32 tab_control_width{ 20.0f };
        f32 tab_button_horizontal_padding{ 10.0f };
        f32 tab_button_vertical_padding{ 2.0f };

        constexpr static inline ds::color<f32> drop_shadow{ 0, 0, 0, 128 };
        ds::color<f32> text_shadow{ 0, 0, 0, 128 };
        ds::color<f32> dialog_shadow{ 0, 0, 0, 128 };
        ds::color<f32> transparent{ 0, 0, 0, 0 };
        ds::color<f32> border_dark{ 29, 29, 29, 255 };
        ds::color<f32> border_light{ 92, 92, 92, 255 };
        ds::color<f32> border_medium{ 35, 35, 35, 255 };
        ds::color<f32> text_color{ Colors::LightGrey };
        ds::color<f32> disabled_text_color{ Colors::DarkGrey };
        ds::color<f32> text_shadow_color{ Colors::Black };
        ds::color<f32> icon_color{ Colors::LightGrey };

        ds::color<f32> button_gradient_top_focused{ 64, 64, 64, 255 };
        ds::color<f32> button_gradient_bot_focused{ 48, 48, 48, 255 };
        ds::color<f32> button_gradient_top_unfocused{ 74, 74, 74, 255 };
        ds::color<f32> button_gradient_bot_unfocused{ 58, 58, 58, 255 };
        ds::color<f32> button_gradient_top_pushed{ 41, 41, 41, 255 };
        ds::color<f32> button_gradient_bot_pushed{ 29, 29, 29, 255 };

        ds::color<f32> dialog_fill_unfocused{ 43, 43, 43, 230 };
        ds::color<f32> dialog_fill_focused{ 45, 45, 45, 230 };
        ds::color<f32> dialog_title_unfocused{ 220, 220, 220, 160 };
        ds::color<f32> dialog_title_focused{ 255, 255, 255, 190 };
        ds::color<f32> dialog_header_gradient_top{ 74, 74, 74, 255 };
        ds::color<f32> dialog_header_gradient_bot{ 58, 58, 58, 255 };
        ds::color<f32> dialog_header_sep_top{ 92, 92, 92, 255 };
        ds::color<f32> dialog_header_sep_bot{ 29, 29, 29, 255 };
        ds::color<f32> dialog_popup_fill{ 50, 50, 50, 255 };
        ds::color<f32> dialog_popup_transparent{ 50, 50, 50, 0 };

        Icon::ID check_box_icon{ Icon::Check };
        Icon::ID message_information_icon{ Icon::InfoCircle };
        Icon::ID message_question_icon{ Icon::QuestionCircle };
        Icon::ID message_warning_icon{ Icon::ExclamationTriangle };
        Icon::ID message_alt_button_icon{ Icon::PlusCircle };
        Icon::ID message_primary_button_icon{ Icon::Check };
        Icon::ID popup_chevron_right_icon{ Icon::ChevronRight };
        Icon::ID popup_chevron_left_icon{ Icon::ChevronLeft };
        Icon::ID text_box_up_icon{ Icon::ChevronUp };
        Icon::ID text_box_down_icon{ Icon::ChevronDown };
    };
}
