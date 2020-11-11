#include "cmd.scoreboard.h"

void printScoreboard(DataObject &data)
{
    if (!data.has_matches_played) {
        fmt::print("\n Your CS:GO match history is empty.\n");
        return;
    }

    // ---------- Output Table

    //{0:^3} {1:<20} {2:^8} {3:^5} {4:<9} {5:<7}

    const auto printRow {[=](const std::string &s1, const std::string &s2, const std::string &s3,
    const std::string &s4, const std::string &s5, const std::string &s6, const std::string &s7, const std::string &s8, const std::string &s9, const std::string &s10, const std::string &s11) {
      return fmt::print(" {0} {1} {2} {3} {4} {5} {6} {7} {8} {9} {10} \n", s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11);
    }};

    fmt::print("\n Hello {}!\n", data.playername);
    fmt::print("\n Here is your scoreboard:\n\n");

    printRow("Match Played", "Result", "Score", "K", "A", "D", "HS(%)", "K/D", "Rating", "MVP", "Score");

    for (const auto &match : data.matches)
    {
        for (const auto &player : match.scoreboard)
        {
            if (player.account_id == data.account_id)
            {
                //fmt::print("AcountID-API: {}\n", data.account_id);
                //fmt::print("AcountID-Match: {}\n", player.account_id);

                printRow(
                    match.matchtime_str,
                    match.result_str,
                    match.getScore(),
                    player.getKills(),
                    player.getAssists(),
                    player.getDeaths(),
                    "hs", //player.getHSRatio(),
                    player.getKillDeathRatio(),
                    "rating",
                    player.getMVPs(),
                    player.getScore()
                );
            }
        }
    }
}