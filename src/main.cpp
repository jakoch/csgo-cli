#include "VersionAndConstants.h"
#include "ExceptionHandler.h"
#include <thread>
#include "CSGOMatchList.h"
#include "CSGOMMHello.h"
#include "CSGOMatchPlayerScore.h"
#include "CSGOMatchData.h"
#include "DataObject.h"
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

std::string getYear()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y");
	return ss.str();
}

void PrintHelp()
{
	std::cout << "" << CSGO_CLI_BINARYNAME << " - v" << CSGO_CLI_VERSION << "\n"
		<< "Copyright (c) " << getYear() << " Jens A. Koch.\n"
		<< "\n"
		<< " CS:GO Console shows your user account, stats and latest matches.\n"
		<< " You can also use the tool to upload demo sharecodes to csgostats.gg.\n"
		<< "\n"
		<< "Usage:\n"
		<< "  command [options] [arguments]\n"
		<< "\n"
		<< "Available commands:\n"
		<< "  -user       " << " Show your profile (SteamID, AccountID, MM-Rank, Likes, VAC-Status)\n"
		<< "  -matches    " << " Show your past matches in table form\n"
		<< "  -upload     " << " Upload your past matches to csgostats.gg\n"
        //<< "  -perf       " << " Show your past matches performance in compact form\n"
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
		Error("Fatal Error", "Steam not running. SteamAPI_Init() failed.\nPlease run Steam.\n");
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
		Error("Fatal Error", "Steam user not logged in. SteamUser()->BLoggedOn() returned false.\nPlease log in.\n");
		exit(1);
	}

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
				Steam_RunCallbacks(GetHSteamPipe(), false);
			}
			catch (ExceptionHandler& e)
			{
				Error("Fatal Error", e.what());
				exit(1);
			}
		};
	});
	if (verbose) std::clog << "LOG:" << "[ End   ] CallbackThread & Steam_RunCallbacks\n";
	return CallbackThread;
}

void initGameClientConnection(DataObject linkObj, bool &verbose)
{
	if (verbose) std::clog << "LOG:" << "[ Start ] Trying to establish a GameClient Connection\n";
	bool resultGameClientConnection = false;
	try
	{
		// make sure we are connected to the GameClient
		if (verbose) std::clog << "LOG:" << "          Requesting: GameClient Connection\n";
		CSGOClient::GetInstance()->WaitForGameClientConnect();
		if (verbose) std::clog << "LOG:" << "          Successful: GameClient connected!\n";
		resultGameClientConnection = true;

		linkObj.account_id = SteamUser()->GetSteamID().GetAccountID();
		linkObj.steam_id = SteamUser()->GetSteamID().ConvertToUint64();
		linkObj.steam_player_level = SteamUser()->GetPlayerSteamLevel();
		linkObj.playername = toWChar(SteamFriends()->GetPersonaName());
	}
	catch (ExceptionHandler& e)
	{
		Error("Fatal error", e.what());
		resultGameClientConnection = false;
	}

	if (!resultGameClientConnection)
	{
		Error("Fatal error", "GameClient could not connect.\n");
		exit(1);
	}
	if (verbose) std::clog << "LOG:" << "[ End   ] Trying to establish a GameClient Connection\n";
}

bool getAccountInfo(DataObject linkObj, bool &verbose)
{
	if (verbose) std::clog << "LOG:" << "[ Start ] [ Thread ] getUserInfo\n";

	bool resultClientHello = false;

	auto hellothread = std::thread([&linkObj, verbose, &resultClientHello]()
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_HELLO_DELAY));

			CSGOMMHello mmhello;
			if (verbose) std::clog << "LOG:" << "          Requesting: Hello\n";
			mmhello.RefreshWait();
			if (verbose) std::clog << "LOG:" << "          Got Hello\n";

			resultClientHello = true;

			//if (paramVerbose) std::clog << "DEBUG:" << mmhello.data.DebugString();

			// player level
			linkObj.player_level		= mmhello.data.player_level();
			linkObj.player_cur_xp		= mmhello.data.player_cur_xp();
			// medals
			/*linkObj.medals_arms		= mmhello.data.medals().medal_arms();
			linkObj.medals_combat		= mmhello.data.medals().medal_combat();
			linkObj.medals_global		= mmhello.data.medals().medal_global();
			linkObj.medals_team			= mmhello.data.medals().medal_team();
			linkObj.medals_weapon		= mmhello.data.medals().medal_weapon();*/
			// vac status
			linkObj.vac_banned			= mmhello.data.vac_banned();
			linkObj.penalty_seconds		= mmhello.data.penalty_seconds();
			linkObj.penalty_reason		= mmhello.data.penalty_reason();
			// ranks
			if (mmhello.data.has_ranking()) {				
				linkObj.rank_id			= mmhello.data.ranking().rank_id();
				linkObj.rank_wins		= mmhello.data.ranking().wins();				
				linkObj.rank_change		= mmhello.data.ranking().rank_change();
								
			}
			// commendations
			if (mmhello.data.has_commendation()) {				
				linkObj.cmd_friendly	= mmhello.data.commendation().cmd_friendly();
				linkObj.cmd_teaching	= mmhello.data.commendation().cmd_teaching();
				linkObj.cmd_leader		= mmhello.data.commendation().cmd_leader();			
			}
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
		if (verbose) std::clog << "LOG:" << "[ End   ] [ Thread ] Hello Matchmaking\n";
		return 0;
	});

	hellothread.join();

	return resultClientHello;
}

