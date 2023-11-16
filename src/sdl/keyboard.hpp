#pragma once

#include <bitset>

#include <fmt/format.h>

namespace SDL3 {
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
}

namespace rl::sdl {
    class Keyboard
    {
    public:
        /**
         * @brief Keyboard Specific Event Identifiers
         * */
        struct Event
        {
            using type = SDL3::SDL_EventType;
            constexpr static inline type KeyDown = SDL3::SDL_EVENT_KEY_DOWN;
            constexpr static inline type KeyUp = SDL3::SDL_EVENT_KEY_UP;
            constexpr static inline type TextEditing = SDL3::SDL_EVENT_TEXT_EDITING;
            constexpr static inline type TextInput = SDL3::SDL_EVENT_TEXT_INPUT;
            constexpr static inline type KeymapChanged = SDL3::SDL_EVENT_KEYMAP_CHANGED;
        };

        /**
         * @brief Keyboard Scancode Identifiers
         * */
        struct Button
        {
            using type = SDL3::SDL_Scancode;
            constexpr static inline type Unknown = SDL3::SDL_SCANCODE_UNKNOWN;
            constexpr static inline type A = SDL3::SDL_SCANCODE_A;
            constexpr static inline type B = SDL3::SDL_SCANCODE_B;
            constexpr static inline type C = SDL3::SDL_SCANCODE_C;
            constexpr static inline type D = SDL3::SDL_SCANCODE_D;
            constexpr static inline type E = SDL3::SDL_SCANCODE_E;
            constexpr static inline type F = SDL3::SDL_SCANCODE_F;
            constexpr static inline type G = SDL3::SDL_SCANCODE_G;
            constexpr static inline type H = SDL3::SDL_SCANCODE_H;
            constexpr static inline type I = SDL3::SDL_SCANCODE_I;
            constexpr static inline type J = SDL3::SDL_SCANCODE_J;
            constexpr static inline type K = SDL3::SDL_SCANCODE_K;
            constexpr static inline type L = SDL3::SDL_SCANCODE_L;
            constexpr static inline type M = SDL3::SDL_SCANCODE_M;
            constexpr static inline type N = SDL3::SDL_SCANCODE_N;
            constexpr static inline type O = SDL3::SDL_SCANCODE_O;
            constexpr static inline type P = SDL3::SDL_SCANCODE_P;
            constexpr static inline type Q = SDL3::SDL_SCANCODE_Q;
            constexpr static inline type R = SDL3::SDL_SCANCODE_R;
            constexpr static inline type S = SDL3::SDL_SCANCODE_S;
            constexpr static inline type T = SDL3::SDL_SCANCODE_T;
            constexpr static inline type U = SDL3::SDL_SCANCODE_U;
            constexpr static inline type V = SDL3::SDL_SCANCODE_V;
            constexpr static inline type W = SDL3::SDL_SCANCODE_W;
            constexpr static inline type X = SDL3::SDL_SCANCODE_X;
            constexpr static inline type Y = SDL3::SDL_SCANCODE_Y;
            constexpr static inline type Z = SDL3::SDL_SCANCODE_Z;
            constexpr static inline type One = SDL3::SDL_SCANCODE_1;
            constexpr static inline type Two = SDL3::SDL_SCANCODE_2;
            constexpr static inline type Three = SDL3::SDL_SCANCODE_3;
            constexpr static inline type Four = SDL3::SDL_SCANCODE_4;
            constexpr static inline type Five = SDL3::SDL_SCANCODE_5;
            constexpr static inline type Six = SDL3::SDL_SCANCODE_6;
            constexpr static inline type Seven = SDL3::SDL_SCANCODE_7;
            constexpr static inline type Eight = SDL3::SDL_SCANCODE_8;
            constexpr static inline type Nine = SDL3::SDL_SCANCODE_9;
            constexpr static inline type Zero = SDL3::SDL_SCANCODE_0;
            constexpr static inline type Return = SDL3::SDL_SCANCODE_RETURN;
            constexpr static inline type Escape = SDL3::SDL_SCANCODE_ESCAPE;
            constexpr static inline type Backspace = SDL3::SDL_SCANCODE_BACKSPACE;
            constexpr static inline type Tab = SDL3::SDL_SCANCODE_TAB;
            constexpr static inline type Space = SDL3::SDL_SCANCODE_SPACE;
            constexpr static inline type Minus = SDL3::SDL_SCANCODE_MINUS;
            constexpr static inline type Equals = SDL3::SDL_SCANCODE_EQUALS;
            constexpr static inline type Leftbracket = SDL3::SDL_SCANCODE_LEFTBRACKET;
            constexpr static inline type Rightbracket = SDL3::SDL_SCANCODE_RIGHTBRACKET;
            constexpr static inline type Backslash = SDL3::SDL_SCANCODE_BACKSLASH;
            constexpr static inline type Nonushash = SDL3::SDL_SCANCODE_NONUSHASH;
            constexpr static inline type Semicolon = SDL3::SDL_SCANCODE_SEMICOLON;
            constexpr static inline type Apostrophe = SDL3::SDL_SCANCODE_APOSTROPHE;
            constexpr static inline type Grave = SDL3::SDL_SCANCODE_GRAVE;
            constexpr static inline type Comma = SDL3::SDL_SCANCODE_COMMA;
            constexpr static inline type Period = SDL3::SDL_SCANCODE_PERIOD;
            constexpr static inline type Slash = SDL3::SDL_SCANCODE_SLASH;
            constexpr static inline type CapsLock = SDL3::SDL_SCANCODE_CAPSLOCK;
            constexpr static inline type F1 = SDL3::SDL_SCANCODE_F1;
            constexpr static inline type F2 = SDL3::SDL_SCANCODE_F2;
            constexpr static inline type F3 = SDL3::SDL_SCANCODE_F3;
            constexpr static inline type F4 = SDL3::SDL_SCANCODE_F4;
            constexpr static inline type F5 = SDL3::SDL_SCANCODE_F5;
            constexpr static inline type F6 = SDL3::SDL_SCANCODE_F6;
            constexpr static inline type F7 = SDL3::SDL_SCANCODE_F7;
            constexpr static inline type F8 = SDL3::SDL_SCANCODE_F8;
            constexpr static inline type F9 = SDL3::SDL_SCANCODE_F9;
            constexpr static inline type F10 = SDL3::SDL_SCANCODE_F10;
            constexpr static inline type F11 = SDL3::SDL_SCANCODE_F11;
            constexpr static inline type F12 = SDL3::SDL_SCANCODE_F12;
            constexpr static inline type PrintScreen = SDL3::SDL_SCANCODE_PRINTSCREEN;
            constexpr static inline type ScrollLock = SDL3::SDL_SCANCODE_SCROLLLOCK;
            constexpr static inline type Pause = SDL3::SDL_SCANCODE_PAUSE;
            constexpr static inline type Insert = SDL3::SDL_SCANCODE_INSERT;
            constexpr static inline type Home = SDL3::SDL_SCANCODE_HOME;
            constexpr static inline type PageUp = SDL3::SDL_SCANCODE_PAGEUP;
            constexpr static inline type Delete = SDL3::SDL_SCANCODE_DELETE;
            constexpr static inline type End = SDL3::SDL_SCANCODE_END;
            constexpr static inline type PageDown = SDL3::SDL_SCANCODE_PAGEDOWN;
            constexpr static inline type Right = SDL3::SDL_SCANCODE_RIGHT;
            constexpr static inline type Left = SDL3::SDL_SCANCODE_LEFT;
            constexpr static inline type Down = SDL3::SDL_SCANCODE_DOWN;
            constexpr static inline type Up = SDL3::SDL_SCANCODE_UP;
            constexpr static inline type NumLockClear = SDL3::SDL_SCANCODE_NUMLOCKCLEAR;
            constexpr static inline type KPDivide = SDL3::SDL_SCANCODE_KP_DIVIDE;
            constexpr static inline type KPMultiply = SDL3::SDL_SCANCODE_KP_MULTIPLY;
            constexpr static inline type KPMinus = SDL3::SDL_SCANCODE_KP_MINUS;
            constexpr static inline type KPPlus = SDL3::SDL_SCANCODE_KP_PLUS;
            constexpr static inline type KPEnter = SDL3::SDL_SCANCODE_KP_ENTER;
            constexpr static inline type KP_1 = SDL3::SDL_SCANCODE_KP_1;
            constexpr static inline type KP_2 = SDL3::SDL_SCANCODE_KP_2;
            constexpr static inline type KP_3 = SDL3::SDL_SCANCODE_KP_3;
            constexpr static inline type KP_4 = SDL3::SDL_SCANCODE_KP_4;
            constexpr static inline type KP_5 = SDL3::SDL_SCANCODE_KP_5;
            constexpr static inline type KP_6 = SDL3::SDL_SCANCODE_KP_6;
            constexpr static inline type KP_7 = SDL3::SDL_SCANCODE_KP_7;
            constexpr static inline type KP_8 = SDL3::SDL_SCANCODE_KP_8;
            constexpr static inline type KP_9 = SDL3::SDL_SCANCODE_KP_9;
            constexpr static inline type KP_0 = SDL3::SDL_SCANCODE_KP_0;
            constexpr static inline type KP_Period = SDL3::SDL_SCANCODE_KP_PERIOD;
            constexpr static inline type NonUsBackslash = SDL3::SDL_SCANCODE_NONUSBACKSLASH;
            constexpr static inline type Application = SDL3::SDL_SCANCODE_APPLICATION;
            constexpr static inline type Power = SDL3::SDL_SCANCODE_POWER;
            constexpr static inline type KPEquals = SDL3::SDL_SCANCODE_KP_EQUALS;
            constexpr static inline type F13 = SDL3::SDL_SCANCODE_F13;
            constexpr static inline type F14 = SDL3::SDL_SCANCODE_F14;
            constexpr static inline type F15 = SDL3::SDL_SCANCODE_F15;
            constexpr static inline type F16 = SDL3::SDL_SCANCODE_F16;
            constexpr static inline type F17 = SDL3::SDL_SCANCODE_F17;
            constexpr static inline type F18 = SDL3::SDL_SCANCODE_F18;
            constexpr static inline type F19 = SDL3::SDL_SCANCODE_F19;
            constexpr static inline type F20 = SDL3::SDL_SCANCODE_F20;
            constexpr static inline type F21 = SDL3::SDL_SCANCODE_F21;
            constexpr static inline type F22 = SDL3::SDL_SCANCODE_F22;
            constexpr static inline type F23 = SDL3::SDL_SCANCODE_F23;
            constexpr static inline type F24 = SDL3::SDL_SCANCODE_F24;
            constexpr static inline type Execute = SDL3::SDL_SCANCODE_EXECUTE;
            constexpr static inline type Help = SDL3::SDL_SCANCODE_HELP;
            constexpr static inline type Menu = SDL3::SDL_SCANCODE_MENU;
            constexpr static inline type Select = SDL3::SDL_SCANCODE_SELECT;
            constexpr static inline type Stop = SDL3::SDL_SCANCODE_STOP;
            constexpr static inline type Again = SDL3::SDL_SCANCODE_AGAIN;
            constexpr static inline type Undo = SDL3::SDL_SCANCODE_UNDO;
            constexpr static inline type Cut = SDL3::SDL_SCANCODE_CUT;
            constexpr static inline type Copy = SDL3::SDL_SCANCODE_COPY;
            constexpr static inline type Paste = SDL3::SDL_SCANCODE_PASTE;
            constexpr static inline type Find = SDL3::SDL_SCANCODE_FIND;
            constexpr static inline type Mute = SDL3::SDL_SCANCODE_MUTE;
            constexpr static inline type VolumeUp = SDL3::SDL_SCANCODE_VOLUMEUP;
            constexpr static inline type VolumeDown = SDL3::SDL_SCANCODE_VOLUMEDOWN;
            constexpr static inline type KpComma = SDL3::SDL_SCANCODE_KP_COMMA;
            constexpr static inline type KpEqualsas400 = SDL3::SDL_SCANCODE_KP_EQUALSAS400;
            constexpr static inline type International1 = SDL3::SDL_SCANCODE_INTERNATIONAL1;
            constexpr static inline type International2 = SDL3::SDL_SCANCODE_INTERNATIONAL2;
            constexpr static inline type International3 = SDL3::SDL_SCANCODE_INTERNATIONAL3;
            constexpr static inline type International4 = SDL3::SDL_SCANCODE_INTERNATIONAL4;
            constexpr static inline type International5 = SDL3::SDL_SCANCODE_INTERNATIONAL5;
            constexpr static inline type International6 = SDL3::SDL_SCANCODE_INTERNATIONAL6;
            constexpr static inline type International7 = SDL3::SDL_SCANCODE_INTERNATIONAL7;
            constexpr static inline type International8 = SDL3::SDL_SCANCODE_INTERNATIONAL8;
            constexpr static inline type International9 = SDL3::SDL_SCANCODE_INTERNATIONAL9;
            constexpr static inline type Lang1 = SDL3::SDL_SCANCODE_LANG1;
            constexpr static inline type Lang2 = SDL3::SDL_SCANCODE_LANG2;
            constexpr static inline type Lang3 = SDL3::SDL_SCANCODE_LANG3;
            constexpr static inline type Lang4 = SDL3::SDL_SCANCODE_LANG4;
            constexpr static inline type Lang5 = SDL3::SDL_SCANCODE_LANG5;
            constexpr static inline type Lang6 = SDL3::SDL_SCANCODE_LANG6;
            constexpr static inline type Lang7 = SDL3::SDL_SCANCODE_LANG7;
            constexpr static inline type Lang8 = SDL3::SDL_SCANCODE_LANG8;
            constexpr static inline type Lang9 = SDL3::SDL_SCANCODE_LANG9;
            constexpr static inline type Alterase = SDL3::SDL_SCANCODE_ALTERASE;
            constexpr static inline type Sysreq = SDL3::SDL_SCANCODE_SYSREQ;
            constexpr static inline type Cancel = SDL3::SDL_SCANCODE_CANCEL;
            constexpr static inline type Clear = SDL3::SDL_SCANCODE_CLEAR;
            constexpr static inline type Prior = SDL3::SDL_SCANCODE_PRIOR;
            constexpr static inline type Return2 = SDL3::SDL_SCANCODE_RETURN2;
            constexpr static inline type Separator = SDL3::SDL_SCANCODE_SEPARATOR;
            constexpr static inline type Oout = SDL3::SDL_SCANCODE_OUT;
            constexpr static inline type Oper = SDL3::SDL_SCANCODE_OPER;
            constexpr static inline type ClearAgain = SDL3::SDL_SCANCODE_CLEARAGAIN;
            constexpr static inline type CRSEL = SDL3::SDL_SCANCODE_CRSEL;
            constexpr static inline type EXSEL = SDL3::SDL_SCANCODE_EXSEL;
            constexpr static inline type KP00 = SDL3::SDL_SCANCODE_KP_00;
            constexpr static inline type KP000 = SDL3::SDL_SCANCODE_KP_000;
            constexpr static inline type ThousandsSeparator = SDL3::SDL_SCANCODE_THOUSANDSSEPARATOR;
            constexpr static inline type DecimalSeparator = SDL3::SDL_SCANCODE_DECIMALSEPARATOR;
            constexpr static inline type CurrencyUnit = SDL3::SDL_SCANCODE_CURRENCYUNIT;
            constexpr static inline type CurrencySubunit = SDL3::SDL_SCANCODE_CURRENCYSUBUNIT;
            constexpr static inline type KPLeftparen = SDL3::SDL_SCANCODE_KP_LEFTPAREN;
            constexpr static inline type KPRightparen = SDL3::SDL_SCANCODE_KP_RIGHTPAREN;
            constexpr static inline type KPLeftbrace = SDL3::SDL_SCANCODE_KP_LEFTBRACE;
            constexpr static inline type KPRightbrace = SDL3::SDL_SCANCODE_KP_RIGHTBRACE;
            constexpr static inline type KPTab = SDL3::SDL_SCANCODE_KP_TAB;
            constexpr static inline type KPBackspace = SDL3::SDL_SCANCODE_KP_BACKSPACE;
            constexpr static inline type KP_A = SDL3::SDL_SCANCODE_KP_A;
            constexpr static inline type KP_B = SDL3::SDL_SCANCODE_KP_B;
            constexpr static inline type KP_C = SDL3::SDL_SCANCODE_KP_C;
            constexpr static inline type KP_D = SDL3::SDL_SCANCODE_KP_D;
            constexpr static inline type KP_E = SDL3::SDL_SCANCODE_KP_E;
            constexpr static inline type KP_F = SDL3::SDL_SCANCODE_KP_F;
            constexpr static inline type KP_Xor = SDL3::SDL_SCANCODE_KP_XOR;
            constexpr static inline type KP_Power = SDL3::SDL_SCANCODE_KP_POWER;
            constexpr static inline type KP_Percent = SDL3::SDL_SCANCODE_KP_PERCENT;
            constexpr static inline type KP_Less = SDL3::SDL_SCANCODE_KP_LESS;
            constexpr static inline type KP_Greater = SDL3::SDL_SCANCODE_KP_GREATER;
            constexpr static inline type KP_Ampersand = SDL3::SDL_SCANCODE_KP_AMPERSAND;
            constexpr static inline type KP_Dblampersand = SDL3::SDL_SCANCODE_KP_DBLAMPERSAND;
            constexpr static inline type KP_Verticalbar = SDL3::SDL_SCANCODE_KP_VERTICALBAR;
            constexpr static inline type KP_Dblverticalbar = SDL3::SDL_SCANCODE_KP_DBLVERTICALBAR;
            constexpr static inline type KP_Colon = SDL3::SDL_SCANCODE_KP_COLON;
            constexpr static inline type KP_Hash = SDL3::SDL_SCANCODE_KP_HASH;
            constexpr static inline type KP_Space = SDL3::SDL_SCANCODE_KP_SPACE;
            constexpr static inline type KP_At = SDL3::SDL_SCANCODE_KP_AT;
            constexpr static inline type KP_Exclam = SDL3::SDL_SCANCODE_KP_EXCLAM;
            constexpr static inline type KP_Memstore = SDL3::SDL_SCANCODE_KP_MEMSTORE;
            constexpr static inline type KP_Memrecall = SDL3::SDL_SCANCODE_KP_MEMRECALL;
            constexpr static inline type KP_Memclear = SDL3::SDL_SCANCODE_KP_MEMCLEAR;
            constexpr static inline type KP_Memadd = SDL3::SDL_SCANCODE_KP_MEMADD;
            constexpr static inline type KP_Memsubtract = SDL3::SDL_SCANCODE_KP_MEMSUBTRACT;
            constexpr static inline type KP_Memmultiply = SDL3::SDL_SCANCODE_KP_MEMMULTIPLY;
            constexpr static inline type KP_Memdivide = SDL3::SDL_SCANCODE_KP_MEMDIVIDE;
            constexpr static inline type KP_Plusminus = SDL3::SDL_SCANCODE_KP_PLUSMINUS;
            constexpr static inline type KP_Clear = SDL3::SDL_SCANCODE_KP_CLEAR;
            constexpr static inline type KP_Clearentry = SDL3::SDL_SCANCODE_KP_CLEARENTRY;
            constexpr static inline type KP_Binary = SDL3::SDL_SCANCODE_KP_BINARY;
            constexpr static inline type KP_Octal = SDL3::SDL_SCANCODE_KP_OCTAL;
            constexpr static inline type KP_Decimal = SDL3::SDL_SCANCODE_KP_DECIMAL;
            constexpr static inline type KP_Hexadecimal = SDL3::SDL_SCANCODE_KP_HEXADECIMAL;
            constexpr static inline type LCtrl = SDL3::SDL_SCANCODE_LCTRL;
            constexpr static inline type LShift = SDL3::SDL_SCANCODE_LSHIFT;
            constexpr static inline type LAlt = SDL3::SDL_SCANCODE_LALT;
            constexpr static inline type LGui = SDL3::SDL_SCANCODE_LGUI;
            constexpr static inline type RCtrl = SDL3::SDL_SCANCODE_RCTRL;
            constexpr static inline type RShift = SDL3::SDL_SCANCODE_RSHIFT;
            constexpr static inline type RAlt = SDL3::SDL_SCANCODE_RALT;
            constexpr static inline type RGui = SDL3::SDL_SCANCODE_RGUI;
            constexpr static inline type Mode = SDL3::SDL_SCANCODE_MODE;
            constexpr static inline type AudioNext = SDL3::SDL_SCANCODE_AUDIONEXT;
            constexpr static inline type AudioPrev = SDL3::SDL_SCANCODE_AUDIOPREV;
            constexpr static inline type AudioStop = SDL3::SDL_SCANCODE_AUDIOSTOP;
            constexpr static inline type AudioPlay = SDL3::SDL_SCANCODE_AUDIOPLAY;
            constexpr static inline type AudioMute = SDL3::SDL_SCANCODE_AUDIOMUTE;
            constexpr static inline type MediaSelect = SDL3::SDL_SCANCODE_MEDIASELECT;
            constexpr static inline type WWW = SDL3::SDL_SCANCODE_WWW;
            constexpr static inline type Mail = SDL3::SDL_SCANCODE_MAIL;
            constexpr static inline type Calculator = SDL3::SDL_SCANCODE_CALCULATOR;
            constexpr static inline type Computer = SDL3::SDL_SCANCODE_COMPUTER;
            constexpr static inline type ACSearch = SDL3::SDL_SCANCODE_AC_SEARCH;
            constexpr static inline type ACHome = SDL3::SDL_SCANCODE_AC_HOME;
            constexpr static inline type ACBack = SDL3::SDL_SCANCODE_AC_BACK;
            constexpr static inline type ACForward = SDL3::SDL_SCANCODE_AC_FORWARD;
            constexpr static inline type ACStop = SDL3::SDL_SCANCODE_AC_STOP;
            constexpr static inline type ACRefresh = SDL3::SDL_SCANCODE_AC_REFRESH;
            constexpr static inline type ACBookmarks = SDL3::SDL_SCANCODE_AC_BOOKMARKS;
            constexpr static inline type BrightnessDown = SDL3::SDL_SCANCODE_BRIGHTNESSDOWN;
            constexpr static inline type BrightnessUp = SDL3::SDL_SCANCODE_BRIGHTNESSUP;
            constexpr static inline type DisplaySwitch = SDL3::SDL_SCANCODE_DISPLAYSWITCH;
            constexpr static inline type KbdIllumToggle = SDL3::SDL_SCANCODE_KBDILLUMTOGGLE;
            constexpr static inline type KbdIllumDown = SDL3::SDL_SCANCODE_KBDILLUMDOWN;
            constexpr static inline type KbdIlluMup = SDL3::SDL_SCANCODE_KBDILLUMUP;
            constexpr static inline type Eject = SDL3::SDL_SCANCODE_EJECT;
            constexpr static inline type Sleep = SDL3::SDL_SCANCODE_SLEEP;
            constexpr static inline type App1 = SDL3::SDL_SCANCODE_APP1;
            constexpr static inline type App2 = SDL3::SDL_SCANCODE_APP2;
            constexpr static inline type AudioRewind = SDL3::SDL_SCANCODE_AUDIOREWIND;
            constexpr static inline type AudioFastforward = SDL3::SDL_SCANCODE_AUDIOFASTFORWARD;
            constexpr static inline type SoftLeft = SDL3::SDL_SCANCODE_SOFTLEFT;
            constexpr static inline type SoftRight = SDL3::SDL_SCANCODE_SOFTRIGHT;
            constexpr static inline type Call = SDL3::SDL_SCANCODE_CALL;
            constexpr static inline type EndCall = SDL3::SDL_SCANCODE_ENDCALL;
            constexpr static inline type ScancodeCount = SDL3::SDL_NUM_SCANCODES;
        };

