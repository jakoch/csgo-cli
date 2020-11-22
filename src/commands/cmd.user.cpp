#include "cmd.user.h"
#include "cstrike15_gcmessages.pb.h"

bool requestPlayersProfile(DataObject &data, bool &verbose)
{
    if (verbose) spdlog::info("[ Start ] [ Thread ] getUserInfo");

    bool result = false;

    auto hellothread = std::thread([&data, verbose, &result]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

            CSGOMMHello mmhello;
            if (verbose) spdlog::info("          Requesting: Hello");
            mmhello.RefreshWait();
            if (verbose) spdlog::info("          Got Hello");

            result = true;

            if (verbose) { spdlog::debug("mmhello.data.DebugString {}", mmhello.data.DebugString()); }

            // player level
            data.player_level          = mmhello.data.player_level();
            data.player_cur_xp         = mmhello.data.player_cur_xp();
            data.player_xp_bonus_flags = mmhello.data.player_xp_bonus_flags();

            // medals
            /*if (mmhello.data.has_medals()) {
                data.medals_arms    = mmhello.data.medals().medal_arms();
                data.medals_combat  = mmhello.data.medals().medal_combat();
                data.medals_global  = mmhello.data.medals().medal_global();
                data.medals_team    = mmhello.data.medals().medal_team();
                data.medals_weapon  = mmhello.data.medals().medal_weapon();
            }*/
            // vac status
            data.vac_banned      = mmhello.data.vac_banned();
            data.penalty_seconds = mmhello.data.penalty_seconds();
            data.penalty_reason  = mmhello.data.penalty_reason();

            // ranks
            if (mmhello.data.has_ranking()) {
                spdlog::debug("mmhello.data.rankings() (MatchMaking) {}", mmhello.data.ranking().DebugString());
                DataObject::RankingInfo ri;
                ri.id     = mmhello.data.ranking().rank_id();
                ri.type   = mmhello.data.ranking().rank_type_id();
                ri.wins   = mmhello.data.ranking().wins();
                ri.change = mmhello.data.ranking().rank_change();
                data.rankings.push_back(ri);
            }
            // commendations
            if (mmhello.data.has_commendation()) {
                data.cmd_friendly = mmhello.data.commendation().cmd_friendly();
                data.cmd_teaching = mmhello.data.commendation().cmd_teaching();
                data.cmd_leader   = mmhello.data.commendation().cmd_leader();
            }
        } catch (CSGO_CLI_TimeoutException) {
            printError("Warning", "Timeout on receiving UserInfo.");
            result = false;
        } catch (ExceptionHandler &e) {
            printError("Fatal error", e.what());
            result = false;
        }
        if (verbose) spdlog::info("[ End   ] [ Thread ] getUserInfo");
        return 0;
    });

    hellothread.join();

    return result;
}

bool requestPlayersRankInfo(DataObject &data, bool &verbose)
{
    if (verbose) spdlog::info("[ Start ] [ Thread ] requestPlayersRankInfo");

    bool result = false;

    auto rankUpdateThread = std::thread([&data, verbose, &result]() {
        try {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            CSGORankUpdate rankUpdate;

            if (verbose) spdlog::info("          Requesting: rankUpdate for Wingman");
            rankUpdate.RefreshWaitWingmanRank();
            if (verbose) spdlog::info("          Got rankUpdate for Wingman");

            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if (verbose) spdlog::info("          Requesting: rankUpdate for DangerZone");
            rankUpdate.RefreshWaitDangerZoneRank();
            if (verbose) spdlog::info("          Got rankUpdate for DangerZone");

            result = true;

            if (verbose) {
                spdlog::debug("rankUpdate.data[0] (Wingman) {}", rankUpdate.data[0].DebugString());
                spdlog::debug("rankUpdate.data[1] (DangerZone) {}", rankUpdate.data[1].DebugString());
            }

            DataObject::RankingInfo ri;

            // wingman
            PlayerRankingInfo wm_pri = rankUpdate.data[0].rankings().Get(0);
            ri.id                    = wm_pri.rank_id();
            ri.type                  = wm_pri.rank_type_id();
            ri.wins                  = wm_pri.wins();
            ri.change                = wm_pri.rank_change();
            data.rankings.push_back(ri);

            ri = {}; // reset

            // dangerzone
            PlayerRankingInfo dz_pri = rankUpdate.data[1].rankings().Get(0);
            ri.id                    = dz_pri.rank_id();
            ri.type                  = dz_pri.rank_type_id();
            ri.wins                  = dz_pri.wins();
            ri.change                = dz_pri.rank_change();
            data.rankings.push_back(ri);

        } catch (CSGO_CLI_TimeoutException) {
            printError("Warning", "Timeout on receiving RankUpdate.");
            result = false;
        } catch (ExceptionHandler &e) {
            printError("Fatal error", e.what());
            result = false;
        }
        if (verbose) spdlog::info("[ End   ] [ Thread ] rankUpdate");
        return 0;
    });

    rankUpdateThread.join();

    return result;
}

