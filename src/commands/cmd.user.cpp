#include "cmd.user.h"

bool requestPlayersProfile(DataObject &data, bool &verbose)
{
    if (verbose) std::clog << "LOG:" << "[ Start ] [ Thread ] getUserInfo\n";

    bool result = false;

    auto hellothread = std::thread([&data, verbose, &result]()
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

            CSGOMMHello mmhello;
            if (verbose) std::clog << "LOG:" << "          Requesting: Hello\n";
            mmhello.RefreshWait();
            if (verbose) std::clog << "LOG:" << "          Got Hello\n";

            result = true;

            if (verbose) std::clog << "DEBUG:" << mmhello.data.ShortDebugString();
            if (verbose) std::clog << "DEBUG:" << mmhello.data.DebugString();
            if (verbose) std::clog << "DEBUG:" << mmhello.data.medals().DebugString();

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
        if (verbose) std::clog << "LOG:" << "[ End   ] [ Thread ] getUserInfo\n";
        return 0;
    });

    hellothread.join();

    return result;
}

void printPlayersProfile(DataObject &data)
{
    // ---------- Format Output Strings

    char name[40];
    sprintf(name, "%ls", data.playername); // %ls format = wchar_t*

    const auto steam_profile_url = fmt::format("https://steamcommunity.com/profiles/{}", data.steam_id);

    const auto rank = fmt::format("{} ({}/18)", data.getPlayerRank(), data.rank_id);

    const auto level = fmt::format("{0} ({1}/40) (XP: {2}/5000 | {3:.2f}%)", data.getPlayerLevel(), data.player_level, data.getPlayerXp(), data.getPlayerXpPercentage());

    const auto likes = fmt::format("{} x friendly, {} x teaching, {} x leader", data.cmd_friendly, data.cmd_teaching, data.cmd_leader);

    const auto penalty = fmt::format("{} ({} Minutes)", data.penalty_reason, (data.penalty_seconds / 60));

    const auto clan = fmt::format("{} \"{}\"", data.clan_name,  data.clan_tag);

    // TODO how to access medals data? they were available, but are now gone...
    //auto medals = fmt::format("{} x arms, {} x combat, {} x global, {} x team, {} x weapon",
    //    data.medals_arms, data.medals_combat, data.medals_global, data.medals_team, data.medals_weapon);

    // ---------- Output Table

    fmt::print("\n Hello {}!\n\n Here is your user profile:\n", name);

    tabulate::Table t;
    t.column(1).format().multi_byte_characters(true); // for name
    t.column(0).format().font_align(FontAlign::left);
    t.column(1).format().font_align(FontAlign::center);

    t.add_row( row_t{" [Steam]"                                                         });
    t.add_row( row_t{""                                                                 });
    t.add_row( row_t{" Name:"               , name                                      });
    t.add_row( row_t{" Clan:"               , clan                                      });
    t.add_row( row_t{" ID:"                 , toSteamIDClassic(data.steam_id)           });
    t.add_row( row_t{" ID32:"               , toSteamID32(data.steam_id)                });
    t.add_row( row_t{" ID64:"               , std::to_string(data.steam_id)             });
    t.add_row( row_t{" Player Level:"       , std::to_string(data.steam_player_level)   });
    t.add_row( row_t{" VAC Status:"         , data.getVacStatus()                       });
    t.add_row( row_t{" Profile URL:"        , steam_profile_url                         });
    t.add_row( row_t{""                                                                 });
    t.add_row( row_t{" [CS:GO]"                                                         });
    t.add_row( row_t{""                                                                 });
    t.add_row( row_t{" Rank:"               , rank                                      });
    t.add_row( row_t{" MatchMaking Wins:"   , std::to_string(data.rank_wins)            });
    t.add_row( row_t{" Player Level:"       , level                                     });
    t.add_row( row_t{" Likes:"              , likes                                     });
    t.add_row( row_t{" Penalty:"            , penalty                                   });
    //t.add_row( row_t{" Medals:"             , medals                                  });

    std::cout << t << std::endl;
}