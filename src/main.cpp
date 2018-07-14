#include "VersionAndConstants.h"
#include "ExceptionHandler.h"
#include <thread>
#include "CSGOMatchList.h"
#include "CSGOMMHello.h"
#include "CSGOMatchPlayerScore.h"
#include "CSGOMatchData.h"
#include "CSGOMMLinkObject.h"
#ifdef _WIN32
#include <io.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
// Includes needed for _setmode() (+io.h)
#include <fcntl.h>
#include <steamtypes.h>
#include "ShareCodeUpload.h"
#include "SteamId.h"
#include "ShareCode.h"

void Error(const char* title, const char* text)
{
    printf("%s: %s\n", title, text);
}

void PrintHelp()
{
	std::cout << "" << CSGO_CLI_BINARYNAME << " - v" << CSGO_CLI_VERSION << "/n"
		<< "Copyright (c) 2018 Jens A. Koch.\n"
		<< "\n"
		<< " CS:GO Console shows your user account, stats and latest matches.\n"
		<< " You can also use the tool to upload demo sharecodes to csgostats.gg.\n"
		<< "\n"
		<< "Usage:\n"
		<< "  command [options] [arguments]\n"
		<< "\n"
		<< "Available commands:\n"
		<< "  -user       " << "Show your profile (SteamID, AccountID, MM-Rank, Likes, VAC-Status)\n"
		<< "  -matches    " << "Show your past matches in table form\n"
		<< "  -upload     " << "Show and upload your past matches to csgostats.gg\n"
        //<< "  -perf       " << "Show your past matches performance in compact form\n"
        << "\n"
        << "Options:\n"
        << "  -h, help    " << " Display this help message\n"
        << "  -v, verbose " << " Increase verbosity of messages\n"
        << "  -V, Version " << " Display application version\n"
        << "\n"
    << std::endl;
}

const wchar_t* toWChar(const char *c)
{
    const size_t cSize = strlen(c) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, c, cSize);

    return wc;
}

struct stop_now_t { };

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);

    int result = 0;

    #ifdef _WIN32
    HWND test = FindWindowW(0, L"Counter-Strike: Global Offensive");
    if(test != NULL)
    {
        Error("Warning", "CS:GO is currently running.\nPlease close the game, before running this program.\n");
        return 1;
    }
#endif

    bool paramVerbose      = false;
    bool paramPrintUser    = false;
    bool paramPrintMatches = false;
    bool paramPrintPerf    = false;
    bool paramUpload       = false;

    CSGOMMLinkObject linkObj;

    const std::vector<std::string> ranks = {
        "-unranked-",
        "Silver 1",
        "Silver 2",
        "Silver 3",
        "Silver 4",
        "Silver Elite",
        "Silver Elite Master"
        "Gold Nova 1",
        "Gold Nova 2",
        "Gold Nova 3",
        "Gold Nova 4",
        "Master Guardian 1",              // single AK
        "Master Guardian 2",              // AK with wings
        "Master Guardian Elite",          // double AK
        "Distinguished Master Guardian",  // star
        "Legendary Eagle",                // eagle
        "Legendary Eagle Master",         // eagle with wings
        "Supreme Master First Class",     // small globe
        "Global Elite",                   // big globe
    };

    const std::vector<std::string> levels =
    {
        "Not Recruited",
        "Recruit",
        "Private",
        "Private",
        "Private",
        "Corporal",
        "Corporal",
        "Corporal",
        "Corporal",
        "Sergeant",
        "Sergeant",
        "Sergeant",
        "Sergeant",
        "Master Sergeant",
        "Master Sergeant",
        "Master Sergeant",
        "Master Sergeant",
        "Sergeant Major",
        "Sergeant Major",
        "Sergeant Major",
        "Sergeant Major",
        "Lieutenant",
        "Lieutenant",
        "Lieutenant",
        "Lieutenant",
        "Captain",
        "Captain",
        "Captain",
        "Captain",
        "Major",
        "Major",
        "Major",
        "Major",
        "Colonel",
        "Colonel",
        "Colonel",
        "Brigadier General",
        "Major General",
        "Lieutenant General",
        "General",
        "Global General"
    };

    // default action
    if(argc<=1)
    {
        PrintHelp();
        return 0;
    }

    for(int i=1; i < argc; i = i + 1)
    {
        std::string option = argv[i];
        if(option == "-h" || option == "--h" || option == "-help" || option == "/?"){
            PrintHelp();
            return 0;
        } else if(option == "-V" || option == "--V" || option == "-version"){
            std::cout << "" << CSGO_CLI_BINARYNAME << " version " << CSGO_CLI_VERSION << "/n";
            return 0;
        } else if(option == "-v" || option == "--v" || option == "-verbose"){
            paramVerbose = true;
        } else if(option == "-matches"){
			paramPrintMatches = true;
        } else if(option == "-perf"){
            paramPrintPerf = true;
        } else if (option == "-user") {
            paramPrintUser = true;
        } else if (option == "-upload") {
			paramPrintMatches = true;
            paramUpload = true;
        } else if(option != ""){
            std::cerr << "ERROR (invalid argument): " << option << '/n';
			std::cerr << "Check '" << CSGO_CLI_BINARYNAME << " -help/n/n";
			//PrintHelp();
            return 1;
        }
    }

    if(paramVerbose) std::clog << "LOG:" << "[ Start ] STEAM_INIT\n";

    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid)) {
        return 1;
    }

