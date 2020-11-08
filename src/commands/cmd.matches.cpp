#include "cmd.matches.h"

bool requestRecentMatches(DataObject &data, bool &verbose)
{
    if (verbose) std::clog << "LOG:" << "[ Start ] [ Thread ] MatchList\n";

    bool result = false;

    auto matchthread = std::thread([&data, verbose, &result]()
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_MATCHLIST_DELAY));

            // refresh match list
            CSGOMatchList matchList;
            if (verbose) std::clog << "LOG:" << "requesting MatchList\n";
            matchList.RefreshWait();
            if (verbose) std::clog << "LOG:" << "got MatchList\n";
            result = true;

            if (verbose) std::clog << "LOG:" << "[ Start ] processing MatchList\n";

            // empty match history
            if (matchList.Matches().size() == 0) {
                data.has_matches_played = false;
            }
            else {
                data.has_matches_played = true;
                data.num_matches_played = matchList.Matches().size();

                for (auto &match : matchList.Matches())
                {
                    if (verbose) std::clog << "LOG:" << "[ Start ] processing Match\n";

                    CSGOMatchData parsedMatch;

                    // MATCHID
                    parsedMatch.matchid = match.matchid();

                    // MATCHTIME
                    parsedMatch.matchtime = match.matchtime();

                    // MATCHTIME_STR
                    parsedMatch.matchtime_str = getDateTime(parsedMatch.matchtime);

                    // EXTRACT REPLAY INFO
                    parsedMatch.server_ip = match.watchablematchinfo().server_ip();
                    parsedMatch.tv_port = match.watchablematchinfo().tv_port();

                    // iterate roundstats
                    CMsgGCCStrike15_v2_MatchmakingServerRoundStats roundStats;
                    for (int i = 0; i < match.roundstatsall().size(); ++i)
                    {
                        roundStats = match.roundstatsall(i);

                        // ROUNDSTATS per player
                        /*for (auto &account_id : roundStats.reservation().account_ids())
                        {
                            if (verbose) std::clog << "LOG:" << "[ Start ] MATCH-PLAYER:" << account_id << "\n";

                            CSGOMatchPlayerScore player;
                            player.index = matchList.getPlayerIndex(account_id, roundStats);
                            player.account_id = account_id;
                            player.steam_id = CSteamID(player.account_id, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64();
                            player.kills = roundStats.kills(player.index);
                            player.assists = roundStats.assists(player.index);
                            player.deaths = roundStats.deaths(player.index);
                            player.mvps = roundStats.mvps(player.index);
                            player.score = roundStats.scores(player.index);

                            parsedMatch.scoreboard.push_back(player);

                            if (verbose) std::clog << "LOG:" << "[ End   ] MATCH-PLAYER\n";
                        }*/

                        //std::cout << match.roundstatsall(i).DebugString() << "/n";
                    }

                    // RESERVATION ID (from last roundstats item)
                    parsedMatch.reservation_id = roundStats.reservationid();

                    // MATCH DURATION in secs
                    parsedMatch.match_duration = roundStats.match_duration();

                    // MATCH DURATION STRING min:sec
                    parsedMatch.match_duration_str = getDateTime(parsedMatch.match_duration, "%M:%Sm");

                    // map
                    parsedMatch.map = match.watchablematchinfo().game_map();
                    parsedMatch.mapgroup = match.watchablematchinfo().game_mapgroup();
                    parsedMatch.gametype = roundStats.reservation().game_type(); //match.watchablematchinfo().game_type();

                    //if (verbose) std::clog << "LOG:" << match.DebugString();

                    // link to replay
                    // roundStats.map() is the http link to the bz2 archived replay file
                    parsedMatch.replaylink = roundStats.map();

                    //if (verbose) std::clog << "LOG:" << roundStats.DebugString();

                    parsedMatch.sharecode = getShareCode(parsedMatch.matchid, parsedMatch.reservation_id, parsedMatch.tv_port);

                    if (matchList.getOwnIndex(roundStats) >= 5) {
                        parsedMatch.score_ally = roundStats.team_scores(1);
                        parsedMatch.score_enemy = roundStats.team_scores(0);
                    }
                    else {
                        parsedMatch.score_ally = roundStats.team_scores(0);
                        parsedMatch.score_enemy = roundStats.team_scores(1);
                    }

                    parsedMatch.result = roundStats.match_result();
                    parsedMatch.result_str = matchList.getMatchResult(roundStats);

                    data.matches.push_back(parsedMatch);

                    if (verbose) std::clog << "LOG:" << "[ End   ] processing Match\n";
                }
            }
            if (verbose) std::clog << "LOG:" << "[ End   ] processing MatchList\n";

        }
        catch (CSGO_CLI_TimeoutException)
        {
            printError("Warning", "Timeout on receiving MatchList.");
            result = false;
        }
        catch (ExceptionHandler& e)
        {
            printError("Fatal Error", e.what());
            result = false;
        }
        if (verbose) std::clog << "LOG:" << "[ End   ] [ Thread ] MatchList\n";
        return 0;
    });

    matchthread.join();

    return result;
}


void printMatches(DataObject &data)
{
    char name[40];
    sprintf(name, "%ls", data.playername); // %ls format = wchar_t*

    fmt::print("\n Hello {}!\n\n", name);

    if (!data.has_matches_played) {
        fmt::print(" Your CS:GO match history is empty.\n");
        return;
    }

    if (data.num_matches_played == 1) {
        fmt::print(" Here is your latest match:\n");
    } else {
        fmt::print(" Here are your {} latest matches:\n", data.num_matches_played);
    }

    tabulate::Table t;
    //t.column(1).format().multi_byte_characters(true); // for name
    //t.column(0).format().font_align(FontAlign::left);
    //t.column(1).format().font_align(FontAlign::center);

    t.add_row(row_t{ "#", "Match Played", "Duration", "Map", "Score", "Result"});

    /*int i = 1;
    for (const auto &match : data.matches)
    {
        t += {
            std::to_string(i),
            // std::to_string(match.matchid),
            match.matchtime_str,
            match.match_duration_str,
            match.getMapname(),
            match.getScore(),
            match.getMatchResult()
            //"Replaylink:" match.replaylink,
            //"Match IP:"              << match.server_ip,
            //"Match Port:"            << match.tv_port,
            //"Match Reservation ID:"  << match.reservation_id,
            //"Replay ShareCode:"        << match.sharecode,
            //"Mapgroup:"              << match.mapgroup,
            //"Gametype:"              << match.gametype
        };
        ++i;
    }*/
    std::cout << t << std::endl;;
}