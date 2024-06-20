// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmd.help.h"

void printHelp()
{
    std::string const binary  = WinCliColors::formatLightGreen(CSGO_CLI_BINARYNAME);
    std::string const version = WinCliColors::formatYellow(CSGO_CLI_VERSION);

    fmt::print("{} v{}, {}\n", binary, version, CSGO_CLI_WEBSITE);
    fmt::print("Copyright (c) 2018-{} Jens A. Koch.\n", getYear());
    fmt::print("\n");
    fmt::print(" CS:GO Console shows your user account, stats and latest matches.\n");
    fmt::print(" You can also use the tool to upload replay sharecodes to csgostats.gg.\n");
    fmt::print("\n");
    fmt::print(" Usage:\n");
    fmt::print("   command [options] [arguments]\n");
    fmt::print("\n");
    fmt::print(" Available commands:\n");
    fmt::print("  -user            Show your Steam and CS:GO profile\n");
    fmt::print("  -matches         Show your past matches in table form\n");
    fmt::print("  -upload          Upload your past matches to csgostats.gg\n");
    fmt::print("  -s, sharecode    Upload a replay sharecode to csgostats.gg\n");
    // fmt::print("  -scoreboard      Show your past matches in scoreboard form\n");
    fmt::print("  -globalstats     Show global server stats\n");
    fmt::print("\n");
    fmt::print("  -V, Version      Display application version\n");
    fmt::print("  -h, help         Display this help message\n");
    fmt::print("\n");
    fmt::print("Options:\n");
    fmt::print("  -v, verbose      Increase verbosity of messages\n");
    fmt::print("  -vv              Raise verbosity level to debug\n");
    fmt::print("\n");
}