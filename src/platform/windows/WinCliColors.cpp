// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WinCliColors.h"

#include <string>

// console color heck by mlocati: https://gist.github.com/mlocati/21a9233ac83f7d3d7837535bc109b3b7
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

namespace WinCliColors
{
    bool consoleHasColorSupport()
    {
        constexpr DWORD MINV_MAJOR = 10;
        constexpr DWORD MINV_MINOR = 0;
        constexpr DWORD MINV_BUILD = 10586;

        HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
        if (!hMod) {
            return false;
        }

        auto rlGetVersion = reinterpret_cast<RtlGetVersionPtr>(GetProcAddress(hMod, "RtlGetVersion"));
        if (!rlGetVersion) {
            return false;
        }

        RTL_OSVERSIONINFOW version_info{};
        version_info.dwOSVersionInfoSize = sizeof(version_info);
        if (rlGetVersion(&version_info) != 0) {
            return false;
        }

        if (version_info.dwMajorVersion > MINV_MAJOR) {
            return true;
        }
        if (version_info.dwMajorVersion == MINV_MAJOR) {
            if (version_info.dwMinorVersion > MINV_MINOR) {
                return true;
            }
            if (version_info.dwMinorVersion == MINV_MINOR && version_info.dwBuildNumber >= MINV_BUILD) {
                return true;
            }
        }

        return false;
    }

    bool enableConsoleColor(bool enabled)
    {
        if (!consoleHasColorSupport()) {
            return false;
        }

        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut == INVALID_HANDLE_VALUE) {
            return false;
        }

        DWORD mode;
        if (!GetConsoleMode(hStdOut, &mode)) {
            return false;
        }

        bool vtEnabled = (mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
        if (vtEnabled == enabled) {
            return true;
        }

        if (enabled) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        } else {
            mode &= ~ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        }

        return SetConsoleMode(hStdOut, mode) != 0;
    }

    // ----------------------------------------------------------------------------

    void printRed(std::string message)
    {
        return fmt::print(fmt::fg(fmt::color::red), "{}", message);
    }
    void printDarkOrange(std::string message)
    {
        return fmt::print(fmt::fg(fmt::color::dark_orange), "{}", message);
    }
    void printYellow(std::string message)
    {
        return fmt::print(fmt::fg(fmt::color::yellow), "{}", message);
    }
    void printGreen(std::string message)
    {
        return fmt::print(fmt::fg(fmt::color::green), "{}", message);
    }
    void printTerminalYellow(std::string message)
    {
        return fmt::print(fmt::fg(fmt::terminal_color::yellow), "{}", message);
    }

    // ----------------------------------------------------------------------------

    std::string formatRed(std::string message)
    {
        return fmt::format(fmt::fg(fmt::color::red), "{}", message);
    }
    std::string formatDarkOrange(std::string message)
    {
        return fmt::format(fmt::fg(fmt::color::dark_orange), "{}", message);
    }
    std::string formatYellow(std::string message)
    {
        return fmt::format(fmt::fg(fmt::color::yellow), "{}", message);
    }
    std::string formatGreen(std::string message)
    {
        return fmt::format(fmt::fg(fmt::color::green), "{}", message);
    }
    std::string formatLightGreen(std::string message)
    {
        return fmt::format(fmt::fg(fmt::color::light_green), "{}", message);
    }
    std::string formatTerminalYellow(std::string message)
    {
        return fmt::format(fmt::fg(fmt::terminal_color::yellow), "{}", message);
    }

} // namespace WinCliColors
