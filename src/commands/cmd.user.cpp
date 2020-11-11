#include "cmd.user.h"

bool requestPlayersProfile(DataObject &data, bool &verbose)
{
    if (verbose) spdlog::info("[ Start ] [ Thread ] getUserInfo");

    bool result = false;

    auto hellothread = std::thread([&data, verbose, &result]()
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

            CSGOMMHello mmhello;
            if (verbose) spdlog::info("          Requesting: Hello");
            mmhello.RefreshWait();
            if (verbose) spdlog::info("          Got Hello");

            result = true;

            if (verbose) spdlog::debug("mmhello.data.ShortDebugString: {}", mmhello.data.ShortDebugString());
            if (verbose) spdlog::debug("mmhello.data.DebugString {}", mmhello.data.DebugString());

            // player level
            data.player_level       = mmhello.data.player_level();
            data.player_cur_xp      = mmhello.data.player_cur_xp();
            // medals
            /*if (mmhello.data.has_medals()) {
                data.medals_arms    = mmhello.data.medals().medal_arms();
                data.medals_combat  = mmhello.data.medals().medal_combat();
                data.medals_global  = mmhello.data.medals().medal_global();
                data.medals_team    = mmhello.data.medals().medal_team();
                data.medals_weapon  = mmhello.data.medals().medal_weapon();
            }*/
            // vac status
            data.vac_banned         = mmhello.data.vac_banned();
            data.penalty_seconds    = mmhello.data.penalty_seconds();
            data.penalty_reason     = mmhello.data.penalty_reason();

            // ranks
            if (mmhello.data.has_ranking()) {
                data.rank_id        = mmhello.data.ranking().rank_id();
                data.rank_wins      = mmhello.data.ranking().wins();
                data.rank_change    = mmhello.data.ranking().rank_change();
            }
            // commendations
            if (mmhello.data.has_commendation()) {
                data.cmd_friendly   = mmhello.data.commendation().cmd_friendly();
                data.cmd_teaching   = mmhello.data.commendation().cmd_teaching();
                data.cmd_leader     = mmhello.data.commendation().cmd_leader();
            }
        }
        catch (CSGO_CLI_TimeoutException)
        {
            printError("Warning", "Timeout on receiving UserInfo.");
            result = false;
        }
        catch (ExceptionHandler& e)
        {
            printError("Fatal error", e.what());
            result = false;
        }
        if (verbose) spdlog::info("[ End   ] [ Thread ] getUserInfo");
        return 0;
    });

    hellothread.join();

    return result;
}

void printPlayersProfile(DataObject &data)
{
    // ---------- Format Output Strings

    std::string rank = fmt::format("{} ({}/18)", data.getPlayerRank(), data.rank_id);

    std::string level = fmt::format("{0} ({1}/40) (XP: {2}/5000 | {3:.2f}%)", data.getPlayerLevel(), data.player_level, data.getPlayerXp(), data.getPlayerXpPercentage());

    std::string likes = fmt::format("{} x friendly, {} x teaching, {} x leader", data.cmd_friendly, data.cmd_teaching, data.cmd_leader);

    std::string penalty = fmt::format("{} ({} Minutes)", data.penalty_reason, (data.penalty_seconds / 60));

    std::string clan = fmt::format("{} \"{}\"", data.clan_name,  data.clan_tag);

    // TODO how to access medals data?
    //auto medals = fmt::format("{} x arms, {} x combat, {} x global, {} x team, {} x weapon",
    //    data.medals_arms, data.medals_combat, data.medals_global, data.medals_team, data.medals_weapon);

    // ---------- Output Table

    const auto printAligned {[=](const std::string &a, const std::string &b = "") {
      return fmt::print(" {0:<18} {1}\n", a, b);
    }};

    fmt::print("\n Hello {}!\n", data.playername);
    fmt::print("\n Here is your user profile:\n\n");

    printAligned("[Steam]"                                                        );
    printAligned(" "                                                              );
    printAligned("Name:"                , data.playername                         );
    printAligned("Clan:"                , clan                                    );
    printAligned("ID:"                  , toSteamIDClassic(data.steam_id)         );
    printAligned("ID32:"                , toSteamID32(data.steam_id)              );
    printAligned("ID64:"                , std::to_string(data.steam_id)           );
    printAligned("Player Level:"        , std::to_string(data.steam_player_level) );
    printAligned("VAC Status:"          , data.getVacStatus()                     );
    printAligned("Profile URL:"         , data.getSteamProfileUrl()               );
    printAligned(" "                                                              );
    printAligned("[CS:GO]"                                                        );
    printAligned(" "                                                              );
    printAligned("Rank:"                , rank                                    );
    printAligned("MatchMaking Wins:"    , std::to_string(data.rank_wins)          );
    printAligned("Player Level:"        , level                                   );
    printAligned("Likes:"               , likes                                   );
    printAligned("Penalty:"             , penalty                                 );
    //printAligned("Medals:"              , medals                                  );
}