void printPlayersProfile(DataObject &data)
{
    // ---------- Format Output Strings

    std::string level = fmt::format(
        "{0} ({1}/40) (XP: {2}/5000 | {3:.2f}%)",
        data.getPlayerLevel(),
        data.player_level,
        data.getPlayerXp(),
        data.getPlayerXpPercentage());

    std::string likes =
        fmt::format("{} x friendly, {} x teaching, {} x leader", data.cmd_friendly, data.cmd_teaching, data.cmd_leader);

    std::string penalty = fmt::format("{} ({} Minutes)", data.penalty_reason, (data.penalty_seconds / 60));

    std::string clan = fmt::format("{} \"{}\"", data.clan_name, data.clan_tag);

    auto mm_ranks     = data.rankings[0];
    auto mm_rank_name = data.getRankName(mm_ranks.id);

    std::string matchmaking_rank = fmt::format("{} ({}/18) ({} wins)", mm_rank_name, mm_ranks.id, mm_ranks.wins);

    auto wm_ranks     = data.rankings[1];
    auto wm_rank_name = data.getRankName(wm_ranks.id);

    std::string wingman_rank = fmt::format("{} ({}/18) ({} wins)", wm_rank_name, wm_ranks.id, wm_ranks.wins);

    auto dz_ranks     = data.rankings[2];
    auto dz_rank_name = data.getRankName(dz_ranks.id);

    std::string dangerzone_rank = fmt::format("{} ({}/18) ({} wins)", dz_rank_name, dz_ranks.id, dz_ranks.wins);

    // TODO how to access medals data?
    // auto medals = fmt::format("{} x arms, {} x combat, {} x global, {} x team, {} x weapon",
    //    data.medals_arms, data.medals_combat, data.medals_global, data.medals_team, data.medals_weapon);

    // ---------- Output Table

    const auto printAligned{[=](const std::string &a, const std::string &b = "") {
        return fmt::print(" {0:<18} {1}\n", a, b);
    }};

    fmt::print("\n Hello {}!\n", data.playername);
    fmt::print("\n Here is your user profile:\n\n");

    printAligned("[Steam]");
    printAligned(" ");
    printAligned("Name:", data.playername);
    printAligned("Clan:", clan);
    printAligned("ID:", toSteamIDClassic(data.steam_id));
    printAligned("ID32:", toSteamID32(data.steam_id));
    printAligned("ID64:", std::to_string(data.steam_id));
    printAligned("Player Level:", std::to_string(data.steam_player_level));
    printAligned("VAC Status:", data.getVacStatus());
    printAligned("Profile URL:", data.getSteamProfileUrl());
    printAligned(" ");
    printAligned("[CS:GO]");
    printAligned(" ");
    printAligned("MatchMaking Rank:", matchmaking_rank);
    printAligned("Wingman Rank:", wingman_rank);
    printAligned("DangerZone Rank:", dangerzone_rank);
    printAligned("Player Level:", level);
    printAligned("Likes:", likes);
    printAligned("Penalty:", penalty);
    printAligned("Overwatch:", data.getCanDoOverwatch());
    // printAligned("Medals:", medals);
}