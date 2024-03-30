#pragma once

#include <bitset>
#include <string>
#include <vector>

#include <fmt/format.h>

#include "sdl/defs.hpp"
#include "utils/numeric.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL_events.h>
SDL_C_LIB_END

namespace rl {
    class EventHandler;
    class MainWindow;

    class Keyboard
    {
        friend MainWindow;

    public:
        // clang-format off

        // Keyboard Specific Event Identifiers
        struct Event
        {
            using type = SDL3::SDL_EventType;

            enum ID : std::underlying_type_t<type> {
                KeyDown       = SDL3::SDL_EVENT_KEY_DOWN,       // Key pressed
                KeyUp         = SDL3::SDL_EVENT_KEY_UP,         // Key released
                TextEditing   = SDL3::SDL_EVENT_TEXT_EDITING,   // Keyboard text editing (composition)
                TextInput     = SDL3::SDL_EVENT_TEXT_INPUT,     // Keyboard text input
                KeymapChanged = SDL3::SDL_EVENT_KEYMAP_CHANGED, // Keymap changed due to a system event such as an input language or keyboard layout change
            };
        };

        // clang-format on

        // Keyboard Scancode Identifiers
        struct Scancode
        {
            using type = SDL3::SDL_Scancode;

            enum ID : std::underlying_type_t<type> {
                Unknown = SDL3::SDL_SCANCODE_UNKNOWN,
                A = SDL3::SDL_SCANCODE_A,
                B = SDL3::SDL_SCANCODE_B,
                C = SDL3::SDL_SCANCODE_C,
                D = SDL3::SDL_SCANCODE_D,
                E = SDL3::SDL_SCANCODE_E,
                F = SDL3::SDL_SCANCODE_F,
                G = SDL3::SDL_SCANCODE_G,
                H = SDL3::SDL_SCANCODE_H,
                I = SDL3::SDL_SCANCODE_I,
                J = SDL3::SDL_SCANCODE_J,
                K = SDL3::SDL_SCANCODE_K,
                L = SDL3::SDL_SCANCODE_L,
                M = SDL3::SDL_SCANCODE_M,
                N = SDL3::SDL_SCANCODE_N,
                O = SDL3::SDL_SCANCODE_O,
                P = SDL3::SDL_SCANCODE_P,
                Q = SDL3::SDL_SCANCODE_Q,
                R = SDL3::SDL_SCANCODE_R,
                S = SDL3::SDL_SCANCODE_S,
                T = SDL3::SDL_SCANCODE_T,
                U = SDL3::SDL_SCANCODE_U,
                V = SDL3::SDL_SCANCODE_V,
                W = SDL3::SDL_SCANCODE_W,
                X = SDL3::SDL_SCANCODE_X,
                Y = SDL3::SDL_SCANCODE_Y,
                Z = SDL3::SDL_SCANCODE_Z,
                One = SDL3::SDL_SCANCODE_1,
                Two = SDL3::SDL_SCANCODE_2,
                Three = SDL3::SDL_SCANCODE_3,
                Four = SDL3::SDL_SCANCODE_4,
                Five = SDL3::SDL_SCANCODE_5,
                Six = SDL3::SDL_SCANCODE_6,
                Seven = SDL3::SDL_SCANCODE_7,
                Eight = SDL3::SDL_SCANCODE_8,
                Nine = SDL3::SDL_SCANCODE_9,
                Zero = SDL3::SDL_SCANCODE_0,
                Return = SDL3::SDL_SCANCODE_RETURN,
                Escape = SDL3::SDL_SCANCODE_ESCAPE,
                Backspace = SDL3::SDL_SCANCODE_BACKSPACE,
                Tab = SDL3::SDL_SCANCODE_TAB,
                Space = SDL3::SDL_SCANCODE_SPACE,
                Minus = SDL3::SDL_SCANCODE_MINUS,
                Equals = SDL3::SDL_SCANCODE_EQUALS,
                Leftbracket = SDL3::SDL_SCANCODE_LEFTBRACKET,
                Rightbracket = SDL3::SDL_SCANCODE_RIGHTBRACKET,
                Backslash = SDL3::SDL_SCANCODE_BACKSLASH,
                Nonushash = SDL3::SDL_SCANCODE_NONUSHASH,
                Semicolon = SDL3::SDL_SCANCODE_SEMICOLON,
                Apostrophe = SDL3::SDL_SCANCODE_APOSTROPHE,
                Grave = SDL3::SDL_SCANCODE_GRAVE,
                Comma = SDL3::SDL_SCANCODE_COMMA,
                Period = SDL3::SDL_SCANCODE_PERIOD,
                Slash = SDL3::SDL_SCANCODE_SLASH,
                CapsLock = SDL3::SDL_SCANCODE_CAPSLOCK,
                F1 = SDL3::SDL_SCANCODE_F1,
                F2 = SDL3::SDL_SCANCODE_F2,
                F3 = SDL3::SDL_SCANCODE_F3,
                F4 = SDL3::SDL_SCANCODE_F4,
                F5 = SDL3::SDL_SCANCODE_F5,
                F6 = SDL3::SDL_SCANCODE_F6,
                F7 = SDL3::SDL_SCANCODE_F7,
                F8 = SDL3::SDL_SCANCODE_F8,
                F9 = SDL3::SDL_SCANCODE_F9,
                F10 = SDL3::SDL_SCANCODE_F10,
                F11 = SDL3::SDL_SCANCODE_F11,
                F12 = SDL3::SDL_SCANCODE_F12,
                PrintScreen = SDL3::SDL_SCANCODE_PRINTSCREEN,
                ScrollLock = SDL3::SDL_SCANCODE_SCROLLLOCK,
                Pause = SDL3::SDL_SCANCODE_PAUSE,
                Insert = SDL3::SDL_SCANCODE_INSERT,
                Home = SDL3::SDL_SCANCODE_HOME,
                PageUp = SDL3::SDL_SCANCODE_PAGEUP,
                Delete = SDL3::SDL_SCANCODE_DELETE,
                End = SDL3::SDL_SCANCODE_END,
                PageDown = SDL3::SDL_SCANCODE_PAGEDOWN,
                Right = SDL3::SDL_SCANCODE_RIGHT,
                Left = SDL3::SDL_SCANCODE_LEFT,
                Down = SDL3::SDL_SCANCODE_DOWN,
                Up = SDL3::SDL_SCANCODE_UP,
                NumLockClear = SDL3::SDL_SCANCODE_NUMLOCKCLEAR,
                KPDivide = SDL3::SDL_SCANCODE_KP_DIVIDE,
                KPMultiply = SDL3::SDL_SCANCODE_KP_MULTIPLY,
                KPMinus = SDL3::SDL_SCANCODE_KP_MINUS,
                KPPlus = SDL3::SDL_SCANCODE_KP_PLUS,
                KPEnter = SDL3::SDL_SCANCODE_KP_ENTER,
                KP_1 = SDL3::SDL_SCANCODE_KP_1,
                KP_2 = SDL3::SDL_SCANCODE_KP_2,
                KP_3 = SDL3::SDL_SCANCODE_KP_3,
                KP_4 = SDL3::SDL_SCANCODE_KP_4,
                KP_5 = SDL3::SDL_SCANCODE_KP_5,
                KP_6 = SDL3::SDL_SCANCODE_KP_6,
                KP_7 = SDL3::SDL_SCANCODE_KP_7,
                KP_8 = SDL3::SDL_SCANCODE_KP_8,
                KP_9 = SDL3::SDL_SCANCODE_KP_9,
                KP_0 = SDL3::SDL_SCANCODE_KP_0,
                KP_Period = SDL3::SDL_SCANCODE_KP_PERIOD,
                NonUsBackslash = SDL3::SDL_SCANCODE_NONUSBACKSLASH,
                Application = SDL3::SDL_SCANCODE_APPLICATION,
                Power = SDL3::SDL_SCANCODE_POWER,
                KPEquals = SDL3::SDL_SCANCODE_KP_EQUALS,
                F13 = SDL3::SDL_SCANCODE_F13,
                F14 = SDL3::SDL_SCANCODE_F14,
                F15 = SDL3::SDL_SCANCODE_F15,
                F16 = SDL3::SDL_SCANCODE_F16,
                F17 = SDL3::SDL_SCANCODE_F17,
                F18 = SDL3::SDL_SCANCODE_F18,
                F19 = SDL3::SDL_SCANCODE_F19,
                F20 = SDL3::SDL_SCANCODE_F20,
                F21 = SDL3::SDL_SCANCODE_F21,
                F22 = SDL3::SDL_SCANCODE_F22,
                F23 = SDL3::SDL_SCANCODE_F23,
                F24 = SDL3::SDL_SCANCODE_F24,
                Execute = SDL3::SDL_SCANCODE_EXECUTE,
                Help = SDL3::SDL_SCANCODE_HELP,
                Menu = SDL3::SDL_SCANCODE_MENU,
                Select = SDL3::SDL_SCANCODE_SELECT,
                Stop = SDL3::SDL_SCANCODE_STOP,
                Again = SDL3::SDL_SCANCODE_AGAIN,
                Undo = SDL3::SDL_SCANCODE_UNDO,
                Cut = SDL3::SDL_SCANCODE_CUT,
                Copy = SDL3::SDL_SCANCODE_COPY,
                Paste = SDL3::SDL_SCANCODE_PASTE,
                Find = SDL3::SDL_SCANCODE_FIND,
                Mute = SDL3::SDL_SCANCODE_MUTE,
                VolumeUp = SDL3::SDL_SCANCODE_VOLUMEUP,
                VolumeDown = SDL3::SDL_SCANCODE_VOLUMEDOWN,
                KpComma = SDL3::SDL_SCANCODE_KP_COMMA,
                KpEqualsas400 = SDL3::SDL_SCANCODE_KP_EQUALSAS400,
                International1 = SDL3::SDL_SCANCODE_INTERNATIONAL1,
                International2 = SDL3::SDL_SCANCODE_INTERNATIONAL2,
                International3 = SDL3::SDL_SCANCODE_INTERNATIONAL3,
                International4 = SDL3::SDL_SCANCODE_INTERNATIONAL4,
                International5 = SDL3::SDL_SCANCODE_INTERNATIONAL5,
                International6 = SDL3::SDL_SCANCODE_INTERNATIONAL6,
                International7 = SDL3::SDL_SCANCODE_INTERNATIONAL7,
                International8 = SDL3::SDL_SCANCODE_INTERNATIONAL8,
                International9 = SDL3::SDL_SCANCODE_INTERNATIONAL9,
                Lang1 = SDL3::SDL_SCANCODE_LANG1,
                Lang2 = SDL3::SDL_SCANCODE_LANG2,
                Lang3 = SDL3::SDL_SCANCODE_LANG3,
                Lang4 = SDL3::SDL_SCANCODE_LANG4,
                Lang5 = SDL3::SDL_SCANCODE_LANG5,
                Lang6 = SDL3::SDL_SCANCODE_LANG6,
                Lang7 = SDL3::SDL_SCANCODE_LANG7,
                Lang8 = SDL3::SDL_SCANCODE_LANG8,
                Lang9 = SDL3::SDL_SCANCODE_LANG9,
                Alterase = SDL3::SDL_SCANCODE_ALTERASE,
                Sysreq = SDL3::SDL_SCANCODE_SYSREQ,
                Cancel = SDL3::SDL_SCANCODE_CANCEL,
                Clear = SDL3::SDL_SCANCODE_CLEAR,
                Prior = SDL3::SDL_SCANCODE_PRIOR,
                Return2 = SDL3::SDL_SCANCODE_RETURN2,
                Separator = SDL3::SDL_SCANCODE_SEPARATOR,
                Oout = SDL3::SDL_SCANCODE_OUT,
                Oper = SDL3::SDL_SCANCODE_OPER,
                ClearAgain = SDL3::SDL_SCANCODE_CLEARAGAIN,
                CRSEL = SDL3::SDL_SCANCODE_CRSEL,
                EXSEL = SDL3::SDL_SCANCODE_EXSEL,
                KP00 = SDL3::SDL_SCANCODE_KP_00,
                KP000 = SDL3::SDL_SCANCODE_KP_000,
                ThousandsSeparator = SDL3::SDL_SCANCODE_THOUSANDSSEPARATOR,
                DecimalSeparator = SDL3::SDL_SCANCODE_DECIMALSEPARATOR,
                CurrencyUnit = SDL3::SDL_SCANCODE_CURRENCYUNIT,
                CurrencySubunit = SDL3::SDL_SCANCODE_CURRENCYSUBUNIT,
                KPLeftparen = SDL3::SDL_SCANCODE_KP_LEFTPAREN,
                KPRightparen = SDL3::SDL_SCANCODE_KP_RIGHTPAREN,
                KPLeftbrace = SDL3::SDL_SCANCODE_KP_LEFTBRACE,
                KPRightbrace = SDL3::SDL_SCANCODE_KP_RIGHTBRACE,
                KPTab = SDL3::SDL_SCANCODE_KP_TAB,
                KPBackspace = SDL3::SDL_SCANCODE_KP_BACKSPACE,
                KP_A = SDL3::SDL_SCANCODE_KP_A,
                KP_B = SDL3::SDL_SCANCODE_KP_B,
                KP_C = SDL3::SDL_SCANCODE_KP_C,
                KP_D = SDL3::SDL_SCANCODE_KP_D,
                KP_E = SDL3::SDL_SCANCODE_KP_E,
                KP_F = SDL3::SDL_SCANCODE_KP_F,
                KP_Xor = SDL3::SDL_SCANCODE_KP_XOR,
                KP_Power = SDL3::SDL_SCANCODE_KP_POWER,
                KP_Percent = SDL3::SDL_SCANCODE_KP_PERCENT,
                KP_Less = SDL3::SDL_SCANCODE_KP_LESS,
                KP_Greater = SDL3::SDL_SCANCODE_KP_GREATER,
                KP_Ampersand = SDL3::SDL_SCANCODE_KP_AMPERSAND,
                KP_Dblampersand = SDL3::SDL_SCANCODE_KP_DBLAMPERSAND,
                KP_Verticalbar = SDL3::SDL_SCANCODE_KP_VERTICALBAR,
                KP_Dblverticalbar = SDL3::SDL_SCANCODE_KP_DBLVERTICALBAR,
                KP_Colon = SDL3::SDL_SCANCODE_KP_COLON,
                KP_Hash = SDL3::SDL_SCANCODE_KP_HASH,
                KP_Space = SDL3::SDL_SCANCODE_KP_SPACE,
                KP_At = SDL3::SDL_SCANCODE_KP_AT,
                KP_Exclam = SDL3::SDL_SCANCODE_KP_EXCLAM,
                KP_Memstore = SDL3::SDL_SCANCODE_KP_MEMSTORE,
                KP_Memrecall = SDL3::SDL_SCANCODE_KP_MEMRECALL,
                KP_Memclear = SDL3::SDL_SCANCODE_KP_MEMCLEAR,
                KP_Memadd = SDL3::SDL_SCANCODE_KP_MEMADD,
                KP_Memsubtract = SDL3::SDL_SCANCODE_KP_MEMSUBTRACT,
                KP_Memmultiply = SDL3::SDL_SCANCODE_KP_MEMMULTIPLY,
                KP_Memdivide = SDL3::SDL_SCANCODE_KP_MEMDIVIDE,
                KP_Plusminus = SDL3::SDL_SCANCODE_KP_PLUSMINUS,
                KP_Clear = SDL3::SDL_SCANCODE_KP_CLEAR,
                KP_Clearentry = SDL3::SDL_SCANCODE_KP_CLEARENTRY,
                KP_Binary = SDL3::SDL_SCANCODE_KP_BINARY,
                KP_Octal = SDL3::SDL_SCANCODE_KP_OCTAL,
                KP_Decimal = SDL3::SDL_SCANCODE_KP_DECIMAL,
                KP_Hexadecimal = SDL3::SDL_SCANCODE_KP_HEXADECIMAL,
                LCtrl = SDL3::SDL_SCANCODE_LCTRL,
                LShift = SDL3::SDL_SCANCODE_LSHIFT,
                LAlt = SDL3::SDL_SCANCODE_LALT,
                LGui = SDL3::SDL_SCANCODE_LGUI,
                RCtrl = SDL3::SDL_SCANCODE_RCTRL,
                RShift = SDL3::SDL_SCANCODE_RSHIFT,
                RAlt = SDL3::SDL_SCANCODE_RALT,
                RGui = SDL3::SDL_SCANCODE_RGUI,
                Mode = SDL3::SDL_SCANCODE_MODE,
                AudioNext = SDL3::SDL_SCANCODE_AUDIONEXT,
                AudioPrev = SDL3::SDL_SCANCODE_AUDIOPREV,
                AudioStop = SDL3::SDL_SCANCODE_AUDIOSTOP,
                AudioPlay = SDL3::SDL_SCANCODE_AUDIOPLAY,
                AudioMute = SDL3::SDL_SCANCODE_AUDIOMUTE,
                MediaSelect = SDL3::SDL_SCANCODE_MEDIASELECT,
                WWW = SDL3::SDL_SCANCODE_WWW,
                Mail = SDL3::SDL_SCANCODE_MAIL,
                Calculator = SDL3::SDL_SCANCODE_CALCULATOR,
                Computer = SDL3::SDL_SCANCODE_COMPUTER,
                ACSearch = SDL3::SDL_SCANCODE_AC_SEARCH,
                ACHome = SDL3::SDL_SCANCODE_AC_HOME,
                ACBack = SDL3::SDL_SCANCODE_AC_BACK,
                ACForward = SDL3::SDL_SCANCODE_AC_FORWARD,
                ACStop = SDL3::SDL_SCANCODE_AC_STOP,
                ACRefresh = SDL3::SDL_SCANCODE_AC_REFRESH,
                ACBookmarks = SDL3::SDL_SCANCODE_AC_BOOKMARKS,
                BrightnessDown = SDL3::SDL_SCANCODE_BRIGHTNESSDOWN,
                BrightnessUp = SDL3::SDL_SCANCODE_BRIGHTNESSUP,
                DisplaySwitch = SDL3::SDL_SCANCODE_DISPLAYSWITCH,
                KbdIllumToggle = SDL3::SDL_SCANCODE_KBDILLUMTOGGLE,
                KbdIllumDown = SDL3::SDL_SCANCODE_KBDILLUMDOWN,
                KbdIlluMup = SDL3::SDL_SCANCODE_KBDILLUMUP,
                Eject = SDL3::SDL_SCANCODE_EJECT,
                Sleep = SDL3::SDL_SCANCODE_SLEEP,
                App1 = SDL3::SDL_SCANCODE_APP1,
                App2 = SDL3::SDL_SCANCODE_APP2,
                AudioRewind = SDL3::SDL_SCANCODE_AUDIOREWIND,
                AudioFastforward = SDL3::SDL_SCANCODE_AUDIOFASTFORWARD,
                SoftLeft = SDL3::SDL_SCANCODE_SOFTLEFT,
                SoftRight = SDL3::SDL_SCANCODE_SOFTRIGHT,
                Call = SDL3::SDL_SCANCODE_CALL,
                EndCall = SDL3::SDL_SCANCODE_ENDCALL,
                Count = SDL3::SDL_NUM_SCANCODES,

