#include "cmd.scoreboard.h"

void printScoreboard(DataObject &data)
{
    if (!data.has_matches_played) {
        fmt::print("\n Your CS:GO match history is empty.\n");
        return;
    }

    fmt::print("\n Here is your scoreboard:\n");

    tabulate::Table t;
    t.add_row(row_t{ "Match Played", "Result", "Score", "K", "A", "D", "Headshot (%)", "K/D ratio (diff)", "Rating", "MVP", "Score" });

    for (const auto &match : data.matches)
    {
        for (const auto &player : match.scoreboard)
        {
            //std::cout << match.matchtime_str,

            if (player.account_id == data.account_id)
            {
                //std::cout << "AcountID-API:" << data.account_id << "\n";
                //std::cout << "AcountID-Match:" << player.account_id << "\n";

                //sprintf(headshot_string, "%d (%d%)", headshot, headshot_percentage = ((headshots / kills) * 100))

                /*t += {
                    match.matchtime_str,
                    match.result_str,
                    match.getScore(),
                    std::to_string(player.kills),
                    std::to_string(player.assists),
                    std::to_string(player.deaths),
                        // headshot_string
                        // k/d ratio (kill/death difference)
                    std::to_string(player.mvps),
                    std::to_string(player.score)
                };*/
            }
        }
    }

    std::cout << t << std::endl;
}