bool getMatches(DataObject linkObj, bool &verbose)
{
	bool resultMatchList = false;
	
	if (verbose) std::clog << "LOG:" << "[ Start ] [ Thread ] MatchList\n";

	auto matchthread = std::thread([&linkObj, verbose, &resultMatchList]()
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_MATCHLIST_DELAY));
			// refresh match list
			CSGOMatchList matchList;
			if (verbose) std::clog << "LOG:" << "requesting MatchList\n";
			matchList.RefreshWait();
			if (verbose) std::clog << "LOG:" << "got MatchList\n";

			resultMatchList = true;

			if (verbose) std::clog << "LOG:" << "[ Start ] processing MatchList\n";

			for (auto &match : matchList.Matches())
			{
				if (verbose) std::clog << "LOG:" << "[ Start ] processing Match\n";

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
				parsedMatch.map = match.watchablematchinfo().game_map();
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
					parsedMatch.score_ally = roundStats.team_scores(1);
					parsedMatch.score_enemy = roundStats.team_scores(0);
				}
				else {
					parsedMatch.score_ally = roundStats.team_scores(0);
					parsedMatch.score_enemy = roundStats.team_scores(1);
				}

				parsedMatch.result = roundStats.match_result();
				parsedMatch.result_str = matchList.getMatchResult(roundStats);

				linkObj.matches.push_back(parsedMatch);

				if (verbose) std::clog << "LOG:" << "[ End   ] processing Match\n";
			}
			if (verbose) std::clog << "LOG:" << "[ End   ] processing MatchList\n";

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
		if (verbose) std::clog << "LOG:" << "[ End   ] [ Thread ] MatchList\n";
		return 0;
	});

	matchthread.join();

	return resultMatchList;
}


void exitIfGameIsRunning()
{
#ifdef _WIN32
	HWND test = FindWindowW(0, L"Counter-Strike: Global Offensive");
	if (test != NULL)
	{
		Error("Warning", "CS:GO is currently running.\nPlease close the game, before running this program.\n");
		exit(1);
	}
#endif
}

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

void printAccountInfo(DataObject data)
{
	wprintf(L"\nName:               %s\n", data.playername);
	fflush(stdout);

	std::cout << std::left << std::setw(20) << "SteamID64:" << data.steam_id << std::endl;
	//std::cout << std::left << std::setw(20) << "SteamID32:" << toSteamID32(linkObj.steam_id) << std::endl;
	std::cout << std::left << std::setw(20) << "SteamID:" << toSteamIDClassic(data.steam_id) << std::endl;
	std::cout << std::left << std::setw(20) << "Rank:" << data.rank_str << "(" << data.rank_id << "/" << ranks.size() << ")" << std::endl;
	std::cout << std::left << std::setw(20) << "MatchMaking Wins:" << data.rank_wins << std::endl;
	std::cout << std::left << std::setw(20) << "Player Level:"
		<< data.player_level_str
		<< " (" << data.player_level << "/40)"
		<< " (XP:" << data.player_cur_xp << ")" << std::endl;
	std::cout << std::left << std::setw(20) << "Likes: "
		<< data.cmd_friendly << " x friendly "
		<< data.cmd_teaching << " x teaching "
		<< data.cmd_leader << " x leader" << std::endl;
	/*
	std::cout << std::left << std::setw(20) << "Medals: "
		<< linkObj.medals_arms      << " x arms "
		<< linkObj.medals_combat    << " x combat "
		<< linkObj.medals_global    << " x global "
		<< linkObj.medals_team      << " x team "
		<< linkObj.medals_weapon    << " x weapon\n";
		*/
	std::cout << std::left << std::setw(20) << "VAC Banned:" << data.vac_banned << std::endl;
	//std::cout << std::left << std::setw(20) << "Penalty:" << linkObj.penalty_reason << " (" << (linkObj.penalty_seconds / 60) << "m)" << std::endl;
}

