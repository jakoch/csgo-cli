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
#include "ConsoleTable.h"

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
		//<< "  -scoreboard " << " Show your past matches in scoreboard form\n"
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

		data.account_id			= SteamUser()->GetSteamID().GetAccountID();
		data.steam_id			= SteamUser()->GetSteamID().ConvertToUint64();
		data.steam_player_level	= SteamUser()->GetPlayerSteamLevel();
		data.playername			= toWChar(SteamFriends()->GetPersonaName());
	}
	catch (ExceptionHandler& e)
	{
		Error("Fatal error", e.what());
		result = false;
	}

	if (!result)
	{
		Error("Fatal error", "GameClient could not connect.\n");
		exit(1);
	}
	if (verbose) std::clog << "LOG:" << "[ End   ] Trying to establish a GameClient Connection\n";
}

bool getUserInfo(DataObject &data, bool &verbose)
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

			//if (paramVerbose) std::clog << "DEBUG:" << mmhello.data.DebugString();

			// player level
			data.player_level		= mmhello.data.player_level();
			data.player_cur_xp		= mmhello.data.player_cur_xp();
			// medals
			/*data.medals_arms		= mmhello.data.medals().medal_arms();
			data.medals_combat		= mmhello.data.medals().medal_combat();
			data.medals_global		= mmhello.data.medals().medal_global();
			data.medals_team		= mmhello.data.medals().medal_team();
			data.medals_weapon		= mmhello.data.medals().medal_weapon();*/
			// vac status
			data.vac_banned			= mmhello.data.vac_banned();
			data.penalty_seconds	= mmhello.data.penalty_seconds();
			data.penalty_reason		= mmhello.data.penalty_reason();
			// ranks
			if (mmhello.data.has_ranking()) {				
				data.rank_id		= mmhello.data.ranking().rank_id();
				data.rank_wins		= mmhello.data.ranking().wins();				
				data.rank_change	= mmhello.data.ranking().rank_change();								
			}
			// commendations
			if (mmhello.data.has_commendation()) {				
				data.cmd_friendly	= mmhello.data.commendation().cmd_friendly();
				data.cmd_teaching	= mmhello.data.commendation().cmd_teaching();
				data.cmd_leader		= mmhello.data.commendation().cmd_leader();			
			}
		}
		catch (CSGO_CLI_TimeoutException)
		{
			Error("Warning", "Timeout on receiving UserInfo.\n");
			result = false;
		}
		catch (ExceptionHandler& e)
		{
			Error("Fatal error", e.what());
			result = false;
		}
		if (verbose) std::clog << "LOG:" << "[ End   ] [ Thread ] getUserInfo\n";
		return 0;
	});

	hellothread.join();

	return result;
}

bool getMatches(DataObject &data, bool &verbose)
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
				
				for (auto &match : matchList.Matches())
				{
					if (verbose) std::clog << "LOG:" << "[ Start ] processing Match\n";

					CSGOMatchData parsedMatch;

					// MATCHID
					parsedMatch.matchid = match.matchid();

					// MATCHTIME
					parsedMatch.matchtime = match.matchtime();

					// MATCHTIME_STR
					char buffer_local[30];
					strftime(buffer_local, 30, "%Y-%m-%d %H:%M:%S", localtime(&parsedMatch.matchtime));
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
					char buffer_local2[8];
					strftime(buffer_local2, 8, "%M:%S", localtime(&parsedMatch.match_duration));
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

					data.matches.push_back(parsedMatch);

					if (verbose) std::clog << "LOG:" << "[ End   ] processing Match\n";
				}
			}
			if (verbose) std::clog << "LOG:" << "[ End   ] processing MatchList\n";

		}
		catch (CSGO_CLI_TimeoutException)
		{
			Error("Warning", "Timeout on receiving MatchList.\n");
			result = false;
		}
		catch (ExceptionHandler& e)
		{
			Error("Fatal error", e.what());
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
		Error("Warning", "\nCS:GO is currently running.\nPlease close the game, before running this program.\n");
		exit(1);
	}
#endif
}

