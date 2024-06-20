// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmd.globalstats.h"

bool requestGlobalStats(DataObject& data, bool& verbose)
{

    if (verbose) {
        spdlog::info("global stats are part of user profile data");
        spdlog::info("[ Start ] [ Thread ] getUserInfo");
    }

    bool result = false;

    auto hellothread = std::thread([&data, verbose, &result]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

            CSGOMMHello mmhello;
            if (verbose)
                spdlog::info("          Requesting: Hello");
            mmhello.RefreshWait();
            if (verbose)
                spdlog::info("          Got Hello");

            result = true;

            if (verbose) {
                spdlog::debug("mmhello.data.DebugString {}", mmhello.data.DebugString());
            }

            data.global_stats.ongoing_matches   = mmhello.data.global_stats().ongoing_matches();
            data.global_stats.players_online    = mmhello.data.global_stats().players_online();
            data.global_stats.players_searching = mmhello.data.global_stats().players_searching();
            data.global_stats.servers_online    = mmhello.data.global_stats().servers_online();
            data.global_stats.servers_available = mmhello.data.global_stats().servers_available();
            data.global_stats.search_time_avg   = mmhello.data.global_stats().search_time_avg();

            // Detailed Search Statistics (players searching per game_type)
            // data.global_stats.                  mmhello.data.global_stats().search_statistics();

        } catch (CSGO_CLI_TimeoutException) {
            printError("Warning", "Timeout on receiving UserInfo.");
            result = false;
        } catch (ExceptionHandler& e) {
            printError("Fatal error", e.what());
            result = false;
        }
        if (verbose)
            spdlog::info("[ End   ] [ Thread ] getUserInfo");
        return 0;
    });

    hellothread.join();

    return result;
}

void printGlobalStats(DataObject& data)
{
    // ---------- Format Output Strings

    // ---------- Output Table

    auto const printAligned{[=](std::string const & a, std::string const & b = "") {
        return fmt::print(" {0:<23} {1}\n", a, b);
    }};

    fmt::print("\n Hello {}!\n", data.playername);
    fmt::print("\n Here are the CS:GO global statistics:\n\n");

    printAligned("Players Online", std::to_string(data.global_stats.players_online));
    printAligned(" ");
    printAligned("Servers Online", std::to_string(data.global_stats.servers_online));
    printAligned("Servers Available", std::to_string(data.global_stats.servers_available));
    printAligned(" ");
    printAligned("Players Searching", std::to_string(data.global_stats.players_searching));
    printAligned("Ongoing Matches", std::to_string(data.global_stats.ongoing_matches));
    printAligned("Average Search Time", data.getAverageSearchTime());
}