                Ctrl = LCtrl | RCtrl,
                Shift = LShift | RShift,
                Modifiers = Ctrl | Shift,
            };
        };

        // Keyboard Keycode Identifiers
        struct Key
        {
            using type = SDL3::SDL_KeyCode;

            enum ID : std::underlying_type_t<type> {
                UNKNOWN = SDL3::SDLK_UNKNOWN,
                RETURN = SDL3::SDLK_RETURN,
                ESCAPE = SDL3::SDLK_ESCAPE,
                BACKSPACE = SDL3::SDLK_BACKSPACE,
                TAB = SDL3::SDLK_TAB,
                SPACE = SDL3::SDLK_SPACE,
                EXCLAIM = SDL3::SDLK_EXCLAIM,
                QUOTEDBL = SDL3::SDLK_QUOTEDBL,
                HASH = SDL3::SDLK_HASH,
                PERCENT = SDL3::SDLK_PERCENT,
                DOLLAR = SDL3::SDLK_DOLLAR,
                AMPERSAND = SDL3::SDLK_AMPERSAND,
                QUOTE = SDL3::SDLK_QUOTE,
                LEFTPAREN = SDL3::SDLK_LEFTPAREN,
                RIGHTPAREN = SDL3::SDLK_RIGHTPAREN,
                ASTERISK = SDL3::SDLK_ASTERISK,
                PLUS = SDL3::SDLK_PLUS,
                COMMA = SDL3::SDLK_COMMA,
                MINUS = SDL3::SDLK_MINUS,
                PERIOD = SDL3::SDLK_PERIOD,
                SLASH = SDL3::SDLK_SLASH,
                Zero = SDL3::SDLK_0,
                One = SDL3::SDLK_1,
                Two = SDL3::SDLK_2,
                Three = SDL3::SDLK_3,
                Four = SDL3::SDLK_4,
                Five = SDL3::SDLK_5,
                Six = SDL3::SDLK_6,
                Seven = SDL3::SDLK_7,
                Eight = SDL3::SDLK_8,
                None = SDL3::SDLK_9,
                Colon = SDL3::SDLK_COLON,
                Semicolon = SDL3::SDLK_SEMICOLON,
                Less = SDL3::SDLK_LESS,
                Equals = SDL3::SDLK_EQUALS,
                Greater = SDL3::SDLK_GREATER,
                Question = SDL3::SDLK_QUESTION,
                At = SDL3::SDLK_AT,
                LeftBracket = SDL3::SDLK_LEFTBRACKET,
                BackSlash = SDL3::SDLK_BACKSLASH,
                RightBracket = SDL3::SDLK_RIGHTBRACKET,
                Caret = SDL3::SDLK_CARET,
                Underscore = SDL3::SDLK_UNDERSCORE,
                BackQuote = SDL3::SDLK_BACKQUOTE,
                A = SDL3::SDLK_a,
                B = SDL3::SDLK_b,
                C = SDL3::SDLK_c,
                D = SDL3::SDLK_d,
                E = SDL3::SDLK_e,
                F = SDL3::SDLK_f,
                G = SDL3::SDLK_g,
                H = SDL3::SDLK_h,
                I = SDL3::SDLK_i,
                J = SDL3::SDLK_j,
                K = SDL3::SDLK_k,
                L = SDL3::SDLK_l,
                M = SDL3::SDLK_m,
                N = SDL3::SDLK_n,
                O = SDL3::SDLK_o,
                P = SDL3::SDLK_p,
                Q = SDL3::SDLK_q,
                R = SDL3::SDLK_r,
                S = SDL3::SDLK_s,
                T = SDL3::SDLK_t,
                U = SDL3::SDLK_u,
                V = SDL3::SDLK_v,
                W = SDL3::SDLK_w,
                X = SDL3::SDLK_x,
                Y = SDL3::SDLK_y,
                Z = SDL3::SDLK_z,
                CapsLock = SDL3::SDLK_CAPSLOCK,
                F1 = SDL3::SDLK_F1,
                F2 = SDL3::SDLK_F2,
                F3 = SDL3::SDLK_F3,
                F4 = SDL3::SDLK_F4,
                F5 = SDL3::SDLK_F5,
                F6 = SDL3::SDLK_F6,
                F7 = SDL3::SDLK_F7,
                F8 = SDL3::SDLK_F8,
                F9 = SDL3::SDLK_F9,
                F10 = SDL3::SDLK_F10,
                F11 = SDL3::SDLK_F11,
                F12 = SDL3::SDLK_F12,
                PrintScreen = SDL3::SDLK_PRINTSCREEN,
                ScrollLock = SDL3::SDLK_SCROLLLOCK,
                Pause = SDL3::SDLK_PAUSE,
                Insert = SDL3::SDLK_INSERT,
                Home = SDL3::SDLK_HOME,
                PageUp = SDL3::SDLK_PAGEUP,
                Delete = SDL3::SDLK_DELETE,
                End = SDL3::SDLK_END,
                PageDown = SDL3::SDLK_PAGEDOWN,
                Right = SDL3::SDLK_RIGHT,
                Left = SDL3::SDLK_LEFT,
                Down = SDL3::SDLK_DOWN,
                Up = SDL3::SDLK_UP,
                NumlockClear = SDL3::SDLK_NUMLOCKCLEAR,
                KP_DIVIDE = SDL3::SDLK_KP_DIVIDE,
                KP_MULTIPLY = SDL3::SDLK_KP_MULTIPLY,
                KP_MINUS = SDL3::SDLK_KP_MINUS,
                KP_PLUS = SDL3::SDLK_KP_PLUS,
                KP_ENTER = SDL3::SDLK_KP_ENTER,
                KP_1 = SDL3::SDLK_KP_1,
                KP_2 = SDL3::SDLK_KP_2,
                KP_3 = SDL3::SDLK_KP_3,
                KP_4 = SDL3::SDLK_KP_4,
                KP_5 = SDL3::SDLK_KP_5,
                KP_6 = SDL3::SDLK_KP_6,
                KP_7 = SDL3::SDLK_KP_7,
                KP_8 = SDL3::SDLK_KP_8,
                KP_9 = SDL3::SDLK_KP_9,
                KP_0 = SDL3::SDLK_KP_0,
                KP_PERIOD = SDL3::SDLK_KP_PERIOD,
                Application = SDL3::SDLK_APPLICATION,
                Power = SDL3::SDLK_POWER,
                KP_Equals = SDL3::SDLK_KP_EQUALS,
                F13 = SDL3::SDLK_F13,
                F14 = SDL3::SDLK_F14,
                F15 = SDL3::SDLK_F15,
                F16 = SDL3::SDLK_F16,
                F17 = SDL3::SDLK_F17,
                F18 = SDL3::SDLK_F18,
                F19 = SDL3::SDLK_F19,
                F20 = SDL3::SDLK_F20,
                F21 = SDL3::SDLK_F21,
                F22 = SDL3::SDLK_F22,
                F23 = SDL3::SDLK_F23,
                F24 = SDL3::SDLK_F24,
                Execute = SDL3::SDLK_EXECUTE,
                Help = SDL3::SDLK_HELP,
                Menu = SDL3::SDLK_MENU,
                Select = SDL3::SDLK_SELECT,
                Stop = SDL3::SDLK_STOP,
                Again = SDL3::SDLK_AGAIN,
                Undo = SDL3::SDLK_UNDO,
                Cut = SDL3::SDLK_CUT,
                Copy = SDL3::SDLK_COPY,
                Paste = SDL3::SDLK_PASTE,
                Find = SDL3::SDLK_FIND,
                Mute = SDL3::SDLK_MUTE,
                VolumeUp = SDL3::SDLK_VOLUMEUP,
                VolumeDown = SDL3::SDLK_VOLUMEDOWN,
                KP_Comma = SDL3::SDLK_KP_COMMA,
                KP_Equalsas400 = SDL3::SDLK_KP_EQUALSAS400,
                AltErase = SDL3::SDLK_ALTERASE,
                SysReq = SDL3::SDLK_SYSREQ,
                Cancel = SDL3::SDLK_CANCEL,
                Clear = SDL3::SDLK_CLEAR,
                Prior = SDL3::SDLK_PRIOR,
                Return2 = SDL3::SDLK_RETURN2,
                Separator = SDL3::SDLK_SEPARATOR,
                Out = SDL3::SDLK_OUT,
                Oper = SDL3::SDLK_OPER,
                ClearAgain = SDL3::SDLK_CLEARAGAIN,
                CrSel = SDL3::SDLK_CRSEL,
                ExSel = SDL3::SDLK_EXSEL,
                KP_00 = SDL3::SDLK_KP_00,
                KP_000 = SDL3::SDLK_KP_000,
                THOUSANDS_SEPARATOR = SDL3::SDLK_THOUSANDSSEPARATOR,
                DECIMAL_SEPARATOR = SDL3::SDLK_DECIMALSEPARATOR,
                CURRENCY_UNIT = SDL3::SDLK_CURRENCYUNIT,
                CURRENCY_SUBUNIT = SDL3::SDLK_CURRENCYSUBUNIT,
                KP_LeftParen = SDL3::SDLK_KP_LEFTPAREN,
                KP_RightParen = SDL3::SDLK_KP_RIGHTPAREN,
                KP_LeftBrace = SDL3::SDLK_KP_LEFTBRACE,
                KP_RightBrace = SDL3::SDLK_KP_RIGHTBRACE,
                KP_Tab = SDL3::SDLK_KP_TAB,
                KP_Backspace = SDL3::SDLK_KP_BACKSPACE,
                KP_A = SDL3::SDLK_KP_A,
                KP_B = SDL3::SDLK_KP_B,
                KP_C = SDL3::SDLK_KP_C,
                KP_D = SDL3::SDLK_KP_D,
                KP_E = SDL3::SDLK_KP_E,
                KP_F = SDL3::SDLK_KP_F,
                KP_Xor = SDL3::SDLK_KP_XOR,
                KP_Power = SDL3::SDLK_KP_POWER,
                KP_Percent = SDL3::SDLK_KP_PERCENT,
                KP_Less = SDL3::SDLK_KP_LESS,
                KP_Greater = SDL3::SDLK_KP_GREATER,
                KP_Ampersand = SDL3::SDLK_KP_AMPERSAND,
                KP_DblAmpersand = SDL3::SDLK_KP_DBLAMPERSAND,
                KP_Verticalbar = SDL3::SDLK_KP_VERTICALBAR,
                KP_DblVerticalBar = SDL3::SDLK_KP_DBLVERTICALBAR,
                KP_Colon = SDL3::SDLK_KP_COLON,
                KP_Hash = SDL3::SDLK_KP_HASH,
                KP_Space = SDL3::SDLK_KP_SPACE,
                KP_At = SDL3::SDLK_KP_AT,
                KP_Exclam = SDL3::SDLK_KP_EXCLAM,
                KP_MemStore = SDL3::SDLK_KP_MEMSTORE,
                KP_MemRecall = SDL3::SDLK_KP_MEMRECALL,
                KP_MemClear = SDL3::SDLK_KP_MEMCLEAR,
                KP_MemAdd = SDL3::SDLK_KP_MEMADD,
                KP_MemSubtract = SDL3::SDLK_KP_MEMSUBTRACT,
                KP_MemMultiply = SDL3::SDLK_KP_MEMMULTIPLY,
                KP_MemDivide = SDL3::SDLK_KP_MEMDIVIDE,
                KP_PlusMinus = SDL3::SDLK_KP_PLUSMINUS,
                KP_Clear = SDL3::SDLK_KP_CLEAR,
                KP_ClearEntry = SDL3::SDLK_KP_CLEARENTRY,
                KP_Binary = SDL3::SDLK_KP_BINARY,
                KP_Octal = SDL3::SDLK_KP_OCTAL,
                KP_Decimal = SDL3::SDLK_KP_DECIMAL,
                KP_Hexadecimal = SDL3::SDLK_KP_HEXADECIMAL,
                LCtrl = SDL3::SDLK_LCTRL,
                LShift = SDL3::SDLK_LSHIFT,
                LAlt = SDL3::SDLK_LALT,
                LGui = SDL3::SDLK_LGUI,
                RCtrl = SDL3::SDLK_RCTRL,
                RShift = SDL3::SDLK_RSHIFT,
                RAlt = SDL3::SDLK_RALT,
                RGui = SDL3::SDLK_RGUI,
                MODE = SDL3::SDLK_MODE,
                AudioNext = SDL3::SDLK_AUDIONEXT,
                AudioPrev = SDL3::SDLK_AUDIOPREV,
                AudioStop = SDL3::SDLK_AUDIOSTOP,
                AudioPlay = SDL3::SDLK_AUDIOPLAY,
                AudioMute = SDL3::SDLK_AUDIOMUTE,
                MediaSelect = SDL3::SDLK_MEDIASELECT,
                WWW = SDL3::SDLK_WWW,
                Mail = SDL3::SDLK_MAIL,
                Calculator = SDL3::SDLK_CALCULATOR,
                Computer = SDL3::SDLK_COMPUTER,
                ACSearch = SDL3::SDLK_AC_SEARCH,
                ACHome = SDL3::SDLK_AC_HOME,
                ACBack = SDL3::SDLK_AC_BACK,
                ACForward = SDL3::SDLK_AC_FORWARD,
                ACStop = SDL3::SDLK_AC_STOP,
                ACRefresh = SDL3::SDLK_AC_REFRESH,
                ACBookmarks = SDL3::SDLK_AC_BOOKMARKS,
                BrightnessDown = SDL3::SDLK_BRIGHTNESSDOWN,
                BrightnessUp = SDL3::SDLK_BRIGHTNESSUP,
                DisplaySwitch = SDL3::SDLK_DISPLAYSWITCH,
                KbdIllumToggle = SDL3::SDLK_KBDILLUMTOGGLE,
                KbdIllumDown = SDL3::SDLK_KBDILLUMDOWN,
                KbdIllumUp = SDL3::SDLK_KBDILLUMUP,
                Eject = SDL3::SDLK_EJECT,
                Sleep = SDL3::SDLK_SLEEP,
                App1 = SDL3::SDLK_APP1,
                App2 = SDL3::SDLK_APP2,
                AudioRewind = SDL3::SDLK_AUDIOREWIND,
                AudioFastForward = SDL3::SDLK_AUDIOFASTFORWARD,
                SoftLeft = SDL3::SDLK_SOFTLEFT,
                SoftRight = SDL3::SDLK_SOFTRIGHT,
                Call = SDL3::SDLK_CALL,
                EndCall = SDL3::SDLK_ENDCALL,
            };
        };