void printAccountInfo(DataObject &data)
{
	char name[40];
	sprintf(name, "%ls", data.playername); // %ls format = wchar_t*

	char rank[50];
	sprintf(rank, "%s (%d/18)", data.getPlayerRank().c_str(), data.rank_id);

	char level[50];
	sprintf(level, "%s (%d/40) (XP:%d)", data.getPlayerLevel().c_str(), data.player_level, data.player_cur_xp);

	char likes[50];
	sprintf(likes, "%d x friendly, %d x teaching, %d x leader", data.cmd_friendly, data.cmd_teaching, data.cmd_leader);

	/*
	char medals[50];
	sprintf(likes, "%d x arms, %d x combat, %d x global, %d x team, %d x weapon", 
		data.medals_arms, data.medals_combat, data.medals_global, data.medals_team, data.medals_weapon);
	*/

	char penalty[100];
	sprintf(penalty, "%d (%d/60m)", data.penalty_reason, (data.penalty_seconds / 60));

	TableFormat t;
	t << std::endl;
	t << "Name:"			    << name								<< std::endl;
	t << "SteamID64:"			<< data.steam_id					<< std::endl;
	//t << "SteamID32:"			<< toSteamID32(data.steam_id)		<< std::endl;
	t << "SteamID:"				<< toSteamIDClassic(data.steam_id)	<< std::endl;
	t << "Rank:"				<< rank								<< std::endl;
	t << "MatchMaking Wins:"	<< data.rank_wins					<< std::endl;
	t << "Player Level:"		<< level							<< std::endl;
	t << "Likes: "				<< likes							<< std::endl;
	//t << "Medals:"            << medals							<< std::endl;
	t << "VAC Status:"			<< data.getVacStatus()				<< std::endl;
	t << "Penalty:"				<< penalty							<< std::endl;
}

void printMatches(DataObject &data)
{
	wprintf(L"\nHello %s!\n\n", data.playername);

	if (!data.has_matches_played) {
		std::cout << "Your CS:GO match history is empty." << std::endl;
		return;
	}

	std::cout << "Here are your latest matches:" << std::endl;

	ConsoleTable t{ "Match Played", "Duration", "Map", "Result", "Score Ally", "Score Enemy" };
	t.setPadding(1);
	t.setStyle(0);

	int i = 0;
	for (auto &match : data.matches)
	{
		t.updateRow(i, 0, match.matchtime_str);
		t.updateRow(i, 1, match.match_duration_str);
		t.updateRow(i, 2, (((match.map).empty()) ? "? " : match.map));
		t.updateRow(i, 3, match.result_str);
		t.updateRow(i, 4, std::to_string(match.score_ally));
		t.updateRow(i, 5, std::to_string(match.score_enemy));
		//std::cout << "Demolink:"              << match.demolink       ;
		//std::cout << "Match IP:"              << match.server_ip      ;
		//std::cout << "Match Port:"            << match.tv_port        ;
		//std::cout << "Match Reservation ID:"  << match.reservation_id ;
		//std::cout << "Demo ShareCode:"        << match.sharecode      ;
		//std::cout << "MatchID:"               << match.matchid        ;
		i++;
	}
}

void printScoreboard(DataObject &data)
{
	std::cout << std::endl;

	if (!data.has_matches_played) {
		std::cout << "Your CS:GO match history is empty." << std::endl;
		return;
	}

	ConsoleTable t{ "Match Played", "Res.", "Score", "K", "A", "D", "MVP", "Score" };
	t.setPadding(1);
	t.setStyle(0);

	int i = 0;

	for (auto &match : data.matches)
	{
		for (auto &player : match.scoreboard)
		{
			if (player.account_id == data.account_id)
			{
				t.updateRow(i, 0, match.matchtime_str);
				t.updateRow(i, 1, match.result_str);
				t.updateRow(i, 2, std::to_string(match.score_ally));
				t.updateRow(i, 3, std::to_string(match.score_enemy));
				t.updateRow(i, 4, std::to_string(player.kills));
				t.updateRow(i, 5, std::to_string(player.assists));
				t.updateRow(i, 6, std::to_string(player.deaths));
				t.updateRow(i, 7, std::to_string(player.mvps));
				t.updateRow(i, 8, std::to_string(player.score));
			}
		}
	}
}

void uploadDemoShareCode(DataObject &data)
{
	if (!data.has_matches_played) {
		std::cout << "No demo sharecodes to upload." << std::endl;
		return;
	}

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
		else if (option == "-scoreboard") {
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
			std::cerr << "ERROR (invalid argument): " << option << std::endl;
			std::cerr << "Please check: '" << CSGO_CLI_BINARYNAME << " -help'" << std::endl;
			return 1;
		}
	}

	// CONNECT TO STEAM_API
	
	initSteamAPI(paramVerbose);
	
	bool running = true;

	std::thread CallbackThread = createCallbackThread(running, paramVerbose);	
	
	DataObject data;

	initGameClientConnection(data, paramVerbose);

	// GET DATA

	if (paramPrintUser) {
		if (!getUserInfo(data, paramVerbose)) {
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

    running = false;
    CallbackThread.join();
    CSGOClient::Destroy();
    SteamAPI_Shutdown();

    return result;
}
