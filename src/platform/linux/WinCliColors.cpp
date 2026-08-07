// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "WinCliColors.h"

#include <string>

namespace WinCliColors
{
    bool consoleHasColorSupport()
    { return true; }

    bool enableConsoleColor([[maybe_unused]] bool enabled)
    { return true; }

    void printRed(std::string message)
    { fmt::print(fmt::fg(fmt::color::red), "{}", message); }
    void printDarkOrange(std::string message)
    { fmt::print(fmt::fg(fmt::color::dark_orange), "{}", message); }
    void printYellow(std::string message)
    { fmt::print(fmt::fg(fmt::color::yellow), "{}", message); }
    void printGreen(std::string message)
    { fmt::print(fmt::fg(fmt::color::green), "{}", message); }
    void printTerminalYellow(std::string message)
    { fmt::print(fmt::fg(fmt::terminal_color::yellow), "{}", message); }

    std::string formatRed(std::string message)
    { return fmt::format(fmt::fg(fmt::color::red), "{}", message); }
    std::string formatDarkOrange(std::string message)
    { return fmt::format(fmt::fg(fmt::color::dark_orange), "{}", message); }
    std::string formatYellow(std::string message)
    { return fmt::format(fmt::fg(fmt::color::yellow), "{}", message); }
    std::string formatGreen(std::string message)
    { return fmt::format(fmt::fg(fmt::color::green), "{}", message); }
    std::string formatLightGreen(std::string message)
    { return fmt::format(fmt::fg(fmt::color::light_green), "{}", message); }
    std::string formatTerminalYellow(std::string message)
    { return fmt::format(fmt::fg(fmt::terminal_color::yellow), "{}", message); }

} // namespace WinCliColors