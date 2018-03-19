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

#include <steamtypes.h>
#include "SteamId.h"
#include "ShareCode.h"
//#include "CodeUpload.h"

void Error(const char* title, const char* text)
{
    printf("%s: %s\n", title, text);
}

void PrintHelp()
{
    std::cout << "" << CSGO_CLI_BINARYNAME << " - v" << CSGO_CLI_VERSION << "" << std::endl
        << " CS:GO Console shows your user account, stats and latest matches." << std::endl        
        << " " << std::endl
		<< "Usage:" << std::endl
		<< "  command [options] [arguments]" << std::endl
		<< " " << std::endl
		<< "Available commands:" << std::endl
		<< "  -user       " << "Your SteamID, AccountID, MM-Rank and likes" << std::endl
		<< "  -matches    " << "past matches Summary" << std::endl
		<< "  -perf       " << "Your past matches performance in slim readable form" << std::endl	
		<< " " << std::endl
		<< "Options: " << std::endl
		<< "  -h, help    " << " Display this help message" << std::endl
		<< "  -v, verbose " << " verbose" << std::endl
		<< "  -V, Version " << " Display application version" << std::endl
		<< " " << std::endl
    << std::endl;
}

struct stop_now_t { };

int main(int argc, char** argv)
{
    CSGOMMLinkObject linkObj;

    int res = 0;

    bool paramVerbose      = true;
    bool paramPrintUser    = false;
    bool paramPrintMatches = false;
    bool paramPrintPerf    = false;
	bool paramUpload       = false;

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
    }

    for( int il=1; il < argc; il = il + 1 )
    {
        std::string option = argv[il];
        if(option == "-h" || option == "--h" || option == "-help" || option == "/?"){
            PrintHelp();
            return 0;
        } else if(option == "-V" || option == "--V" || option == "-version"){
            std::cout << "" << CSGO_CLI_BINARYNAME << " - version (" << CSGO_CLI_VERSION << ")" << std::endl;
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
			paramUpload = true;
        } else if(option != ""){
            std::cerr << "ERROR - InvalidArgument: " << option << std::endl;
            std::cerr << "Check '" << CSGO_CLI_BINARYNAME << " -help'" << std::endl << std::endl;
            //PrintHelp();
            return 1;
        }
    }

#ifdef _WIN32
    HWND test = FindWindowW(0, L"Counter-Strike: Global Offensive");
    if(test != NULL)
    {
        Error("Warning", "CS:GO is currently running.\nPlease close the game, before running this program.\n");
        return 1;
    }
