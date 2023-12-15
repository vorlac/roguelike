#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <map>
#include <thread>

#include "gui/nanovg.h"
#include "gui/screen.hpp"
#include "sdl/defs.hpp"

#if defined(_WIN32)
  #include <windows.h>
#endif

#if !defined(_WIN32)
  #include <locale.h>
  #include <signal.h>
  #include <sys/dir.h>
#endif

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
SDL_C_LIB_END

#pragma warning(disable : 4244)

namespace rl::gui {
    NVGcolor Color::toNvgColor() const
    {
        return reinterpret_cast<const NVGcolor&>(this->_d);
    }

    extern std::map<SDL3::SDL_Window*, Screen*> __sdlgui_screens;

    PntRect srect2pntrect(const SDL3::SDL_FRect& srect)
    {
        return {
            static_cast<int>(srect.x),
            static_cast<int>(srect.y),
            static_cast<int>(srect.x + srect.w),
            static_cast<int>(srect.y + srect.h),
        };
    }

    SDL3::SDL_FRect pntrect2srect(const PntRect& frect)
    {
        return {
            static_cast<float>(frect.x1),
            static_cast<float>(frect.y1),
            static_cast<float>(frect.x2 - frect.x1),
            static_cast<float>(frect.y2 - frect.y1),
        };
    }

    SDL3::SDL_FRect clip_rects(SDL3::SDL_FRect af, const SDL3::SDL_FRect& bf)
    {
        PntFRect a{ af.x, af.y, af.x + af.w, af.y + af.h };
        PntFRect b{ bf.x, bf.y, bf.x + bf.w, bf.y + bf.h };
        if (a.x1 < b.x1)
            a.x1 = b.x1;
        if (a.y1 < b.y1)
            a.y1 = b.y1;
        if (b.x2 < a.x2)
            a.x2 = b.x2;
        if (b.y2 < a.y2)
            a.y2 = b.y2;

        return { a.x1, a.y1, a.x2 - a.x1, a.y2 - a.y1 };
    }

    PntRect clip_rects(PntRect a, const PntRect& b)
    {
        if (a.x1 < b.x1)
            a.x1 = b.x1;
        if (a.y1 < b.y1)
            a.y1 = b.y1;
        if (b.x2 < a.x2)
            a.x2 = b.x2;
        if (b.y2 < a.y2)
            a.y2 = b.y2;

        return a;
    }

    SDL3::SDL_Color Color::toSdlColor() const
    {
        SDL3::SDL_Color color{ (uint8_t)std::round(r() * 255), (uint8_t)std::round(g() * 255),
                               (uint8_t)std::round(b() * 255), (uint8_t)std::round(a() * 255) };
        return color;
    }

    std::array<char, 8> utf8(int c)
    {
        std::array<char, 8> seq;
        int n = 0;
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
        else
            n = 6;

        seq[n] = '\0';

        switch (n)
        {
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
        return seq;
    }

#if !defined(__APPLE__)
    std::string file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes,
                            bool save)
    {
  #define FILE_DIALOG_MAX_BUFFER 1024
  #if defined(_WIN32)
        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        char tmp[FILE_DIALOG_MAX_BUFFER];
        ofn.lpstrFile = tmp;
        ZeroMemory(tmp, FILE_DIALOG_MAX_BUFFER);
        ofn.nMaxFile = FILE_DIALOG_MAX_BUFFER;
        ofn.nFilterIndex = 1;

        std::string filter;

        if (!save && filetypes.size() > 1)
        {
            filter.append("Supported file types (");
            for (size_t i = 0; i < filetypes.size(); ++i)
            {
                filter.append("*.");
                filter.append(filetypes[i].first);
                if (i + 1 < filetypes.size())
                    filter.append(";");
            }
            filter.append(")");
            filter.push_back('\0');
            for (size_t i = 0; i < filetypes.size(); ++i)
            {
                filter.append("*.");
                filter.append(filetypes[i].first);
                if (i + 1 < filetypes.size())
                    filter.append(";");
            }
            filter.push_back('\0');
        }
        for (auto pair : filetypes)
        {
            filter.append(pair.second);
            filter.append(" (*.");
            filter.append(pair.first);
            filter.append(")");
            filter.push_back('\0');
            filter.append("*.");
            filter.append(pair.first);
            filter.push_back('\0');
        }
        filter.push_back('\0');
        ofn.lpstrFilter = filter.data();

        if (save)
        {
            ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
            if (GetSaveFileNameA(&ofn) == FALSE)
                return "";
        }
        else
        {
            ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (GetOpenFileNameA(&ofn) == FALSE)
                return "";
        }
        return std::string(ofn.lpstrFile);
  #else
        char buffer[FILE_DIALOG_MAX_BUFFER];
        std::string cmd = "/usr/bin/zenity --file-selection ";
        if (save)
            cmd += "--save ";
        cmd += "--file-filter=\"";
        for (auto pair : filetypes)
            cmd += "\"*." + pair.first + "\" ";
        cmd += "\"";
        FILE* output = popen(cmd.c_str(), "r");
        if (output == nullptr)
            throw std::runtime_error("popen() failed -- could not launch zenity!");
        while (fgets(buffer, FILE_DIALOG_MAX_BUFFER, output) != NULL)
            ;
        pclose(output);
        std::string result(buffer);
        result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
        return result;
  #endif
    }
#endif

    void Object::decRef(bool dealloc) const noexcept
    {
        --m_refCount;
        if (m_refCount == 0 && dealloc)
        {
            delete this;
        }
        else if (m_refCount < 0)
        {
            fprintf(stderr, "Internal error: Object reference count < 0!\n");
            abort();
        }
    }

    Object::~Object()
    {
    }
}
