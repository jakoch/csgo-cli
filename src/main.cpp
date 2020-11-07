#include <steam/steamtypes.h>
#include <fmt/format.h>
#include <fmt/color.h>

#include "VersionAndConstants.h"
#include "ExceptionHandler.h"
#include "WinCliColors.h"
#include "CSGOMMHello.h"
#include "CSGOMatchData.h"
#include "CSGOMatchList.h"
#include "CSGOMatchPlayerScore.h"
#include "ConsoleTable.h"
#include "DataObject.h"
#include "SteamId.h"
#include "ShareCode.h"
#include "ShareCodeUpload.h"
#include "ShareCodeCache.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

#include <thread>
#include <iostream>
#include <ostream>
#include <sstream>
#include <iomanip>
// Includes needed for _setmode() (+io.h)
#include <fcntl.h>

using namespace WinCliColors;

struct TableFormat {
    int width;
    TableFormat() : width(20) {}
    template<typename T>
    TableFormat& operator<<(const T& data) {
        std::cout << std::left << std::setw(width) << data;
        return *this;
    }
    TableFormat& operator<<(std::ostream&(*out)(std::ostream&)) {
        std::cout << out;
        return *this;
    }
};

void printError(const char *title, const char *text)
{
    fprintf(stdout, "\x1B[91m%s:\033[0m %s\n", title, text);
}

std::string getYear()
{
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y");
    return ss.str();
}

std::string getDateTime(const time_t &time, const char *time_format = "%Y-%m-%d %H:%M:%S")
{
    std::stringstream ss;
    ss << std::put_time(localtime(&time), time_format);
    return ss.str();
}

void printHelp()
{
  std::cout
      << formatLightGreen(CSGO_CLI_BINARYNAME) << " v" << formatYellow(CSGO_CLI_VERSION) << ", "
      << CSGO_CLI_WEBSITE << "\n"
      << "Copyright (c) 2018-" << getYear() << " Jens A. Koch.\n"
      << "\n"
      << " CS:GO Console shows your user account, stats and latest matches.\n"
      << " You can also use the tool to upload replay sharecodes to "
         "csgostats.gg.\n"
      << "\n"
      << "Usage:\n"
      << "  command [options] [arguments]\n"
      << "\n"
      << "Available commands:\n"
      << "  -user         "
      << "  Show your profile (SteamID, AccountID, MM-Rank, Likes, "
         "VAC-Status)\n"
      << "  -matches      "
      << "  Show your past matches in table form\n"
      << "  -upload       "
      << "  Upload your past matches to csgostats.gg\n"
      << "  -s, sharecode "
      << "  Upload a replay sharecode to csgostats.gg\n"
      //<< "  -scoreboard " << "  Show your past matches in scoreboard form\n"
      << "\n"
      << "  -V, Version   "
      << "  Display application version\n"
      << "  -h, help      "
      << "  Display this help message\n"
      << "\n"
      << "Options:\n"
      << "  -v, verbose   "
      << "  Increase verbosity of messages\n"
      << "\n";
}

const wchar_t* toWChar(const char *c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}

void initSteamAPI(bool &verbose)
{
    if (verbose) std::clog << "LOG:" << "[ Start ] STEAM_INIT\n";

    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)) {
        exit(1);
    }

#ifdef _WIN32
    int savedStderr;
    if (!verbose) {
        savedStderr = _dup(_fileno(stderr));
        freopen("NUL", "w", stderr);
    }
#endif

    if (!SteamAPI_Init())
    {
        printError("Fatal Error", "Steam not running. SteamAPI_Init() failed.\nPlease run Steam.");
        exit(1);
    }

#ifdef _WIN32
    if (!verbose) {
        fflush(stderr);
        _dup2(savedStderr, _fileno(stderr));
        _close(savedStderr);
        clearerr(stderr);
    }
#endif

    if (!SteamUser()->BLoggedOn())
    {
        printError("Fatal Error", "Steam user not logged in. SteamUser()->BLoggedOn() returned false.\nPlease log in.");
        exit(1);
    }

    // TODO
    // setPersonaState(Invisible) 7

    if (verbose) std::clog << "LOG:" << "[ End   ] STEAM_INIT\n";
}

std::thread createCallbackThread(bool &running, bool &verbose)
{
    if (verbose) std::clog << "LOG:" << "[ Start ] CallbackThread & Steam_RunCallbacks\n";
    auto CallbackThread = std::thread([&running]()
    {
        while (running)
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_CALLBACK_INTERVAL));
                SteamAPI_RunCallbacks();
            }
            catch (ExceptionHandler& e)
            {
                printError("Fatal Error", e.what());
                exit(1);
            }
        };
    });
    if (verbose) std::clog << "LOG:" << "[ End   ] CallbackThread & Steam_RunCallbacks\n";
    return CallbackThread;
}