#ifdef _WIN32
    int savedStderr;
    if(!paramVerbose) {
        savedStderr =  _dup(_fileno(stderr));
        freopen("NUL", "w", stderr);
    }
#endif

    if (!SteamAPI_Init())
    {
        Error("Fatal Error", "Steam not running. SteamAPI_Init() failed.\nPlease run Steam.\n");
        return 1;
    }

#ifdef _WIN32
    if(!paramVerbose) {
        fflush(stderr);
        _dup2(savedStderr, _fileno(stderr));
        _close(savedStderr);
        clearerr(stderr);
    }
#endif

    if (!SteamUser()->BLoggedOn())
    {
        Error("Fatal Error", "Steam user not logged in. SteamUser()->BLoggedOn() returned false.\nPlease log in.\n");
        return 1;
    }

    if(paramVerbose) std::clog << "LOG:" << "[ End   ] STEAM_INIT\n";

    if(paramVerbose) std::clog << "LOG:" << "[ Start ] CallbackThread & Steam_RunCallbacks\n";
    bool running = true;
    auto CallbackThread = std::thread([&running]()
    {
        while (running)
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_CALLBACK_INTERVAL));
                Steam_RunCallbacks(GetHSteamPipe(), false);
            }
            catch (ExceptionHandler& e)
            {
                Error("Fatal Error", e.what());
                exit(1);
            }
        };
    });
    if(paramVerbose) std::clog << "LOG:" << "[ End   ] CallbackThread & Steam_RunCallbacks\n";

    if(paramVerbose) std::clog << "LOG:" << "[ Start ] Trying-GameClient-Connect\n";
    bool resultGameClientConnection = false;
    try
    {
        // make sure we are connected to the GameClient
        if(paramVerbose) std::clog << "LOG:" << "Requesting: GameClient Connection\n";
        CSGOClient::GetInstance()->WaitForGcConnect();
        if(paramVerbose) std::clog << "LOG:" << "Successful: GameClient connected!\n";
        resultGameClientConnection = true;

        linkObj.account_id  = SteamUser()->GetSteamID().GetAccountID();
        linkObj.steam_id    = SteamUser()->GetSteamID().ConvertToUint64();
        linkObj.playername  = toWChar(SteamFriends()->GetPersonaName());
    }
    catch (ExceptionHandler& e)
    {
        Error("Fatal error", e.what());
        resultGameClientConnection = false;
    } 

    if(!resultGameClientConnection)
    {
        Error("Fatal error", "GameClient could not connect.\n");
        return 1;
    }
    if (paramVerbose) std::clog << "LOG:" << "[ End   ] Trying-GameClient-Connect\n";

    bool resultClientHello = false;
    if (paramPrintUser)
    {
        if (paramVerbose) std::clog << "LOG:" << "[ Start ] [ Thread ] Hello\n";

        auto hellothread = std::thread([&linkObj, paramVerbose, &resultClientHello, ranks, levels]()
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

                CSGOMMHello mmhello;
                if (paramVerbose) std::clog << "LOG:" << "requesting MMHello\n";
                mmhello.RefreshWait();
                if (paramVerbose) std::clog << "LOG:" << "got MMHello\n";

                resultClientHello = true;
                //if (paramVerbose) std::clog << "DEBUG:" << mmhello.exposedProt.DebugString();
                                
                linkObj.player_level      = mmhello.data.player_level();
                linkObj.player_level_str  = levels[mmhello.data.player_level() - 1];
                linkObj.player_cur_xp     = mmhello.data.player_cur_xp();

                linkObj.medals_arms       = mmhello.data.medals().medal_arms();
                linkObj.medals_combat     = mmhello.data.medals().medal_combat();
                linkObj.medals_global     = mmhello.data.medals().medal_global();
                linkObj.medals_team       = mmhello.data.medals().medal_team();
                linkObj.medals_weapon     = mmhello.data.medals().medal_weapon();

                linkObj.vac_banned        = mmhello.data.vac_banned();
                linkObj.penalty_seconds   = mmhello.data.penalty_seconds();
                linkObj.penalty_reason    = mmhello.data.penalty_reason();

                if (mmhello.data.ranking().has_rank_id())
                {
                    linkObj.rank_id   = mmhello.data.ranking().rank_id();
                    linkObj.rank_str  = ranks[mmhello.data.ranking().rank_id() - 1];
                }
                if (mmhello.data.ranking().has_wins())
                    linkObj.rank_wins = mmhello.data.ranking().wins();
                if (mmhello.data.ranking().has_rank_change())
                    linkObj.rank_change = mmhello.data.ranking().rank_change();
                if (mmhello.data.commendation().has_cmd_friendly())
                    linkObj.cmd_friendly = mmhello.data.commendation().cmd_friendly();
                if (mmhello.data.commendation().has_cmd_teaching())
                    linkObj.cmd_teaching = mmhello.data.commendation().cmd_teaching();
                if (mmhello.data.commendation().has_cmd_leader())
                    linkObj.cmd_leader = mmhello.data.commendation().cmd_leader();
            }
            catch (stop_now_t) {
                return 0;
            }
            catch (CSGO_CLI_TimeoutException)
            {
                Error("Warning", "Timeout on receiving CMsgGCCStrike15_v2_MatchmakingGC2ClientHello\n");
                resultClientHello = false;
            }
            catch (ExceptionHandler& e)
            {
                Error("Fatal error", e.what());
                resultClientHello = false;
            }
            if (paramVerbose) std::clog << "LOG:" << "[ End   ] [ Thread ] Hello\n";
            return 0;
        });

        hellothread.join();
    }

    bool resultMatchList = false;

    if (paramPrintMatches)
    {
        if (paramVerbose) std::clog << "LOG:" << "[ Start ] [ Thread ] MatchList\n";

        auto matchthread = std::thread([&linkObj, paramVerbose, &resultMatchList]()
        {
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_MATCHLIST_DELAY));
                // refresh match list
                CSGOMatchList matchList;
                if (paramVerbose) std::clog << "LOG:" << "requesting MatchList\n";
                matchList.RefreshWait();
                if (paramVerbose) std::clog << "LOG:" << "got MatchList\n";

                resultMatchList = true;

                if (paramVerbose) std::clog << "LOG:" << "[ Start ] processing MatchList\n";

                for (auto &match : matchList.Matches())
                {
                    if (paramVerbose) std::clog << "LOG:" << "[ Start ] processing Match\n";

                    CSGOMatchData parsedMatch;

                    // MATCHID
                    parsedMatch.matchid = match.matchid();

                    // MATCHTIME
                    parsedMatch.matchtime = match.matchtime();

                    // MATCHTIME_STR
                    char buffer_local[80];
                    strftime(buffer_local, 80, "%Y-%m-%d %H:%M:%S", localtime(&parsedMatch.matchtime));
                    parsedMatch.matchtime_str = buffer_local;

                    // EXTRACT REPLAY INFO
                    parsedMatch.server_ip = match.watchablematchinfo().server_ip();
                    parsedMatch.tv_port = match.watchablematchinfo().tv_port();

                    //std::cout << match.DebugString();
                    //std::cout << match.watchablematchinfo().DebugString();

                    // iterate roundstats
                    CMsgGCCStrike15_v2_MatchmakingServerRoundStats roundStats;
                    for (int i = 0; i < match.roundstatsall().size(); ++i) {
                        roundStats = match.roundstatsall(i);

                        // ROUNDSTATS per player
                        /*for (auto &account_id : roundStats.reservation().account_ids())
                        {
                            if (paramVerbose) std::clog << "LOG:" << "[ Start ] MATCH-PLAYER\n";

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

                            if (paramVerbose) std::clog << "LOG:" << "[ End   ] MATCH-PLAYER\n";
                        }*/

                        //std::cout << match.roundstatsall(i).DebugString() << "/n";
                    }

                    // RESERVATION ID (from last roundstats item)
                    parsedMatch.reservation_id = roundStats.reservationid();

                    // MATCH DURATION in secs
                    parsedMatch.match_duration = roundStats.match_duration();

                    // MATCH DURATION STRING min:sec
                    char buffer_local2[80];
                    strftime(buffer_local2, 80, "%M:%S", localtime(&parsedMatch.match_duration));
                    parsedMatch.match_duration_str = buffer_local2;

                    // map
                    parsedMatch.map      = match.watchablematchinfo().game_map();
                    parsedMatch.mapgroup = match.watchablematchinfo().game_mapgroup();
                    parsedMatch.gametype = match.watchablematchinfo().game_type();

                    //if (paramVerbose) std::clog << "LOG:" << match.DebugString();

                    // link to replay / demo
                    // roundStats.map() is the http link to the bz2 archived demo file
                    parsedMatch.demolink = roundStats.map();
                    
                    //if (paramVerbose) std::clog << "LOG:" << roundStats.DebugString();

                    // calculate ShareCode for demo
                    parsedMatch.sharecode = toDemoShareCode(parsedMatch.matchid, parsedMatch.reservation_id, parsedMatch.tv_port);

                    if (matchList.getOwnIndex(roundStats) >= 5) {
                        parsedMatch.score_ally  = roundStats.team_scores(1);
                        parsedMatch.score_enemy = roundStats.team_scores(0);
                    } else  {
                        parsedMatch.score_ally  = roundStats.team_scores(0);
                        parsedMatch.score_enemy = roundStats.team_scores(1);
                    }

                    parsedMatch.result = roundStats.match_result();
                    parsedMatch.result_str = matchList.getMatchResult(roundStats);

                    linkObj.matches.push_back(parsedMatch);

                    if (paramVerbose) std::clog << "LOG:" << "[ End   ] processing Match\n";
                }
                if (paramVerbose) std::clog << "LOG:" << "[ End   ] processing MatchList\n";

            }
            catch (stop_now_t) {
                return 0;
            }
            catch (CSGO_CLI_TimeoutException)
            {
                Error("Warning", "Timeout on receiving CMsgGCCStrike15_v2_MatchList.\n");
                resultMatchList = false;
            }
            catch (ExceptionHandler& e)
            {
                Error("Fatal error", e.what());
                resultMatchList = false;
            }
            if (paramVerbose) std::clog << "LOG:" << "[ End   ] [ Thread ] MatchList\n";
            return 0;
        });

        matchthread.join();
    }

    //if(paramVerbose) std::clog << "LOG:" << "Waiting for ThreadResults...\n";
    //if(paramVerbose) std::clog << "LOG:" << "Waiting for ThreadResults - COMPLETED\n";

    // OUTPUT

    if (paramPrintUser && resultClientHello)
    {
        wprintf(L"\nName:               %s\n", linkObj.playername);
        std::cout << std::left << std::setw(20) << "SteamID64:" << linkObj.steam_id << "/n";
        //std::cout << std::left << std::setw(20) << "SteamID32:" << toSteamID32(linkObj.steam_id) << "/n";
        std::cout << std::left << std::setw(20) << "SteamID:" << toSteamIDClassic(linkObj.steam_id) << "/n";
        std::cout << std::left << std::setw(20) << "Rank:" << linkObj.rank_str << " (" << linkObj.rank_id << "/" << ranks.size() << ")\n";
        std::cout << std::left << std::setw(20) << "MatchMaking Wins:" << linkObj.rank_wins << "\n";
        std::cout << std::left << std::setw(20) << "Player Level:" 
            << linkObj.player_level_str 
            << " (" << linkObj.player_level << "/40)" 
            << " (XP:" << linkObj.player_cur_xp << ")\n";
        std::cout << std::left << std::setw(20) << "Likes: "
            << linkObj.cmd_friendly     << " x friendly "
            << linkObj.cmd_teaching     << " x teaching "
            << linkObj.cmd_leader       << " x leader\n";
        std::cout << std::left << std::setw(20) << "Medals: "
            << linkObj.medals_arms      << " x arms "
            << linkObj.medals_combat    << " x combat "
            << linkObj.medals_global    << " x global "
            << linkObj.medals_team      << " x team "
            << linkObj.medals_weapon    << " x weapon\n";
        std::cout << std::left << std::setw(20) << "VAC Banned:" << linkObj.vac_banned << "/n";
        //std::cout << std::left << std::setw(20) << "Penalty:" << linkObj.penalty_reason << " (" << (linkObj.penalty_seconds / 60) << "m)\n";
    }
    else if (paramPrintUser)
    {
        Error("\nError", "Steam did not respond in time. Could not print -user.\n");
        result = 1;
    }

    if(paramPrintMatches && resultMatchList)
    {
        wprintf(L"\n Hello %s!\n\n Here are your latest matches:\n", linkObj.playername);
        std::cout << std::endl;
        std::cout << " | "
            << std::setw(19) << std::left << "Match Played" << " | "
            << std::setw(8)  << std::left << "Duration"     << " | "
            << std::setw(8)  << std::left << "Map"          << " | "
            << std::setw(7)  << std::left << "Result"       << " | "
            << std::setw(7)  << std::left << "Score"        << " | ";
        std::cout << std::endl;

        for (auto &match : linkObj.matches)
        {
            std::cout << " | "
                << std::setw(19) << std::left  << match.matchtime_str        << " | "
                << std::setw(8)  << std::left  << match.match_duration_str   << " | "
                << std::setw(8)  << std::left  << (((match.map).empty()) ? "? " : match.map) << " | "
                << std::setw(7)  << std::left  << match.result_str           << " | "
                << std::setw(2)  << std::right << match.score_ally           << " : "
                << std::setw(2)  << std::right << match.score_enemy          << " | ";
            std::cout << std::endl;
            //std::cout << "Demolink:"              << match.demolink       << "/n";
            //std::cout << "Match IP:"              << match.server_ip      << "/n";
            //std::cout << "Match Port:"            << match.tv_port        << "/n";
            //std::cout << "Match Reservation ID:"  << match.reservation_id << "/n";
            //std::cout << "Demo ShareCode:"        << match.sharecode      << "/n";
            //std::cout   << "MatchID:"               << match.matchid        << "/n";
        }
    }
    else if(paramPrintMatches)
    {
        Error("\nError", "Steam did not respond in time. Could not print -matches.\n");
        result = 1;
    }

    if(paramPrintPerf && resultMatchList)
    {
        std::cout << std::endl;
        std::cout << " | "
            << std::setw(19) << std::left << "Match Played" << " | "
            << std::setw(4)  << std::left << "Res."         << " | "
            << std::setw(7)  << std::left << "Score"        << " || "
            << std::setw(2)  << std::left << "K"            << " | "
            << std::setw(2)  << std::left << "A"            << " | "
            << std::setw(2)  << std::left << "D"            << " | "
            << std::setw(3)  << std::left << "MVP"          << " | "
            << std::setw(5)  << std::left << "Score";
        std::cout << std::endl;

        for (auto &match : linkObj.matches)
        {
            for (auto &player : match.scoreboard)
            {
                if(player.account_id == linkObj.account_id)
                {
                    std::cout << "|| "
                        << std::setw(19) << std::left << match.matchtime_str << " | "
                        << std::setw(4)  << std::left << match.result_str    << " | "
                        << std::setw(2)  << std::right << match.score_ally
                        << " : "
                        << std::setw(2)  << std::right << match.score_enemy
                        << " ||"
                        << std::setw(3)  << std::right << player.kills       << " |"
                        << std::setw(3)  << std::right << player.assists     << " |"
                        << std::setw(3)  << std::right << player.deaths      << " |"
                        << std::setw(3)  << std::right << player.mvps        << " | "
                        << std::setw(5)  << std::right << player.score;
                    std::cout << std::endl;
                }
            }
        }
    }
    else if(paramPrintPerf)
    {
        Error("\nError", "Steam did not respond in time. Could not print -perf.\n");
        result = 1;
    }

    if (paramUpload & resultMatchList)
    {
        std::cout << "\n Uploading Demo ShareCodes to https://csgostats.gg/:" << std::endl;

        for (auto &match : linkObj.matches)
        {
            std::string jsonResponse;

            if (uploadShareCode(match.sharecode, jsonResponse) == 0)
            {
                if (processJsonResponse(jsonResponse) != 0)
                {
                    Error("\nError", "Could not parse the response (to the demo sharecode POST request).\n");
                }
            }
            else {
                Error("\nError", "Could not POST demo sharecode.\n");
                //result = 1;
            }
        }
    }
    else if (paramUpload)
    {
        Error("\nError", "Steam did not respond in time. Could not fetch match sharecodes for uploading.\n");
        result = 1;
    }

    // shutdown
    running = false;
    CallbackThread.join();
    CSGOClient::Destroy();
    SteamAPI_Shutdown();

    return result;
}
