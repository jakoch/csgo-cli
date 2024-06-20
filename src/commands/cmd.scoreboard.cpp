// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmd.scoreboard.h"

void printScoreboard(DataObject& data)
{
    if (!data.has_matches_played) {
        fmt::print("\n Your CS:GO match history is empty.\n");
        return;
    }

    // ---------- Output Table

    //{0:^3} {1:<20} {2:^8} {3:^5} {4:<9} {5:<7}

    auto const printRow{[=](std::string const & s1,
                            std::string const & s2,
                            std::string const & s3,
                            std::string const & s4,
                            std::string const & s5,
                            std::string const & s6,
                            std::string const & s7,
                            std::string const & s8,
                            std::string const & s9,
                            std::string const & s10,
                            std::string const & s11) {
        return fmt::print(
            " {0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} \n", s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11);
    }};

    fmt::print("\n Hello {}!\n", data.playername);
    fmt::print("\n Here is your scoreboard:\n\n");

    printRow("Match Played", "Result", "Score", "K", "A", "D", "HS(%)", "K/D", "Rating", "MVP", "Score");

    for (auto const & match : data.matches) {
        for (auto const & player : match.scoreboard) {
            if (player.account_id == data.account_id) {
                fmt::print("{}\n", match.matchtime_str);
                /*printRow(
                    match.matchtime_str,
                    match.result_str,
                    match.getScore(),
                    player.getKills(),
                    player.getAssists(),
                    player.getDeaths(),
                    "hs", // player.getHSRatio(),
                    player.getKillDeathRatio(),
                    "rating",
                    player.getMVPs(),
                    player.getScore());*/
            }
        }
    }
}