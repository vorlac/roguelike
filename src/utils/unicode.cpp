#include <string>

#include "utils/numeric.hpp"

namespace rl {
    std::string utf8(u32 c)
    {
        char seq[8] = { 0 };
        i32 n{ 0 };

        if (c < 0x80)
            n = 1;
        else if (c < 0x800)
            n = 2;
        else if (c < 0x10000)
            n = 3;
        else if (c < 0x200000)
            n = 4;
        else if (c < 0x4000000)
            n = 5;
        else if (c <= 0x7fffffff)
            n = 6;

        seq[n] = '\0';

        switch (n) {
            case 6:
                seq[5] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x4000000;
                [[fallthrough]];
            case 5:
                seq[4] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x200000;
                [[fallthrough]];
            case 4:
                seq[3] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x10000;
                [[fallthrough]];
            case 3:
                seq[2] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0x800;
                [[fallthrough]];
            case 2:
                seq[1] = 0x80 | (c & 0x3f);
                c = c >> 6;
                c |= 0xc0;
                [[fallthrough]];
            case 1:
                seq[0] = static_cast<char>(c);
        }
        return std::string(seq, seq + n);
    }
}
