// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fmt/color.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <steam/steamtypes.h>

#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../platform/windows/WinCliColors.h"
#include "../VersionAndConstants.h"

#include "../commands/cmd.globalstats.h"
#include "../commands/cmd.help.h"
#include "../commands/cmd.matches.h"
#include "../commands/cmd.scoreboard.h"
#include "../commands/cmd.upload.h"
#include "../commands/cmd.user.h"

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#endif

#include <iomanip>
#include <iostream>
#include <ostream>
#include <sstream>
#include <thread>
// Includes needed for _setmode() (+io.h)
#include <fcntl.h>

void initSteamAPI(bool& verbose)
{
    if (verbose)
        spdlog::info("[ Start ] STEAM_INIT");

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

    if (!SteamAPI_Init()) {
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

    if (!SteamUser()->BLoggedOn()) {
        printError("Fatal Error", "Steam user not logged in. SteamUser()->BLoggedOn() returned false.\nPlease log in.");
        exit(1);
    }

    // TODO(my_username): setPersonaState(Invisible) 7

    if (verbose) {
        spdlog::info("[ End   ] STEAM_INIT");
    }
}

std::thread createCallbackThread(bool& running, bool& verbose)
{
    if (verbose) {
        spdlog::info("[ Start ] CallbackThread & Steam_RunCallbacks");
    }

    auto CallbackThread = std::thread([&running]() {
        while (running) {
            try {
                std::this_thread::sleep_for(std::chrono::milliseconds(CSGO_CLI_STEAM_CALLBACK_INTERVAL));
                SteamAPI_RunCallbacks();
            } catch (ExceptionHandler& e) {
                printError("Fatal Error", e.what());
                exit(1);
            }
        }
    });

    if (verbose) {
        spdlog::info("[ End   ] CallbackThread & Steam_RunCallbacks");
    }

    return CallbackThread;
}

void initGameClientConnection(DataObject& data, bool& verbose)
{
    if (verbose) {
        spdlog::info("[ Start ] Trying to establish a GameClient Connection");
    }

    bool result = false;
    try {
        // make sure we are connected to the GameClient
        if (verbose) {
            spdlog::info("          -> Requesting: GameClient Connection");
        }
        CSGOClient::GetInstance()->WaitForGameClientConnect();
        if (verbose) {
            spdlog::info("          -> Successful: GameClient connected!");
        }
        result = true;

        data.account_id         = SteamUser()->GetSteamID().GetAccountID();
        data.steam_id           = SteamUser()->GetSteamID().ConvertToUint64();
        data.steam_player_level = SteamUser()->GetPlayerSteamLevel();
        // this is a "const char*" UTF data narrowing to std::string
        data.playername = reinterpret_cast<char const *>(SteamFriends()->GetPersonaName());

        CSteamID clan_id = SteamFriends()->GetClanByIndex(0);
        data.clan_name   = SteamFriends()->GetClanName(clan_id);
        data.clan_tag    = SteamFriends()->GetClanTag(clan_id);
    } catch (ExceptionHandler& e) {
        printError("Fatal error", e.what());
        result = false;
    }

    if (!result) {
        printError("Fatal error", "GameClient could not connect.");
        exit(1);
    }

    if (verbose) {
        spdlog::info("[ End   ] Trying to establish a GameClient Connection");
    }
}

void exitIfGameIsRunning()
{
#ifdef _WIN32
    HWND test = FindWindowW(0, L"Counter-Strike: Global Offensive");
    if (test != NULL) {
        printError("Warning", "\nCS:GO is currently running.\nPlease close the game, before running this program.");
        exit(1);
    }
#endif
}

int main(int argc, char** argv)
{
    SetConsoleOutputCP(CP_UTF8);
    WinCliColors::enableConsoleColor(true);

    bool paramVerbose          = false;
    bool paramPrintUser        = false;
    bool paramPrintMatches     = false;
    bool paramPrintScoreboard  = false;
    bool paramUploadShareCodes = false;
    bool paramUploadShareCode  = false;
    bool paramPrintGlobalStats = false;
    std::string shareCode;

    // default action
    if (argc <= 1) {
        printHelp();
        return 0;
    }

    for (int i = 1; i < argc; i = i + 1) {
        std::string option = argv[i];
        if (option == "-h" || option == "--h" || option == "-help" || option == "/?") {
            printHelp();
            return 0;
        } else if (option == "-V" || option == "--V" || option == "-version") {
            fmt::print(
                "{} version {}\n",
                WinCliColors::formatLightGreen(CSGO_CLI_BINARYNAME),
                WinCliColors::formatYellow(CSGO_CLI_VERSION));
            return 0;
        } else if (option == "-v" || option == "--v" || option == "-verbose") {
            paramVerbose = true;
        } else if (option == "-vv" || option == "--vv") {
            paramVerbose = true;
            spdlog::set_level(spdlog::level::debug);
        } else if (option == "-globalstats") {
            paramPrintGlobalStats = true;
        } else if (option == "-matches") {
            paramPrintMatches = true;
        } else if (option == "-scoreboard") {
            paramPrintScoreboard = true;
        } else if (option == "-user") {
            paramPrintUser = true;
        } else if (option == "-upload") {
            paramPrintMatches     = true;
            paramUploadShareCodes = true;
        } else if (option == "-sharecode" || option == "-s") {
            paramUploadShareCode = true;
            shareCode            = argv[i + 1];
            i++;
        } else if (option != "") {
            printError("ERROR (invalid argument)", option.c_str());
            fmt::print("Please check: '{} -help'\n", CSGO_CLI_BINARYNAME);
            return 1;
        }
    }

    if (paramVerbose && !paramPrintUser && !paramPrintGlobalStats && !paramPrintMatches && !paramPrintScoreboard &&
        !paramUploadShareCode && !paramUploadShareCodes) {
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
        if (!requestPlayersRankInfo(data, paramVerbose)) {
            printError("Error", "Steam did not respond in time. Could not print -user.");
            exit(1);
        }
    }

    if (paramPrintGlobalStats) {
        if (!requestGlobalStats(data, paramVerbose)) {
            printError("Error", "Steam did not respond in time. Could not print -globalstats.");
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

    if (paramPrintGlobalStats) {
        printGlobalStats(data);
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

    return EXIT_SUCCESS;
}