void initGameClientConnection(DataObject &data, bool &verbose)
{
    if (verbose) std::clog << "LOG:" << "[ Start ] Trying to establish a GameClient Connection\n";
    bool result = false;
    try
    {
        // make sure we are connected to the GameClient
        if (verbose) std::clog << "LOG:" << "          Requesting: GameClient Connection\n";
        CSGOClient::GetInstance()->WaitForGameClientConnect();
        if (verbose) std::clog << "LOG:" << "          Successful: GameClient connected!\n";
        result = true;

        data.account_id         = SteamUser()->GetSteamID().GetAccountID();
        data.steam_id           = SteamUser()->GetSteamID().ConvertToUint64();
        data.steam_player_level = SteamUser()->GetPlayerSteamLevel();
        data.playername         = toWChar(SteamFriends()->GetPersonaName());

        CSteamID clan_id        = SteamFriends()->GetClanByIndex(0);
        data.clan_name          = SteamFriends()->GetClanName(clan_id);
        data.clan_tag           = SteamFriends()->GetClanTag(clan_id);
    }
    catch (ExceptionHandler& e)
    {
        printError("Fatal error", e.what());
        result = false;
    }

    if (!result)
    {
        printError("Fatal error", "GameClient could not connect.");
        exit(1);
    }
    if (verbose) std::clog << "LOG:" << "[ End   ] Trying to establish a GameClient Connection\n";
}

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

void exitIfGameIsRunning()
{
#ifdef _WIN32
    HWND test = FindWindowW(0, L"Counter-Strike: Global Offensive");
    if (test != NULL)
    {
        printError("Warning", "\nCS:GO is currently running.\nPlease close the game, before running this program.");
        exit(1);
    }
#endif
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

    fmt::print("\n Hello {}!\n\n", name);

    TableFormat t;
    t << " Here is your user profile:"                              << std::endl;
    t                                                               << std::endl;
    t << " [Steam]"                                                 << std::endl;
    t                                                               << std::endl;
    t << " Name:"               << name                             << std::endl;
    t << " Clan:"               << clan                             << std::endl;
    t << " ID:"                 << toSteamIDClassic(data.steam_id)  << std::endl;
    t << " ID32:"               << toSteamID32(data.steam_id)       << std::endl;
    t << " ID64:"               << data.steam_id                    << std::endl;
    t << " Player Level:"       << data.steam_player_level          << std::endl;
    t << " VAC Status:"         << data.getVacStatus()              << std::endl;
    t << " Profile URL:"        << steam_profile_url                << std::endl;
    t                                                               << std::endl;
    t << " [CS:GO]"                                                 << std::endl;
    t                                                               << std::endl;
    t << " Rank:"               << rank                             << std::endl;
    t << " MatchMaking Wins:"   << data.rank_wins                   << std::endl;
    t << " Player Level:"       << level                            << std::endl;
    t << " Likes:"              << likes                            << std::endl;
    //t << " Medals:"             << medals                           << std::endl;
    t << " Penalty:"            << penalty                          << std::endl;
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

    ConsoleTable t{ "#", "Match Played", "Duration", "Map", "Score", "Result"};
    t.setPadding(1);
    t.setStyle(3);

    int i = 1;
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
    }
    std::cout << t;
}

void printScoreboard(DataObject &data)
{
    if (!data.has_matches_played) {
        fmt::print("\n Your CS:GO match history is empty.\n");
        return;
    }

    fmt::print("\n Here is your scoreboard:\n");

    ConsoleTable t{ "Match Played", "Result", "Score", "K", "A", "D", "Headshot (%)", "K/D ratio (diff)", "Rating", "MVP", "Score" };
    t.setPadding(1);
    t.setStyle(0);

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

                t += {
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
                };
            }
        }
    }

    std::cout << t << std::endl;
}

static inline void uploadShareCode(std::string &sharecode, ShareCodeCache *matchCache, ShareCodeUpload *codeUpload)
{
  if (matchCache->find(sharecode)) {
        auto msg1 = fmt::format(fmt::fg(fmt::color::indian_red), "Skipped.");
        auto msg2 = fmt::format(fmt::fg(fmt::color::green), "The ShareCode \"{}\" was already uploaded.", sharecode);
        fmt::print(" {} {}\n", msg1, msg2);
        return;
    }

    std::string jsonResponse;

    fmt::print(" Uploading ShareCode: {}\n", formatTerminalYellow(sharecode));

    if (codeUpload->uploadShareCode(sharecode, jsonResponse) == 0)
    {
        int upload_status = codeUpload->processJsonResponse(jsonResponse);

        if (upload_status == 4 || upload_status == 5) { // in-progress || complete
            matchCache->insert(sharecode);
        }
        else if (upload_status <= 3)
        {
            printError("Error", "Could not parse the response (to the replay sharecode POST request).");
        }

    }
    else {
        printError("Error", "Could not POST replay sharecode.");
    }
}