#endif

    if(paramVerbose) std::clog << "LOG:" << "--- START-STEAM_INIT" << std::endl;

    if (SteamAPI_RestartAppIfNecessary(k_uAppIdInvalid))
        return 1;


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

	if(paramVerbose) std::clog << "LOG:" << "--- END-STEAM_INIT" << std::endl;

    if(paramVerbose) std::clog << "LOG:" << "--- START-StartCallbackThread" << std::endl;
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
    if(paramVerbose) std::clog << "LOG:" << "--- END-StartCallbackThread" << std::endl;
	
    if(paramVerbose) std::clog << "LOG:" << "--- START-Trying-GameClient-Connect" << std::endl;
    bool resGc = false;
    try
    {
        // make sure we are connected to the GameClient
        if(paramVerbose) std::clog << "LOG:" << "--- requesting: GameClient Connection" << std::endl;
        CSGOClient::GetInstance()->WaitForGcConnect();
        if(paramVerbose) std::clog << "LOG:" << "--- Successful: GameClient connected" << std::endl;
        resGc = true;

        linkObj.account_id = SteamUser()->GetSteamID().GetAccountID();
        linkObj.steam_id = SteamUser()->GetSteamID().ConvertToUint64();
        linkObj.playername = SteamFriends()->GetPersonaName();
    }
    catch (ExceptionHandler& e)
    {
        Error("Fatal error", e.what());
        resGc = false;
    }
    if(paramVerbose) std::clog << "LOG:" << "--- END-Trying-GameClient-Connect" << std::endl;

    if(!resGc)
    {
        Error("Fatal error", "GameClient could not connect.\n");
        return 1;
    }

	bool resHello = false;
	if (paramPrintUser)
	{
		if (paramVerbose) std::clog << "LOG:" << "--- START-Thread-Hello" << std::endl;
		
		auto hellothread = std::thread([&linkObj, paramVerbose, &resHello, ranks, levels]()
		{
			try
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));
				
				CSGOMMHello mmhello;
				if (paramVerbose) std::clog << "LOG:" << "--- requesting MMHello" << std::endl;
				mmhello.RefreshWait();
				if (paramVerbose) std::clog << "LOG:" << "--- got MMHello" << std::endl;

				resHello = true;
				//if (paramVerbose) std::clog << "DEBUG:" << mmhello.exposedProt.DebugString();
				
				linkObj.vac_banned = mmhello.exposedProt.vac_banned();
				linkObj.player_level = mmhello.exposedProt.player_level();
				linkObj.player_level_str = levels[mmhello.exposedProt.player_level() - 1];
				linkObj.player_cur_xp = mmhello.exposedProt.player_cur_xp();

				linkObj.medals_arms = mmhello.exposedProt.medals().medal_arms();
				linkObj.medals_combat = mmhello.exposedProt.medals().medal_combat();
				linkObj.medals_global = mmhello.exposedProt.medals().medal_global();
				linkObj.medals_team = mmhello.exposedProt.medals().medal_team();
				linkObj.medals_weapon = mmhello.exposedProt.medals().medal_weapon();

				linkObj.penalty_seconds = mmhello.exposedProt.penalty_seconds();
				linkObj.penalty_reason = mmhello.exposedProt.penalty_reason();
												
				if (mmhello.exposedProt.ranking().has_rank_id())
				{
					linkObj.rank_id = mmhello.exposedProt.ranking().rank_id();
					linkObj.rank_str = ranks[mmhello.exposedProt.ranking().rank_id() - 1];
				}
				if (mmhello.exposedProt.ranking().has_wins())
					linkObj.rank_wins = mmhello.exposedProt.ranking().wins();
				if (mmhello.exposedProt.ranking().has_rank_change())
					linkObj.rank_change = mmhello.exposedProt.ranking().rank_change();
				if (mmhello.exposedProt.commendation().has_cmd_friendly())
					linkObj.cmd_friendly = mmhello.exposedProt.commendation().cmd_friendly();
				if (mmhello.exposedProt.commendation().has_cmd_teaching())
					linkObj.cmd_teaching = mmhello.exposedProt.commendation().cmd_teaching();
				if (mmhello.exposedProt.commendation().has_cmd_leader())
					linkObj.cmd_leader = mmhello.exposedProt.commendation().cmd_leader();
			}
			catch (stop_now_t) {
				return 0;
			}
			catch (CSGO_CLI_TimeoutException)
			{
				Error("Warning", "Timeout on receiving CMsgGCCStrike15_v2_MatchmakingGC2ClientHello\n");
				resHello = false;
			}
			catch (ExceptionHandler& e)
			{
				Error("Fatal error", e.what());
				resHello = false;
			}
			if (paramVerbose) std::clog << "LOG:" << "--- END-Thread-Hello" << std::endl;
			return 0;
		});

		hellothread.join();
	}

	bool resList = false;

	if (paramPrintMatches)
	{
		if (paramVerbose) std::clog << "LOG:" << "--- START-Thread-MatchList" << std::endl;
		
		auto matchthread = std::thread([&linkObj, paramVerbose, &resList]()
		{
			try
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_MATCHLIST_DELAY));
				// refresh match list
				CSGOMatchList matchList;
				if (paramVerbose) std::clog << "LOG:" << "--- requesting MatchList" << std::endl;
				matchList.RefreshWait();
				if (paramVerbose) std::clog << "LOG:" << "--- got MatchList" << std::endl;

				resList = true;

				if (paramVerbose) std::clog << "LOG:" << "--- START-MATCHLIST" << std::endl;

				for (auto &match : matchList.Matches())
				{
					if (paramVerbose) std::clog << "LOG:" << "--- START-MATCH" << std::endl;

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
							if (paramVerbose) std::clog << "LOG:" << "--- START-MATCH-PLAYER" << std::endl;

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

							if (paramVerbose) std::clog << "LOG:" << "--- END-MATCH-PLAYER" << std::endl;
						}*/
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
					parsedMatch.map = match.watchablematchinfo().game_map();

					//if (paramVerbose) std::clog << "LOG:" << match.DebugString();

					// link to replay / demo
					// roundStats.map() is the http link to the bz2 archived demo file 
					parsedMatch.demolink = roundStats.map();

					//if (paramVerbose) std::clog << "LOG:" << roundStats.DebugString();

					// calculate ShareCode for demo
					parsedMatch.sharecode = toDemoShareCode(parsedMatch.matchid, parsedMatch.reservation_id, parsedMatch.tv_port);
					
					if (matchList.getOwnIndex(roundStats) >= 5)	{
						parsedMatch.score_ally = roundStats.team_scores(1);
						parsedMatch.score_enemy = roundStats.team_scores(0);
					} else 	{
						parsedMatch.score_ally = roundStats.team_scores(0);
						parsedMatch.score_enemy = roundStats.team_scores(1);
					}

					parsedMatch.result = roundStats.match_result();
					parsedMatch.result_str = matchList.getMatchResult(roundStats);
														
					linkObj.matches.push_back(parsedMatch);

					if (paramVerbose) std::clog << "LOG:" << "--- END-MATCH" << std::endl;
				}
				if (paramVerbose) std::clog << "LOG:" << "--- END-MATCHLIST" << std::endl;

			}
			catch (stop_now_t) {
				return 0;
			}
			catch (CSGO_CLI_TimeoutException)
			{
				Error("Warning", "Timeout on receiving CMsgGCCStrike15_v2_MatchList\n");
				resList = false;
			}
			catch (ExceptionHandler& e)
			{
				Error("Fatal error", e.what());
				resList = false;
			}
			if (paramVerbose) std::clog << "LOG:" << "--- END-Thread-MatchList" << std::endl;
			return 0;
		});

		matchthread.join();
	}

    //if(paramVerbose) std::clog << "LOG:" << "--- Waiting for ThreadResults..." << std::endl;
    //if(paramVerbose) std::clog << "LOG:" << "--- Waiting for ThreadResults - COMPLETED" << std::endl;

	// OUTPUT

	if (paramPrintUser && resHello)
	{
		std::cout << std::endl;
		std::cout << std::left << std::setw(20) << "Name:" << linkObj.playername << std::endl;
		std::cout << std::left << std::setw(20) << "SteamID64:" << linkObj.steam_id << std::endl;
		//std::cout << std::left << std::setw(20) << "SteamID32:" << toSteamID32(linkObj.steam_id) << std::endl;
		std::cout << std::left << std::setw(20) << "SteamID:" << toSteamIDClassic(linkObj.steam_id) << std::endl;
		std::cout << std::left << std::setw(20) << "Rank:" << linkObj.rank_str << " (" << linkObj.rank_id << "/" << ranks.size() << ")" << std::endl;
		std::cout << std::left << std::setw(20) << "MatchMaking Wins:" << linkObj.rank_wins << std::endl;
		std::cout << std::left << std::setw(20) << "Player Level:" << linkObj.player_level_str << " (" << linkObj.player_level << "/40)" << " (XP:" << linkObj.player_cur_xp << ")"<< std::endl;
		std::cout << std::left << std::setw(20) << "Likes: "
			<< linkObj.cmd_friendly << " x " << "friendly "
			<< linkObj.cmd_teaching << " x " << "teaching "
			<< linkObj.cmd_leader << " x " << "leader" << std::endl;
		std::cout << std::left << std::setw(20) << "Medals: "
			<< linkObj.medals_arms << " x arms "
			<< linkObj.medals_combat << " x combat "
			<< linkObj.medals_global << " x global "
			<< linkObj.medals_team << " x team "
			<< linkObj.medals_weapon << " x weapon" << std::endl;
		std::cout << std::left << std::setw(20) << "VAC Banned:" << linkObj.vac_banned << std::endl;
		//std::cout << std::left << std::setw(20) << "Penalty:" << linkObj.penalty_reason << " (" << (linkObj.penalty_seconds / 60) << "m)" << std::endl;
	}
	else if (paramPrintUser)
	{
		Error("\nError", "Steam did not respond in time. Could not print -user\n");
		res = 1;
	}
	
    if(paramPrintMatches && resList)
    {
		std::cout << std::endl;
        std::cout << "=| "
            << std::setw(19) << std::left << "Match Played" << " | "
			<< std::setw(8) << std::left << "Map" << " | "
            << std::setw(7) << std::left << "Result" << " | "
            << std::setw(7) << std::left << "Score";
        std::cout << std::endl;

        for (auto &match : linkObj.matches)
        {			
			std::cout << "| "
				<< std::setw(19) << std::left << match.matchtime_str << " | "
				<< std::setw(8) << std::left << match.map << " | "
				<< std::setw(4) << std::left << match.result_str << " | "
				<< std::setw(2) << std::right << match.score_ally << " : "
				<< std::setw(2) << std::right << match.score_enemy << " | ";
			std::cout << std::endl;
			std::cout << "Demolink:" << match.demolink << std::endl;
			std::cout << "Match IP:" << match.server_ip << std::endl;
			std::cout << "Match Port:" << match.tv_port << std::endl;
			std::cout << "Match Reservation ID:" << match.reservation_id << std::endl;
			std::cout << "Match Duration:" << match.match_duration_str << std::endl;
			std::cout << "Demo ShareCode:" << match.sharecode << std::endl;
            std::cout << std::endl;
        }
    }
    else if(paramPrintMatches)
    {
        Error("\nError", "Steam did not respond in time. Could not print -matches\n");
        res = 1;
    }
	
    if(paramPrintPerf && resList)
    {
        std::cout << std::endl;
        std::cout << "=| "
            << std::setw(19) << std::left << "Match Played" << " | "
            << std::setw(4) << std::left << "Res." << " | "
            << std::setw(7) << std::left << "Score" << " || "
            << std::setw(2) << std::left << "K" << " | "
            << std::setw(2) << std::left << "A" << " | "
            << std::setw(2) << std::left << "D" << " | "
            << std::setw(3) << std::left << "MVP" << "| "
            << std::setw(5) << std::left << "Score";
        std::cout << std::endl;

        for (auto &match : linkObj.matches)
        {
            for (auto &player : match.scoreboard)
            {
                if(player.account_id == linkObj.account_id)
                {
                    std::cout << "|| "
                        << std::setw(19) << std::left << match.matchtime_str << " | "
                        << std::setw(4) << std::left << match.result_str << " | "
                        << std::setw(2) << std::right << match.score_ally
                        << " : "
                        << std::setw(2) << std::right << match.score_enemy
                        << " ||"
                        << std::setw(3) << std::right << player.kills << " |"
                        << std::setw(3) << std::right << player.assists << " |"
                        << std::setw(3) << std::right << player.deaths << " |"
                        << std::setw(3) << std::right << player.mvps << " | "
                        << std::setw(5) << std::right << player.score;
                    std::cout << std::endl;
                }
            }
        }
    }
    else if(paramPrintPerf)
    {
        Error("\nError", "Steam did not respond in time. Could not print -perf\n");
        res = 1;
    }

	if (paramUpload & resList)
	{
		curl(linkObj.matches[1].sharecode);
	}
	else if (paramUpload)
	{
		Error("\nError", "Steam did not respond in time. Could not upload match sharecodes.\n");
		res = 1;
	}
		    
    // shutdown
    running = false;
    CallbackThread.join();
    CSGOClient::Destroy();
    SteamAPI_Shutdown();

    return res;
}
