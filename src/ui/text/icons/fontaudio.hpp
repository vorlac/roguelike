#pragma once

#include <string_view>

#include "utils/numeric.hpp"

namespace rl::ui::icon {

    struct FontAudio
    {
        constexpr static inline std::string_view FileName{ "fontaudio.ttf" };

        constexpr static inline u16 IconMin{ 0xf101 };
        constexpr static inline u16 IconMax16{ 0xf19b };
        constexpr static inline u16 IconMax{ 0xf19b };

        constexpr static inline std::string_view Adr{ "\xef\x84\x81" };                // U+f101
        constexpr static inline std::string_view Adsr{ "\xef\x84\x82" };               // U+f102
        constexpr static inline std::string_view Ahdsr{ "\xef\x84\x83" };              // U+f103
        constexpr static inline std::string_view Ar{ "\xef\x84\x84" };                 // U+f104
        constexpr static inline std::string_view Armrecording{ "\xef\x84\x85" };       // U+f105
        constexpr static inline std::string_view Arpchord{ "\xef\x84\x86" };           // U+f106
        constexpr static inline std::string_view Arpdown{ "\xef\x84\x87" };            // U+f107
        constexpr static inline std::string_view Arpdownandup{ "\xef\x84\x88" };       // U+f108
        constexpr static inline std::string_view Arpdownup{ "\xef\x84\x89" };          // U+f109
        constexpr static inline std::string_view Arpplayorder{ "\xef\x84\x8a" };       // U+f10a
        constexpr static inline std::string_view Arprandom{ "\xef\x84\x8b" };          // U+f10b
        constexpr static inline std::string_view Arpup{ "\xef\x84\x8c" };              // U+f10c
        constexpr static inline std::string_view Arpupandown{ "\xef\x84\x8d" };        // U+f10d
        constexpr static inline std::string_view Arpupdown{ "\xef\x84\x8e" };          // U+f10e
        constexpr static inline std::string_view ArrowsHorz{ "\xef\x84\x8f" };         // U+f10f
        constexpr static inline std::string_view ArrowsVert{ "\xef\x84\x90" };         // U+f110
        constexpr static inline std::string_view Automation_2p{ "\xef\x84\x91" };      // U+f111
        constexpr static inline std::string_view Automation_3p{ "\xef\x84\x92" };      // U+f112
        constexpr static inline std::string_view Automation_4p{ "\xef\x84\x93" };      // U+f113
        constexpr static inline std::string_view Backward{ "\xef\x84\x94" };           // U+f114
        constexpr static inline std::string_view Bluetooth{ "\xef\x84\x95" };          // U+f115
        constexpr static inline std::string_view CaretDown{ "\xef\x84\x96" };          // U+f116
        constexpr static inline std::string_view CaretLeft{ "\xef\x84\x97" };          // U+f117
        constexpr static inline std::string_view CaretRight{ "\xef\x84\x98" };         // U+f118
        constexpr static inline std::string_view CaretUp{ "\xef\x84\x99" };            // U+f119
        constexpr static inline std::string_view Close{ "\xef\x84\x9a" };              // U+f11a
        constexpr static inline std::string_view Copy{ "\xef\x84\x9b" };               // U+f11b
        constexpr static inline std::string_view Cpu{ "\xef\x84\x9c" };                // U+f11c
        constexpr static inline std::string_view Cutter{ "\xef\x84\x9d" };             // U+f11d
        constexpr static inline std::string_view DigitalColon{ "\xef\x84\x9e" };       // U+f11e
        constexpr static inline std::string_view DigitalDot{ "\xef\x84\x9f" };         // U+f11f
        constexpr static inline std::string_view Digital0{ "\xef\x84\xa0" };           // U+f120
        constexpr static inline std::string_view Digital1{ "\xef\x84\xa1" };           // U+f121
        constexpr static inline std::string_view Digital2{ "\xef\x84\xa2" };           // U+f122
        constexpr static inline std::string_view Digital3{ "\xef\x84\xa3" };           // U+f123
        constexpr static inline std::string_view Digital4{ "\xef\x84\xa4" };           // U+f124
        constexpr static inline std::string_view Digital5{ "\xef\x84\xa5" };           // U+f125
        constexpr static inline std::string_view Digital6{ "\xef\x84\xa6" };           // U+f126
        constexpr static inline std::string_view Digital7{ "\xef\x84\xa7" };           // U+f127
        constexpr static inline std::string_view Digital8{ "\xef\x84\xa8" };           // U+f128
        constexpr static inline std::string_view Digital9{ "\xef\x84\xa9" };           // U+f129
        constexpr static inline std::string_view Diskio{ "\xef\x84\xaa" };             // U+f12a
        constexpr static inline std::string_view Drumpad{ "\xef\x84\xab" };            // U+f12b
        constexpr static inline std::string_view Duplicate{ "\xef\x84\xac" };          // U+f12c
        constexpr static inline std::string_view Eraser{ "\xef\x84\xad" };             // U+f12d
        constexpr static inline std::string_view Ffwd{ "\xef\x84\xae" };               // U+f12e
        constexpr static inline std::string_view FilterBandpass{ "\xef\x84\xaf" };     // U+f12f
        constexpr static inline std::string_view FilterBell{ "\xef\x84\xb0" };         // U+f130
        constexpr static inline std::string_view FilterBypass{ "\xef\x84\xb1" };       // U+f131
        constexpr static inline std::string_view FilterHighpass{ "\xef\x84\xb2" };     // U+f132
        constexpr static inline std::string_view FilterLowpass{ "\xef\x84\xb3" };      // U+f133
        constexpr static inline std::string_view FilterNotch{ "\xef\x84\xb4" };        // U+f134
        constexpr static inline std::string_view FilterRezHighpass{ "\xef\x84\xb5" };  // U+f135
        constexpr static inline std::string_view FilterRezLowpass{ "\xef\x84\xb6" };   // U+f136
        constexpr static inline std::string_view FilterShelvingHi{ "\xef\x84\xb7" };   // U+f137
        constexpr static inline std::string_view FilterShelvingLo{ "\xef\x84\xb8" };   // U+f138
        constexpr static inline std::string_view Foldback{ "\xef\x84\xb9" };           // U+f139
        constexpr static inline std::string_view Forward{ "\xef\x84\xba" };            // U+f13a
        constexpr static inline std::string_view HExpand{ "\xef\x84\xbb" };            // U+f13b
        constexpr static inline std::string_view Hardclip{ "\xef\x84\xbc" };           // U+f13c
        constexpr static inline std::string_view Hardclipcurve{ "\xef\x84\xbd" };      // U+f13d
        constexpr static inline std::string_view Headphones{ "\xef\x84\xbe" };         // U+f13e
        constexpr static inline std::string_view Keyboard{ "\xef\x84\xbf" };           // U+f13f
        constexpr static inline std::string_view Lock{ "\xef\x85\x80" };               // U+f140
        constexpr static inline std::string_view LogoAax{ "\xef\x85\x81" };            // U+f141
        constexpr static inline std::string_view LogoAbletonlink{ "\xef\x85\x82" };    // U+f142
        constexpr static inline std::string_view LogoAu{ "\xef\x85\x83" };             // U+f143
        constexpr static inline std::string_view LogoAudacity{ "\xef\x85\x84" };       // U+f144
        constexpr static inline std::string_view LogoAudiobus{ "\xef\x85\x85" };       // U+f145
        constexpr static inline std::string_view LogoCubase{ "\xef\x85\x86" };         // U+f146
        constexpr static inline std::string_view LogoFl{ "\xef\x85\x87" };             // U+f147
        constexpr static inline std::string_view LogoJuce{ "\xef\x85\x88" };           // U+f148
        constexpr static inline std::string_view LogoLadspa{ "\xef\x85\x89" };         // U+f149
        constexpr static inline std::string_view LogoLive{ "\xef\x85\x8a" };           // U+f14a
        constexpr static inline std::string_view LogoLv2{ "\xef\x85\x8b" };            // U+f14b
        constexpr static inline std::string_view LogoProtools{ "\xef\x85\x8c" };       // U+f14c
        constexpr static inline std::string_view LogoRackext{ "\xef\x85\x8d" };        // U+f14d
        constexpr static inline std::string_view LogoReaper{ "\xef\x85\x8e" };         // U+f14e
        constexpr static inline std::string_view LogoReason{ "\xef\x85\x8f" };         // U+f14f
        constexpr static inline std::string_view LogoRewire{ "\xef\x85\x90" };         // U+f150
        constexpr static inline std::string_view LogoStudioone{ "\xef\x85\x91" };      // U+f151
        constexpr static inline std::string_view LogoTracktion{ "\xef\x85\x92" };      // U+f152
        constexpr static inline std::string_view LogoVst{ "\xef\x85\x93" };            // U+f153
        constexpr static inline std::string_view LogoWaveform{ "\xef\x85\x94" };       // U+f154
        constexpr static inline std::string_view Loop{ "\xef\x85\x95" };               // U+f155
        constexpr static inline std::string_view Metronome{ "\xef\x85\x96" };          // U+f156
        constexpr static inline std::string_view Microphone{ "\xef\x85\x97" };         // U+f157
        constexpr static inline std::string_view Midiplug{ "\xef\x85\x98" };           // U+f158
        constexpr static inline std::string_view Modrandom{ "\xef\x85\x99" };          // U+f159
        constexpr static inline std::string_view Modsawdown{ "\xef\x85\x9a" };         // U+f15a
        constexpr static inline std::string_view Modsawup{ "\xef\x85\x9b" };           // U+f15b
        constexpr static inline std::string_view Modsh{ "\xef\x85\x9c" };              // U+f15c
        constexpr static inline std::string_view Modsine{ "\xef\x85\x9d" };            // U+f15d
        constexpr static inline std::string_view Modsquare{ "\xef\x85\x9e" };          // U+f15e
        constexpr static inline std::string_view Modtri{ "\xef\x85\x9f" };             // U+f15f
        constexpr static inline std::string_view Modularplug{ "\xef\x85\xa0" };        // U+f160
        constexpr static inline std::string_view Mono{ "\xef\x85\xa1" };               // U+f161
        constexpr static inline std::string_view Mute{ "\xef\x85\xa2" };               // U+f162
        constexpr static inline std::string_view Next{ "\xef\x85\xa3" };               // U+f163
        constexpr static inline std::string_view Open{ "\xef\x85\xa4" };               // U+f164
        constexpr static inline std::string_view Paste{ "\xef\x85\xa5" };              // U+f165
        constexpr static inline std::string_view Pause{ "\xef\x85\xa6" };              // U+f166
        constexpr static inline std::string_view Pen{ "\xef\x85\xa7" };                // U+f167
        constexpr static inline std::string_view Phase{ "\xef\x85\xa8" };              // U+f168
        constexpr static inline std::string_view Play{ "\xef\x85\xa9" };               // U+f169
        constexpr static inline std::string_view Pointer{ "\xef\x85\xaa" };            // U+f16a
        constexpr static inline std::string_view Powerswitch{ "\xef\x85\xab" };        // U+f16b
        constexpr static inline std::string_view PresetA{ "\xef\x85\xac" };            // U+f16c
        constexpr static inline std::string_view PresetAb{ "\xef\x85\xad" };           // U+f16d
        constexpr static inline std::string_view PresetB{ "\xef\x85\xae" };            // U+f16e
        constexpr static inline std::string_view PresetBa{ "\xef\x85\xaf" };           // U+f16f
        constexpr static inline std::string_view Prev{ "\xef\x85\xb0" };               // U+f170
        constexpr static inline std::string_view PunchIn{ "\xef\x85\xb1" };            // U+f171
        constexpr static inline std::string_view PunchOut{ "\xef\x85\xb2" };           // U+f172
        constexpr static inline std::string_view Ram{ "\xef\x85\xb3" };                // U+f173
        constexpr static inline std::string_view Random_1dice{ "\xef\x85\xb4" };       // U+f174
        constexpr static inline std::string_view Random_2dice{ "\xef\x85\xb5" };       // U+f175
        constexpr static inline std::string_view Record{ "\xef\x85\xb6" };             // U+f176
        constexpr static inline std::string_view Redo{ "\xef\x85\xb7" };               // U+f177
        constexpr static inline std::string_view RepeatOne{ "\xef\x85\xb8" };          // U+f178
        constexpr static inline std::string_view Repeat{ "\xef\x85\xb9" };             // U+f179
        constexpr static inline std::string_view Rew{ "\xef\x85\xba" };                // U+f17a
        constexpr static inline std::string_view RoundswitchOff{ "\xef\x85\xbb" };     // U+f17b
        constexpr static inline std::string_view RoundswitchOn{ "\xef\x85\xbc" };      // U+f17c
        constexpr static inline std::string_view Save{ "\xef\x85\xbd" };               // U+f17d
        constexpr static inline std::string_view Saveas{ "\xef\x85\xbe" };             // U+f17e
        constexpr static inline std::string_view Scissors{ "\xef\x85\xbf" };           // U+f17f
        constexpr static inline std::string_view Shuffle{ "\xef\x86\x80" };            // U+f180
        constexpr static inline std::string_view SliderRound_1{ "\xef\x86\x81" };      // U+f181
        constexpr static inline std::string_view SliderRound_2{ "\xef\x86\x82" };      // U+f182
        constexpr static inline std::string_view SliderRound_3{ "\xef\x86\x83" };      // U+f183
        constexpr static inline std::string_view Sliderhandle_1{ "\xef\x86\x84" };     // U+f184
        constexpr static inline std::string_view Sliderhandle_2{ "\xef\x86\x85" };     // U+f185
        constexpr static inline std::string_view Softclip{ "\xef\x86\x86" };           // U+f186
        constexpr static inline std::string_view Softclipcurve{ "\xef\x86\x87" };      // U+f187
        constexpr static inline std::string_view Solo{ "\xef\x86\x88" };               // U+f188
        constexpr static inline std::string_view Speaker{ "\xef\x86\x89" };            // U+f189
        constexpr static inline std::string_view SquareswitchOff{ "\xef\x86\x8a" };    // U+f18a
        constexpr static inline std::string_view SquareswitchOn{ "\xef\x86\x8b" };     // U+f18b
        constexpr static inline std::string_view Stereo{ "\xef\x86\x8c" };             // U+f18c
        constexpr static inline std::string_view Stop{ "\xef\x86\x8d" };               // U+f18d
        constexpr static inline std::string_view Thunderbolt{ "\xef\x86\x8e" };        // U+f18e
        constexpr static inline std::string_view Timeselect{ "\xef\x86\x8f" };         // U+f18f
        constexpr static inline std::string_view Undo{ "\xef\x86\x90" };               // U+f190
        constexpr static inline std::string_view Unlock{ "\xef\x86\x91" };             // U+f191
        constexpr static inline std::string_view Usb{ "\xef\x86\x92" };                // U+f192
        constexpr static inline std::string_view VExpand{ "\xef\x86\x93" };            // U+f193
        constexpr static inline std::string_view VroundswitchOff{ "\xef\x86\x94" };    // U+f194
        constexpr static inline std::string_view VroundswitchOn{ "\xef\x86\x95" };     // U+f195
        constexpr static inline std::string_view VsquareswitchOff{ "\xef\x86\x96" };   // U+f196
        constexpr static inline std::string_view VsquareswitchOn{ "\xef\x86\x97" };    // U+f197
        constexpr static inline std::string_view Waveform{ "\xef\x86\x98" };           // U+f198
        constexpr static inline std::string_view Xlrplug{ "\xef\x86\x99" };            // U+f199
        constexpr static inline std::string_view Zoomin{ "\xef\x86\x9a" };             // U+f19a
        constexpr static inline std::string_view Zoomout{ "\xef\x86\x9b" };            // U+f19b
    };
}
