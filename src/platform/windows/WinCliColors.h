#ifndef WinCliColors_H
#define WinCliColors_H

#include <fmt/color.h>
#include <fmt/format.h>

#include "Windows.h"

namespace WinCliColors
{
    bool consoleHasColorSupport();
    bool enableConsoleColor(bool enabled);

    void printRed(std::string message);
    void printDarkOrange(std::string message);
    void printYellow(std::string message);
    void printGreen(std::string message);
    void printTerminalYellow(std::string message);

    std::string formatRed(std::string message);
    std::string formatDarkOrange(std::string message);
    std::string formatYellow(std::string message);
    std::string formatGreen(std::string message);
    std::string formatLightGreen(std::string message);
    std::string formatTerminalYellow(std::string message);

} // namespace WinCliColors

#endif