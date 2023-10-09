#pragma once

#include <array>
#include <utility>
#include <raylib.h>
#include <string_view>

namespace rl
{
    constexpr inline std::array
        = std::make_array<std::pair<KeyboardKey, std::string_view>> KeyLabelMapp{
              { KEY_A, "A" },           { KEY_B, "B" },
              { KEY_C, "C" },           { KEY_D, "D" },
              { KEY_E, "E" },           { KEY_F, "F" },
              { KEY_G, "G" },           { KEY_H, "H" },
              { KEY_I, "I" },           { KEY_J, "J" },
              { KEY_K, "K" },           { KEY_L, "L" },
              { KEY_M, "M" },           { KEY_N, "N" },
              { KEY_O, "O" },           { KEY_P, "P" },
              { KEY_Q, "Q" },           { KEY_R, "R" },
              { KEY_S, "S" },           { KEY_T, "T" },
              { KEY_U, "U" },           { KEY_V, "V" },
              { KEY_W, "W" },           { KEY_X, "X" },
              { KEY_Y, "Y" },           { KEY_Z, "Z" },
              { KEY_ONE, "One" },       { KEY_TWO, "Two" },
              { KEY_THREE, "Three" },   { KEY_FOUR, "Four" },
              { KEY_FIVE, "Five" },     { KEY_SIX, "Six" },
              { KEY_SEVEN, "Seven" },   { KEY_EIGHT, "Eight" },
              { KEY_NINE, "Nine" },     { KEY_ZERO, "Zero" },
              { KEY_SPACE, "Space" },   { KEY_ESCAPE, "Escape" },
              { KEY_UP, "Up" },         { KEY_DOWN, "Down" },
              { KEY_LEFT, "Left" },     { KEY_RIGHT, "Right" },
              { KEY_ENTER, "Enter" },   { KEY_BACKSPACE, "Backspace" },
              { KEY_KP_ADD, "Kp_Add" }, { KEY_KP_SUBTRACT, "Kp_Subtract" },

          };
}