void printMatches(DataObject data)
{
	wprintf(L"\n Hello %s!\n\n Here are your latest matches:\n", data.playername);
	std::cout << std::endl;
	std::cout << " | "
		<< std::setw(19) << std::left << "Match Played" << " | "
		<< std::setw(8) << std::left << "Duration" << " | "
		<< std::setw(8) << std::left << "Map" << " | "
		<< std::setw(7) << std::left << "Result" << " | "
		<< std::setw(7) << std::left << "Score" << " | ";
	std::cout << std::endl;

	for (auto &match : data.matches)
	{
		std::cout << " | "
			<< std::setw(19) << std::left << match.matchtime_str << " | "
			<< std::setw(8) << std::left << match.match_duration_str << " | "
			<< std::setw(8) << std::left << (((match.map).empty()) ? "? " : match.map) << " | "
			<< std::setw(7) << std::left << match.result_str << " | "
			<< std::setw(2) << std::right << match.score_ally << " : "
			<< std::setw(2) << std::right << match.score_enemy << " | ";
		std::cout << std::endl;
		//std::cout << "Demolink:"              << match.demolink       << "\n";
		//std::cout << "Match IP:"              << match.server_ip      << "/n";
		//std::cout << "Match Port:"            << match.tv_port        << "/n";
		//std::cout << "Match Reservation ID:"  << match.reservation_id << "/n";
		//std::cout << "Demo ShareCode:"        << match.sharecode      << "/n";
		//std::cout   << "MatchID:"               << match.matchid        << "/n";
	}
}

void printScoreboard(DataObject data)
{
	std::cout << std::endl;
	std::cout << " | "
		<< std::setw(19) << std::left << "Match Played" << " | "
		<< std::setw(4) << std::left << "Res." << " | "
		<< std::setw(7) << std::left << "Score" << " || "
		<< std::setw(2) << std::left << "K" << " | "
		<< std::setw(2) << std::left << "A" << " | "
		<< std::setw(2) << std::left << "D" << " | "
		<< std::setw(3) << std::left << "MVP" << " | "
		<< std::setw(5) << std::left << "Score";
	std::cout << std::endl;

	for (auto &match : data.matches)
	{
		for (auto &player : match.scoreboard)
		{
			if (player.account_id == data.account_id)
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

void uploadDemoShareCode(DataObject data)
{
	std::cout << "\n Uploading Demo ShareCodes to https://csgostats.gg/:" << std::endl;

	for (auto &match : data.matches)
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
			//exit(1);
		}
	}
}

int main(int argc, char** argv)
{
	SetConsoleOutputCP(CP_UTF8);
	
	exitIfGameIsRunning();

	int result = 0;

	bool paramVerbose = false;
	bool paramPrintUser = false;
	bool paramPrintMatches = false;
	bool paramPrintScoreboard = false;
	bool paramUploadShareCode = false;

	DataObject data;
	
	// default action
	if (argc <= 1)
	{
		PrintHelp();
		return 0;
	}

	for (int i = 1; i < argc; i = i + 1)
	{
		std::string option = argv[i];
		if (option == "-h" || option == "--h" || option == "-help" || option == "/?") {
			PrintHelp();
			return 0;
		}
		else if (option == "-V" || option == "--V" || option == "-version") {
			std::cout << "" << CSGO_CLI_BINARYNAME << " version " << CSGO_CLI_VERSION << std::endl;
			return 0;
		}
		else if (option == "-v" || option == "--v" || option == "-verbose") {
			paramVerbose = true;
		}
		else if (option == "-matches") {
			paramPrintMatches = true;
		}
		else if (option == "-perf") {
			paramPrintScoreboard = true;
		}
		else if (option == "-user") {
			paramPrintUser = true;
		}
		else if (option == "-upload") {
			paramPrintMatches = true;
			paramUploadShareCode = true;
		}
		else if (option != "") {
			std::cerr << "ERROR (invalid argument): " << option << '/n';
			std::cerr << "Check '" << CSGO_CLI_BINARYNAME << " -help/n" << std::endl;
			return 1;
		}
	}

	// CONNECT TO STEAM_API
	
	initSteamAPI(paramVerbose);
	
	bool running = true;

	std::thread CallbackThread = createCallbackThread(running, paramVerbose);	

	initGameClientConnection(data, paramVerbose);

	// GET DATA

	if (paramPrintUser) {
		if (!getAccountInfo(data, paramVerbose)) {
			Error("\nError", "Steam did not respond in time. Could not print -user.\n");
			exit(1);
		}
	}

	if (paramPrintMatches || paramPrintScoreboard || paramUploadShareCode) {
		if (!getMatches(data, paramVerbose)) {
			Error("\nError", "Steam did not respond in time.\n");
			exit(1);
		}
	}
			
    // OUTPUT

	if (paramPrintUser) {		
		printAccountInfo(data);
	}

	if (paramPrintMatches) {
		printMatches(data);
	}
   
	if (paramPrintScoreboard) {
		printScoreboard(data);
	}

	if (paramUploadShareCode) {
		uploadDemoShareCode(data);
	}
	
    // SHUTDOWN

	if (paramVerbose) std::clog << "LOG:" << "Init Shutdown...\n";

    running = false;
    CallbackThread.join();
    CSGOClient::Destroy();
    SteamAPI_Shutdown();

	if (paramVerbose) std::clog << "LOG:" << "Shutdown: Done.\n";

    return result;
}