        /**
         * @brief Keyboard Keycode Identifiers
         * */
        struct Key
        {
            using type = SDL3::SDL_KeyCode;
            constexpr static inline type UNKNOWN = SDL3::SDLK_UNKNOWN;
            constexpr static inline type RETURN = SDL3::SDLK_RETURN;
            constexpr static inline type ESCAPE = SDL3::SDLK_ESCAPE;
            constexpr static inline type BACKSPACE = SDL3::SDLK_BACKSPACE;
            constexpr static inline type TAB = SDL3::SDLK_TAB;
            constexpr static inline type SPACE = SDL3::SDLK_SPACE;
            constexpr static inline type EXCLAIM = SDL3::SDLK_EXCLAIM;
            constexpr static inline type QUOTEDBL = SDL3::SDLK_QUOTEDBL;
            constexpr static inline type HASH = SDL3::SDLK_HASH;
            constexpr static inline type PERCENT = SDL3::SDLK_PERCENT;
            constexpr static inline type DOLLAR = SDL3::SDLK_DOLLAR;
            constexpr static inline type AMPERSAND = SDL3::SDLK_AMPERSAND;
            constexpr static inline type QUOTE = SDL3::SDLK_QUOTE;
            constexpr static inline type LEFTPAREN = SDL3::SDLK_LEFTPAREN;
            constexpr static inline type RIGHTPAREN = SDL3::SDLK_RIGHTPAREN;
            constexpr static inline type ASTERISK = SDL3::SDLK_ASTERISK;
            constexpr static inline type PLUS = SDL3::SDLK_PLUS;
            constexpr static inline type COMMA = SDL3::SDLK_COMMA;
            constexpr static inline type MINUS = SDL3::SDLK_MINUS;
            constexpr static inline type PERIOD = SDL3::SDLK_PERIOD;
            constexpr static inline type SLASH = SDL3::SDLK_SLASH;
            constexpr static inline type Zero = SDL3::SDLK_0;
            constexpr static inline type One = SDL3::SDLK_1;
            constexpr static inline type Two = SDL3::SDLK_2;
            constexpr static inline type Three = SDL3::SDLK_3;
            constexpr static inline type Four = SDL3::SDLK_4;
            constexpr static inline type Five = SDL3::SDLK_5;
            constexpr static inline type Six = SDL3::SDLK_6;
            constexpr static inline type Seven = SDL3::SDLK_7;
            constexpr static inline type Eight = SDL3::SDLK_8;
            constexpr static inline type None = SDL3::SDLK_9;
            constexpr static inline type Colon = SDL3::SDLK_COLON;
            constexpr static inline type Semicolon = SDL3::SDLK_SEMICOLON;
            constexpr static inline type Less = SDL3::SDLK_LESS;
            constexpr static inline type Equals = SDL3::SDLK_EQUALS;
            constexpr static inline type Greater = SDL3::SDLK_GREATER;
            constexpr static inline type Question = SDL3::SDLK_QUESTION;
            constexpr static inline type At = SDL3::SDLK_AT;
            constexpr static inline type LeftBracket = SDL3::SDLK_LEFTBRACKET;
            constexpr static inline type BackSlash = SDL3::SDLK_BACKSLASH;
            constexpr static inline type RightBracket = SDL3::SDLK_RIGHTBRACKET;
            constexpr static inline type Caret = SDL3::SDLK_CARET;
            constexpr static inline type Underscore = SDL3::SDLK_UNDERSCORE;
            constexpr static inline type BackQuote = SDL3::SDLK_BACKQUOTE;
            constexpr static inline type A = SDL3::SDLK_a;
            constexpr static inline type B = SDL3::SDLK_b;
            constexpr static inline type C = SDL3::SDLK_c;
            constexpr static inline type D = SDL3::SDLK_d;
            constexpr static inline type E = SDL3::SDLK_e;
            constexpr static inline type F = SDL3::SDLK_f;
            constexpr static inline type G = SDL3::SDLK_g;
            constexpr static inline type H = SDL3::SDLK_h;
            constexpr static inline type I = SDL3::SDLK_i;
            constexpr static inline type J = SDL3::SDLK_j;
            constexpr static inline type K = SDL3::SDLK_k;
            constexpr static inline type L = SDL3::SDLK_l;
            constexpr static inline type M = SDL3::SDLK_m;
            constexpr static inline type N = SDL3::SDLK_n;
            constexpr static inline type O = SDL3::SDLK_o;
            constexpr static inline type P = SDL3::SDLK_p;
            constexpr static inline type Q = SDL3::SDLK_q;
            constexpr static inline type R = SDL3::SDLK_r;
            constexpr static inline type S = SDL3::SDLK_s;
            constexpr static inline type T = SDL3::SDLK_t;
            constexpr static inline type U = SDL3::SDLK_u;
            constexpr static inline type V = SDL3::SDLK_v;
            constexpr static inline type W = SDL3::SDLK_w;
            constexpr static inline type X = SDL3::SDLK_x;
            constexpr static inline type Y = SDL3::SDLK_y;
            constexpr static inline type Z = SDL3::SDLK_z;
            constexpr static inline type CapsLock = SDL3::SDLK_CAPSLOCK;
            constexpr static inline type F1 = SDL3::SDLK_F1;
            constexpr static inline type F2 = SDL3::SDLK_F2;
            constexpr static inline type F3 = SDL3::SDLK_F3;
            constexpr static inline type F4 = SDL3::SDLK_F4;
            constexpr static inline type F5 = SDL3::SDLK_F5;
            constexpr static inline type F6 = SDL3::SDLK_F6;
            constexpr static inline type F7 = SDL3::SDLK_F7;
            constexpr static inline type F8 = SDL3::SDLK_F8;
            constexpr static inline type F9 = SDL3::SDLK_F9;
            constexpr static inline type F10 = SDL3::SDLK_F10;
            constexpr static inline type F11 = SDL3::SDLK_F11;
            constexpr static inline type F12 = SDL3::SDLK_F12;
            constexpr static inline type PrintScreen = SDL3::SDLK_PRINTSCREEN;
            constexpr static inline type ScrollLock = SDL3::SDLK_SCROLLLOCK;
            constexpr static inline type Pause = SDL3::SDLK_PAUSE;
            constexpr static inline type Insert = SDL3::SDLK_INSERT;
            constexpr static inline type Home = SDL3::SDLK_HOME;
            constexpr static inline type PageUp = SDL3::SDLK_PAGEUP;
            constexpr static inline type Delete = SDL3::SDLK_DELETE;
            constexpr static inline type End = SDL3::SDLK_END;
            constexpr static inline type PageDown = SDL3::SDLK_PAGEDOWN;
            constexpr static inline type Right = SDL3::SDLK_RIGHT;
            constexpr static inline type Left = SDL3::SDLK_LEFT;
            constexpr static inline type Down = SDL3::SDLK_DOWN;
            constexpr static inline type Up = SDL3::SDLK_UP;
            constexpr static inline type NumlockClear = SDL3::SDLK_NUMLOCKCLEAR;
            constexpr static inline type KP_DIVIDE = SDL3::SDLK_KP_DIVIDE;
            constexpr static inline type KP_MULTIPLY = SDL3::SDLK_KP_MULTIPLY;
            constexpr static inline type KP_MINUS = SDL3::SDLK_KP_MINUS;
            constexpr static inline type KP_PLUS = SDL3::SDLK_KP_PLUS;
            constexpr static inline type KP_ENTER = SDL3::SDLK_KP_ENTER;
            constexpr static inline type KP_1 = SDL3::SDLK_KP_1;
            constexpr static inline type KP_2 = SDL3::SDLK_KP_2;
            constexpr static inline type KP_3 = SDL3::SDLK_KP_3;
            constexpr static inline type KP_4 = SDL3::SDLK_KP_4;
            constexpr static inline type KP_5 = SDL3::SDLK_KP_5;
            constexpr static inline type KP_6 = SDL3::SDLK_KP_6;
            constexpr static inline type KP_7 = SDL3::SDLK_KP_7;
            constexpr static inline type KP_8 = SDL3::SDLK_KP_8;
            constexpr static inline type KP_9 = SDL3::SDLK_KP_9;
            constexpr static inline type KP_0 = SDL3::SDLK_KP_0;
            constexpr static inline type KP_PERIOD = SDL3::SDLK_KP_PERIOD;
            constexpr static inline type Application = SDL3::SDLK_APPLICATION;
            constexpr static inline type Power = SDL3::SDLK_POWER;
            constexpr static inline type KP_Equals = SDL3::SDLK_KP_EQUALS;
            constexpr static inline type F13 = SDL3::SDLK_F13;
            constexpr static inline type F14 = SDL3::SDLK_F14;
            constexpr static inline type F15 = SDL3::SDLK_F15;
            constexpr static inline type F16 = SDL3::SDLK_F16;
            constexpr static inline type F17 = SDL3::SDLK_F17;
            constexpr static inline type F18 = SDL3::SDLK_F18;
            constexpr static inline type F19 = SDL3::SDLK_F19;
            constexpr static inline type F20 = SDL3::SDLK_F20;
            constexpr static inline type F21 = SDL3::SDLK_F21;
            constexpr static inline type F22 = SDL3::SDLK_F22;
            constexpr static inline type F23 = SDL3::SDLK_F23;
            constexpr static inline type F24 = SDL3::SDLK_F24;
            constexpr static inline type Execute = SDL3::SDLK_EXECUTE;
            constexpr static inline type Help = SDL3::SDLK_HELP;
            constexpr static inline type Menu = SDL3::SDLK_MENU;
            constexpr static inline type Select = SDL3::SDLK_SELECT;
            constexpr static inline type Stop = SDL3::SDLK_STOP;
            constexpr static inline type Again = SDL3::SDLK_AGAIN;
            constexpr static inline type Undo = SDL3::SDLK_UNDO;
            constexpr static inline type Cut = SDL3::SDLK_CUT;
            constexpr static inline type Copy = SDL3::SDLK_COPY;
            constexpr static inline type Paste = SDL3::SDLK_PASTE;
            constexpr static inline type Find = SDL3::SDLK_FIND;
            constexpr static inline type Mute = SDL3::SDLK_MUTE;
            constexpr static inline type VolumeUp = SDL3::SDLK_VOLUMEUP;
            constexpr static inline type VolumeDown = SDL3::SDLK_VOLUMEDOWN;
            constexpr static inline type KP_Comma = SDL3::SDLK_KP_COMMA;
            constexpr static inline type KP_Equalsas400 = SDL3::SDLK_KP_EQUALSAS400;
            constexpr static inline type AltErase = SDL3::SDLK_ALTERASE;
            constexpr static inline type SysReq = SDL3::SDLK_SYSREQ;
            constexpr static inline type Cancel = SDL3::SDLK_CANCEL;
            constexpr static inline type Clear = SDL3::SDLK_CLEAR;
            constexpr static inline type Prior = SDL3::SDLK_PRIOR;
            constexpr static inline type Return2 = SDL3::SDLK_RETURN2;
            constexpr static inline type Separator = SDL3::SDLK_SEPARATOR;
            constexpr static inline type Out = SDL3::SDLK_OUT;
            constexpr static inline type Oper = SDL3::SDLK_OPER;
            constexpr static inline type ClearAgain = SDL3::SDLK_CLEARAGAIN;
            constexpr static inline type CrSel = SDL3::SDLK_CRSEL;
            constexpr static inline type ExSel = SDL3::SDLK_EXSEL;
            constexpr static inline type KP_00 = SDL3::SDLK_KP_00;
            constexpr static inline type KP_000 = SDL3::SDLK_KP_000;
            constexpr static inline type THOUSANDS_SEPARATOR = SDL3::SDLK_THOUSANDSSEPARATOR;
            constexpr static inline type DECIMAL_SEPARATOR = SDL3::SDLK_DECIMALSEPARATOR;
            constexpr static inline type CURRENCY_UNIT = SDL3::SDLK_CURRENCYUNIT;
            constexpr static inline type CURRENCY_SUBUNIT = SDL3::SDLK_CURRENCYSUBUNIT;
            constexpr static inline type KP_LeftParen = SDL3::SDLK_KP_LEFTPAREN;
            constexpr static inline type KP_RightParen = SDL3::SDLK_KP_RIGHTPAREN;
            constexpr static inline type KP_LeftBrace = SDL3::SDLK_KP_LEFTBRACE;
            constexpr static inline type KP_RightBrace = SDL3::SDLK_KP_RIGHTBRACE;
            constexpr static inline type KP_Tab = SDL3::SDLK_KP_TAB;
            constexpr static inline type KP_Backspace = SDL3::SDLK_KP_BACKSPACE;
            constexpr static inline type KP_A = SDL3::SDLK_KP_A;
            constexpr static inline type KP_B = SDL3::SDLK_KP_B;
            constexpr static inline type KP_C = SDL3::SDLK_KP_C;
            constexpr static inline type KP_D = SDL3::SDLK_KP_D;
            constexpr static inline type KP_E = SDL3::SDLK_KP_E;
            constexpr static inline type KP_F = SDL3::SDLK_KP_F;
            constexpr static inline type KP_Xor = SDL3::SDLK_KP_XOR;
            constexpr static inline type KP_Power = SDL3::SDLK_KP_POWER;
            constexpr static inline type KP_Percent = SDL3::SDLK_KP_PERCENT;
            constexpr static inline type KP_Less = SDL3::SDLK_KP_LESS;
            constexpr static inline type KP_Greater = SDL3::SDLK_KP_GREATER;
            constexpr static inline type KP_Ampersand = SDL3::SDLK_KP_AMPERSAND;
            constexpr static inline type KP_DblAmpersand = SDL3::SDLK_KP_DBLAMPERSAND;
            constexpr static inline type KP_Verticalbar = SDL3::SDLK_KP_VERTICALBAR;
            constexpr static inline type KP_DblVerticalBar = SDL3::SDLK_KP_DBLVERTICALBAR;
            constexpr static inline type KP_Colon = SDL3::SDLK_KP_COLON;
            constexpr static inline type KP_Hash = SDL3::SDLK_KP_HASH;
            constexpr static inline type KP_Space = SDL3::SDLK_KP_SPACE;
            constexpr static inline type KP_At = SDL3::SDLK_KP_AT;
            constexpr static inline type KP_Exclam = SDL3::SDLK_KP_EXCLAM;
            constexpr static inline type KP_MemStore = SDL3::SDLK_KP_MEMSTORE;
            constexpr static inline type KP_MemRecall = SDL3::SDLK_KP_MEMRECALL;
            constexpr static inline type KP_MemClear = SDL3::SDLK_KP_MEMCLEAR;
            constexpr static inline type KP_MemAdd = SDL3::SDLK_KP_MEMADD;
            constexpr static inline type KP_MemSubtract = SDL3::SDLK_KP_MEMSUBTRACT;
            constexpr static inline type KP_MemMultiply = SDL3::SDLK_KP_MEMMULTIPLY;
            constexpr static inline type KP_MemDivide = SDL3::SDLK_KP_MEMDIVIDE;
            constexpr static inline type KP_PlusMinus = SDL3::SDLK_KP_PLUSMINUS;
            constexpr static inline type KP_Clear = SDL3::SDLK_KP_CLEAR;
            constexpr static inline type KP_ClearEntry = SDL3::SDLK_KP_CLEARENTRY;
            constexpr static inline type KP_Binary = SDL3::SDLK_KP_BINARY;
            constexpr static inline type KP_Octal = SDL3::SDLK_KP_OCTAL;
            constexpr static inline type KP_Decimal = SDL3::SDLK_KP_DECIMAL;
            constexpr static inline type KP_Hexadecimal = SDL3::SDLK_KP_HEXADECIMAL;
            constexpr static inline type LCtrl = SDL3::SDLK_LCTRL;
            constexpr static inline type LShift = SDL3::SDLK_LSHIFT;
            constexpr static inline type LAlt = SDL3::SDLK_LALT;
            constexpr static inline type LGui = SDL3::SDLK_LGUI;
            constexpr static inline type RCtrl = SDL3::SDLK_RCTRL;
            constexpr static inline type RShift = SDL3::SDLK_RSHIFT;
            constexpr static inline type RAlt = SDL3::SDLK_RALT;
            constexpr static inline type RGui = SDL3::SDLK_RGUI;
            constexpr static inline type MODE = SDL3::SDLK_MODE;
            constexpr static inline type AudioNext = SDL3::SDLK_AUDIONEXT;
            constexpr static inline type AudioPrev = SDL3::SDLK_AUDIOPREV;
            constexpr static inline type AudioStop = SDL3::SDLK_AUDIOSTOP;
            constexpr static inline type AudioPlay = SDL3::SDLK_AUDIOPLAY;
            constexpr static inline type AudioMute = SDL3::SDLK_AUDIOMUTE;
            constexpr static inline type MediaSelect = SDL3::SDLK_MEDIASELECT;
            constexpr static inline type WWW = SDL3::SDLK_WWW;
            constexpr static inline type Mail = SDL3::SDLK_MAIL;
            constexpr static inline type Calculator = SDL3::SDLK_CALCULATOR;
            constexpr static inline type Computer = SDL3::SDLK_COMPUTER;
            constexpr static inline type ACSearch = SDL3::SDLK_AC_SEARCH;
            constexpr static inline type ACHome = SDL3::SDLK_AC_HOME;
            constexpr static inline type ACBack = SDL3::SDLK_AC_BACK;
            constexpr static inline type ACForward = SDL3::SDLK_AC_FORWARD;
            constexpr static inline type ACStop = SDL3::SDLK_AC_STOP;
            constexpr static inline type ACRefresh = SDL3::SDLK_AC_REFRESH;
            constexpr static inline type ACBookmarks = SDL3::SDLK_AC_BOOKMARKS;
            constexpr static inline type BrightnessDown = SDL3::SDLK_BRIGHTNESSDOWN;
            constexpr static inline type BrightnessUp = SDL3::SDLK_BRIGHTNESSUP;
            constexpr static inline type DisplaySwitch = SDL3::SDLK_DISPLAYSWITCH;
            constexpr static inline type KbdIllumToggle = SDL3::SDLK_KBDILLUMTOGGLE;
            constexpr static inline type KbdIllumDown = SDL3::SDLK_KBDILLUMDOWN;
            constexpr static inline type KbdIllumUp = SDL3::SDLK_KBDILLUMUP;
            constexpr static inline type Eject = SDL3::SDLK_EJECT;
            constexpr static inline type Sleep = SDL3::SDLK_SLEEP;
            constexpr static inline type App1 = SDL3::SDLK_APP1;
            constexpr static inline type App2 = SDL3::SDLK_APP2;
            constexpr static inline type AudioRewind = SDL3::SDLK_AUDIOREWIND;
            constexpr static inline type AudioFastForward = SDL3::SDLK_AUDIOFASTFORWARD;
            constexpr static inline type SoftLeft = SDL3::SDLK_SOFTLEFT;
            constexpr static inline type SoftRight = SDL3::SDLK_SOFTRIGHT;
            constexpr static inline type Call = SDL3::SDLK_CALL;
            constexpr static inline type EndCall = SDL3::SDLK_ENDCALL;
        };