    public:
        [[nodiscard]] std::string get_inputted_text() const;
        [[nodiscard]] std::string get_inputted_text_compisition() const;
        [[nodiscard]] i32 get_inputted_text_cursor_loc() const;
        [[nodiscard]] i32 get_inputted_text_length() const;
        [[nodiscard]] Scancode::ID keys_down() const;
        [[nodiscard]] std::string get_key_state(Scancode::ID key) const;

        [[nodiscard]] bool is_button_pressed(Scancode::ID key) const;
        [[nodiscard]] bool is_button_released(Scancode::ID key) const;
        [[nodiscard]] bool is_button_held(Scancode::ID key) const;
        [[nodiscard]] bool is_button_down(Scancode::ID key) const;
        [[nodiscard]] bool all_buttons_down(const std::vector<Scancode::ID>& keys) const;
        [[nodiscard]] bool any_buttons_down(const std::vector<Scancode::ID>& keys) const;

    protected:
        void process_button_down(Scancode::ID key);
        void process_button_up(Scancode::ID key);
        void process_text_input(const char* text);
        void process_text_editing(const char* composition, i32 start, i32 length);

    private:
        i32 m_cursor_pos{ 0 };
        i32 m_text_length{ 0 };
        std::string m_text{};
        std::string m_composition{};
        std::bitset<Scancode::Count> m_held{ 0 };
        std::bitset<Scancode::Count> m_pressed{ 0 };
        mutable std::bitset<Scancode::Count> m_released{ 0 };
    };
}

namespace rl {
    auto format_as(const Keyboard& kb);
}