void uploadReplayShareCodes(DataObject &data, bool &verbose)
{
  if (!data.has_matches_played) {
        printRed(" No replay sharecodes to upload.\n");
        return;
    }

    fmt::print("\n Uploading Replay ShareCode{} to https://csgostats.gg/: \n\n",
        (data.num_matches_played == 1) ? "" : "s"
    );

    ShareCodeCache *matchCache = new ShareCodeCache(verbose);
    ShareCodeUpload *codeUpload = new ShareCodeUpload(verbose);

    for (auto &match : data.matches)
    {
        uploadShareCode(match.sharecode, matchCache, codeUpload);
    }
}

void uploadSingleShareCode(std::string &sharecode, bool &verbose)
{
    printTerminalYellow("\n Uploading Single Replay ShareCode to https://csgostats.gg/: \n\n");

    ShareCodeCache *matchCache = new ShareCodeCache(verbose);
    ShareCodeUpload *codeUpload = new ShareCodeUpload(verbose);

    uploadShareCode(sharecode, matchCache, codeUpload);
}

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);
    WinCliColors::enableConsoleColor(true);

    int result = 0;

    bool paramVerbose = false;
    bool paramPrintUser = false;
    bool paramPrintMatches = false;
    bool paramPrintScoreboard = false;
    bool paramUploadShareCodes = false;
    bool paramUploadShareCode = false;
    std::string shareCode;

    // default action
    if (argc <= 1)
    {
        printHelp();
        return 0;
    }

    for (int i = 1; i < argc; i = i + 1)
    {
        std::string option = argv[i];
        if (option == "-h" || option == "--h" || option == "-help" || option == "/?") {
            printHelp();
            return 0;
        }
        else if (option == "-V" || option == "--V" || option == "-version") {
            fmt::print("{} version {}\n", formatLightGreen(CSGO_CLI_BINARYNAME), formatYellow(CSGO_CLI_VERSION));
            return 0;
        }
        else if (option == "-v" || option == "--v" || option == "-verbose") {
            paramVerbose = true;
        }
        else if (option == "-matches") {
            paramPrintMatches = true;
        }
        else if (option == "-scoreboard") {
            paramPrintScoreboard = true;
        }
        else if (option == "-user") {
            paramPrintUser = true;
        }
        else if (option == "-upload") {
            paramPrintMatches = true;
            paramUploadShareCodes = true;
        }
        else if (option == "-sharecode" || option == "-s") {
            paramUploadShareCode = true;
            shareCode = argv[i + 1];
            i++;
        } else if (option != "") {
            printError("ERROR (invalid argument)", option.c_str());
            fmt::print("Please check: '{} -help'\n", CSGO_CLI_BINARYNAME);
            return 1;
        }
    }

    if (paramVerbose && !paramPrintUser && !paramPrintMatches && !paramPrintScoreboard && !paramUploadShareCode && !paramUploadShareCodes) {
        printError("ERROR", "You are using (-v|-verbose) without any other command.");
        fmt::print("Please check: '{} -help'\n", CSGO_CLI_BINARYNAME);
        return 1;
    }

    // HANDLE UPLOADING OF SINGLE SHARECODE (no need to connect to STEAM_API)

    if (paramUploadShareCode) {
        uploadSingleShareCode(shareCode, paramVerbose);
        return 0;
    }

    // CONNECT TO STEAM_API

    exitIfGameIsRunning();

    initSteamAPI(paramVerbose);

    bool running = true;

    std::thread CallbackThread = createCallbackThread(running, paramVerbose);

    DataObject data;

    initGameClientConnection(data, paramVerbose);

    // GET DATA

    if (paramPrintUser) {
        if (!requestPlayersProfile(data, paramVerbose)) {
            printError("Error", "Steam did not respond in time. Could not print -user.");
            exit(1);
        }
    }

    if (paramPrintMatches || paramPrintScoreboard || paramUploadShareCodes) {
        if (!requestRecentMatches(data, paramVerbose)) {
            printError("Error", "Steam did not respond in time.");
            exit(1);
        }
    }

    // OUTPUT

    if (paramPrintUser) {
        printPlayersProfile(data);
    }

    if (paramPrintMatches) {
        printMatches(data);
    }

    if (paramPrintScoreboard) {
        printScoreboard(data);
    }

    if (paramUploadShareCodes) {
        uploadReplayShareCodes(data, paramVerbose);
    }

    // SHUTDOWN

    running = false;
    CallbackThread.join();
    CSGOClient::Destroy();
    SteamAPI_Shutdown();

    return result;
}
