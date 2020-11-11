#include "cmd.matches.h"

bool requestRecentMatches(DataObject &data, bool &verbose)
{
    if (verbose) spdlog::info("[ Start ] [ Thread ] MatchList");

    bool result = false;

    auto matchthread = std::thread([&data, verbose, &result]()
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_MATCHLIST_DELAY));

            // refresh match list
            CSGOMatchList matchList;
            if (verbose) spdlog::info("          -> requesting MatchList");
            matchList.RefreshWait();
            if (verbose) spdlog::info("          -> got MatchList");
            result = true;

            if (verbose) spdlog::info("[ Start ] processing MatchList");

            // empty match history
            if (matchList.Matches().size() == 0) {
                data.has_matches_played = false;
            }
            else {
                data.has_matches_played = true;
                data.num_matches_played = matchList.Matches().size();

                int matches_num = 1;

                for (auto &match : matchList.Matches())
                {
                    if (verbose) {
                       spdlog::info("[ Start ] processing Match #{}", matches_num);
                      //spdlog::debug("match.DebugString /n {}", match.DebugString());
                    }

                    CSGOMatchData parsedMatch;
                    parsedMatch.matchid       = match.matchid();
                    parsedMatch.matchtime     = match.matchtime();
                    parsedMatch.matchtime_str = getDateTime(parsedMatch.matchtime);
                    // replay info
                    parsedMatch.server_ip     = match.watchablematchinfo().server_ip();
                    parsedMatch.tv_port       = match.watchablematchinfo().tv_port();
                    // map
                    parsedMatch.map           = match.watchablematchinfo().game_map();
                    parsedMatch.mapgroup      = match.watchablematchinfo().game_mapgroup();
                    parsedMatch.gametype      = match.watchablematchinfo().game_type();

                    // iterate roundstats
                    CMsgGCCStrike15_v2_MatchmakingServerRoundStats roundStats;
                    for (int i = 0; i < match.roundstatsall().size(); ++i)
                    {
                        roundStats = match.roundstatsall(i);

                        //if (verbose) spdlog::debug("roundStats.DebugString /n {}", roundStats.DebugString());

                        // ROUNDSTATS per player
                        for (auto &account_id : roundStats.reservation().account_ids())
                        {
                            if (verbose) spdlog::info("[ Start ] Match-Player: {}", account_id);

                            CSGOMatchPlayerScore player;
                            player.index      = matchList.getPlayerIndex(account_id, roundStats);
                            player.account_id = account_id;
                            player.steam_id   = CSteamID(player.account_id, k_EUniversePublic, k_EAccountTypeIndividual).ConvertToUint64();
                            player.kills      = roundStats.kills(player.index);
                            player.assists    = roundStats.assists(player.index);
                            player.deaths     = roundStats.deaths(player.index);
                            //player.kdr      = player.kills/player.deaths;
                            player.mvps       = roundStats.mvps(player.index);
                            player.score      = roundStats.scores(player.index);

                            parsedMatch.scoreboard.push_back(player);

                            if (verbose) spdlog::info("[ End   ] Match-Player");
                        }

                        std::cout << match.roundstatsall(i).DebugString() << "\n";
                    }

                    // RESERVATION ID (from last roundstats item)
                    parsedMatch.reservation_id     = roundStats.reservationid();
                    parsedMatch.match_duration     = roundStats.match_duration(); // seconds
                    parsedMatch.match_duration_str = getDateTime(parsedMatch.match_duration, "%M:%Sm");
                    parsedMatch.replaylink         = roundStats.map(); // http link to the bz2 archived replay file
                    parsedMatch.sharecode          = getShareCode(parsedMatch.matchid, parsedMatch.reservation_id, parsedMatch.tv_port);

                    if (matchList.getOwnIndex(roundStats) >= 5) {
                        parsedMatch.score_ally  = roundStats.team_scores(1);
                        parsedMatch.score_enemy = roundStats.team_scores(0);
                    }
                    else {
                        parsedMatch.score_ally  = roundStats.team_scores(0);
                        parsedMatch.score_enemy = roundStats.team_scores(1);
                    }

                    parsedMatch.result     = roundStats.match_result();
                    parsedMatch.result_str = matchList.getMatchResult(roundStats);

                    data.matches.push_back(parsedMatch);

                    if (verbose) spdlog::info("[ End   ] processing Match #{}", matches_num);
                    matches_num++;
                }
            }
            if (verbose) spdlog::info("[ End   ] processing MatchList");
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
        if (verbose) spdlog::info("[ End   ] [ Thread ] MatchList");
        return 0;
    });

    matchthread.join();

    return result;
}


void printMatches(DataObject &data)
{
    fmt::print("\n Hello {}!\n\n", data.playername);

    if (!data.has_matches_played) {
        fmt::print(" Your CS:GO match history is empty.\n");
        return;
    }

    if (data.num_matches_played == 1) {
        fmt::print(" Here is your latest match:\n");
    } else {
        fmt::print(" Here are your {} latest matches:\n\n", data.num_matches_played);
    }

    const auto printRow {[=](const std::string &s1, const std::string &s2, const std::string &s3, const std::string &s4, const std::string &s5, const std::string &s6) {
      return fmt::print("{0:^3} {1:<20} {2:^8} {3:^5} {4:<9} {5:<7} \n", s1, s2, s3, s4, s5, s6);
    }};

    printRow("#", "Match Played", "Duration", "Map", "Score", "Result\n");

    int i = 1;
    for (const auto &match : data.matches)
    {
      printRow(
          std::to_string(i),
          match.matchtime_str,
          match.match_duration_str,
          match.getMapname(),
          match.getScore(),
          match.getMatchResult()
      );
      ++i;
    }

    // std::to_string(match.matchid),
    //"Replaylink:" match.replaylink,
    //"Match IP:"              << match.server_ip,
    //"Match Port:"            << match.tv_port,
    //"Match Reservation ID:"  << match.reservation_id,
    //"Replay ShareCode:"        << match.sharecode,
    //"Mapgroup:"              << match.mapgroup,
    //"Gametype:"              << match.gametype
}