    public:
        inline void process_button_down(Keyboard::Button::type kb_button)
        {
            if (this->is_button_down(kb_button))
                m_buttons_held[kb_button] = 1;
            else
                m_buttons_pressed[kb_button] = 1;
        }

        inline void process_button_up(Keyboard::Button::type kb_button)
        {
            m_buttons_pressed[kb_button] = 0;
            m_buttons_held[kb_button] = 0;
        }

        inline bool is_button_down(const Keyboard::Button::type kb_button) const
        {
            return m_buttons_pressed[kb_button] != 0;
        }

        inline bool is_button_held(const Keyboard::Button::type kb_button) const
        {
            return m_buttons_pressed[kb_button] != 0;
        }

        inline std::string get_key_state(const Keyboard::Button::type kb_button) const
        {
            return this->is_button_held(kb_button) ? "Held"
                 : this->is_button_down(kb_button) ? "Pressed"
                 : this->is_button_down(kb_button) ? "Released"
                                                   : "None";
        }

    private:
        std::bitset<Button::ScancodeCount> m_buttons_pressed{ 0 };
        std::bitset<Button::ScancodeCount> m_buttons_held{ 0 };
    };

    inline auto format_as(const Keyboard& kb)
    {
        return fmt::format("KB[W={} A={}, S={}, D={}]", kb.get_key_state(Keyboard::Button::W),
                           kb.get_key_state(Keyboard::Button::A),
                           kb.get_key_state(Keyboard::Button::S),
                           kb.get_key_state(Keyboard::Button::D));
    }
}
