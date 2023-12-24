#include "gui/screen.hpp"

#if defined(_WIN32)
  #ifndef NOMINMAX
    #define NOMINMAX 1
  #endif
  #ifndef LEAN_AND_MEAN
    #define LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>

#include "gui/opengl.hpp"

#if !defined(_WIN32)
  #include <dirent.h>
  #include <locale.h>
  #include <signal.h>
#endif

namespace rl::gui {

    extern std::map<GLFWwindow*, Screen*> __nanogui_screens;

    void init()
    {
#if !defined(_WIN32)
        /* Avoid locale-related number parsing issues */
        setlocale(LC_NUMERIC, "C");
#endif

        glfwSetErrorCallback([](int error, const char* descr) {
            if (error == GLFW_NOT_INITIALIZED)
                return; /* Ignore */
            std::cerr << "GLFW error " << error << ": " << descr << std::endl;
        });

        if (!glfwInit())
            throw std::runtime_error("Could not initialize GLFW!");

        glfwSetTime(0);
    }

    static bool mainloop_active = false;

    std::mutex m_async_mutex;
    std::vector<std::function<void()>> m_async_functions;

    void mainloop(float refresh)
    {
        if (mainloop_active)
            throw std::runtime_error("Main loop is already running!");

        auto mainloop_iteration = []() {
            int num_screens = 0;

            {
                std::lock_guard<std::mutex> guard(m_async_mutex);
                for (auto& f : m_async_functions)
                    f();
                m_async_functions.clear();
            }

            for (auto kv : __nanogui_screens)
            {
                Screen* screen = kv.second;
                if (!screen->visible())
                {
                    continue;
                }
                else if (glfwWindowShouldClose(screen->glfw_window()))
                {
                    screen->set_visible(false);
                    continue;
                }

                screen->draw_all();
                num_screens++;
            }

            if (num_screens == 0)
            {
                // Give up if there was nothing to draw
                mainloop_active = false;
                return;
            }

            // Wait for mouse/keyboard or empty refresh events
            glfwWaitEvents();
        };

        mainloop_active = true;

        std::thread refresh_thread;
        std::chrono::microseconds quantum;
        size_t quantum_count = 1;
        if (refresh >= 0)
        {
            quantum = std::chrono::microseconds((int64_t)(refresh * 1'000));
            while (quantum.count() > 50'000)
            {
                quantum /= 2;
                quantum_count *= 2;
            }
        }
        else
        {
            quantum = std::chrono::microseconds(50'000);
            quantum_count = std::numeric_limits<size_t>::max();
        }

        // If there are no mouse/keyboard events, try to refresh the
        // view roughly every 50 ms (default); this is to support animations
        // such as progress bars while keeping the system load
        // reasonably low
        refresh_thread = std::thread([quantum, quantum_count]() {
            while (true)
            {
                for (size_t i = 0; i < quantum_count; ++i)
                {
                    if (!mainloop_active)
                        return;
                    std::this_thread::sleep_for(quantum);
                    for (auto kv : __nanogui_screens)
                        if (kv.second->tooltip_fade_in_progress())
                            kv.second->redraw();
                }
                for (auto kv : __nanogui_screens)
                    kv.second->redraw();
            }
        });

        try
        {
            while (mainloop_active)
                mainloop_iteration();

            /* Process events once more */
            glfwPollEvents();
        }
        catch (const std::exception& e)
        {
            std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
            leave();
        }

        refresh_thread.join();
    }

    void async(const std::function<void()>& func)
    {
        std::lock_guard<std::mutex> guard(m_async_mutex);
        m_async_functions.push_back(func);
    }

    void leave()
    {
        mainloop_active = false;
    }

    bool active()
    {
        return mainloop_active;
    }

    std::pair<bool, bool> test_10bit_edr_support()
    {
        return { false, false };
    }

    void shutdown()
    {
        glfwTerminate();
    }

    std::string utf8(uint32_t c)
    {
        char seq[8];
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
        else if (c <= 0x7fffffff)
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
        return std::string(seq, seq + n);
    }

    int __nanogui_get_image(NVGcontext* ctx, const std::string& name, uint8_t* data, uint32_t size)
    {
        static std::map<std::string, int> icon_cache;
        auto it = icon_cache.find(name);
        if (it != icon_cache.end())
            return it->second;
        int icon_id = nvgCreateImageMem(ctx, 0, data, size);
        if (icon_id == 0)
            throw std::runtime_error("Unable to load resource data.");
        icon_cache[name] = icon_id;
        return icon_id;
    }

    std::vector<std::pair<int, std::string>> load_image_directory(NVGcontext* ctx,
                                                                  const std::string& path)
    {
        std::vector<std::pair<int, std::string>> result;
#if !defined(_WIN32)
        DIR* dp = opendir(path.c_str());
        if (!dp)
            throw std::runtime_error("Could not open image directory!");
        struct dirent* ep;
        while ((ep = readdir(dp)))
        {
            const char* fname = ep->d_name;
#else
        WIN32_FIND_DATA ffd;
        std::string search_path = path + "/*.*";
        HANDLE handle = FindFirstFileA(search_path.c_str(), &ffd);
        if (handle == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Could not open image directory!");
        do
        {
            const char* fname = ffd.cFileName;
#endif
            if (strstr(fname, "png") == nullptr)
                continue;
            std::string full_name = path + "/" + std::string(fname);
            int img = nvgCreateImage(ctx, full_name.c_str(), 0);
            if (img == 0)
                throw std::runtime_error("Could not open image data!");
            result.push_back(std::make_pair(img, full_name.substr(0, full_name.length() - 4)));
#if !defined(_WIN32)
        }
        closedir(dp);
#else
        }
        while (FindNextFileA(handle, &ffd) != 0);
        FindClose(handle);
#endif
        return result;
    }

    std::string file_dialog(const std::vector<std::pair<std::string, std::string>>& filetypes,
                            bool save)
    {
        auto result = file_dialog(filetypes, save, false);
        return result.empty() ? "" : result.front();
    }

    std::vector<std::string> file_dialog(
        const std::vector<std::pair<std::string, std::string>>& filetypes, bool save, bool multiple)
    {
        const static int FILE_DIALOG_MAX_BUFFER = 16384;
        if (save && multiple)
            throw std::invalid_argument("save and multiple must not both be true.");

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
                return {};
        }
        else
        {
            ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            if (multiple)
                ofn.Flags |= OFN_ALLOWMULTISELECT;
            if (GetOpenFileNameA(&ofn) == FALSE)
                return {};
        }

        size_t i = 0;
        std::vector<std::string> result;
        while (tmp[i] != '\0')
        {
            result.emplace_back(&tmp[i]);
            i += result.back().size() + 1;
        }

        if (result.size() > 1)
        {
            for (i = 1; i < result.size(); ++i)
                result[i] = result[0] + "\\" + result[i];
            result.erase(begin(result));
        }

        if (save && ofn.nFilterIndex > 0)
        {
            auto ext = filetypes[ofn.nFilterIndex - 1].first;
            if (ext != "*")
            {
                ext.insert(0, ".");

                auto& name = result.front();
                if (name.size() <= ext.size() ||
                    name.compare(name.size() - ext.size(), ext.size(), ext) != 0)
                {
                    name.append(ext);
                }
            }
        }

        return result;
#else
        char buffer[FILE_DIALOG_MAX_BUFFER];
        buffer[0] = '\0';

        std::string cmd = "zenity --file-selection ";
        // The safest separator for multiple selected paths is /, since / can never occur
        // in file names. Only where two paths are concatenated will there be two / following
        // each other.
        if (multiple)
            cmd += "--multiple --separator=\"/\" ";
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
        std::string paths(buffer);
        paths.erase(std::remove(paths.begin(), paths.end(), '\n'), paths.end());

        std::vector<std::string> result;
        while (!paths.empty())
        {
            size_t end = paths.find("//");
            if (end == std::string::npos)
            {
                result.emplace_back(paths);
                paths = "";
            }
            else
            {
                result.emplace_back(paths.substr(0, end));
                paths = paths.substr(end + 1);
            }
        }

        return result;
#endif
    }

    Object::~Object()
    {
    }

    void Object::inc_ref() const noexcept
    {
        uintptr_t value = m_state.load(std::memory_order_relaxed);

        while (true)
        {
            assert((value & 1) != 0);
            if (!m_state.compare_exchange_weak(value, value + 2, std::memory_order_relaxed,
                                               std::memory_order_relaxed))
                continue;
            break;
        }
    }

    void Object::dec_ref() const noexcept
    {
        uintptr_t value = m_state.load(std::memory_order_relaxed);

        while (true)
        {
            assert((value & 1) != 0);

            if (value == 1)
            {
                fprintf(stderr, "Object::dec_ref(%p): reference count underflow!", this);
                abort();
            }
            else if (value == 3)
            {
                delete this;
            }
            else
            {
                if (!m_state.compare_exchange_weak(value, value - 2, std::memory_order_relaxed,
                                                   std::memory_order_relaxed))
                    continue;
            }
            break;
        }
    }
}
