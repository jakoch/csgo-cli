#include "cmd.help.h"

void printHelp()
{
  std::cout << WinCliColors::formatLightGreen(CSGO_CLI_BINARYNAME)
      << " v" << WinCliColors::formatYellow(CSGO_CLI_VERSION)
      << ", " << CSGO_CLI_WEBSITE << "\n"
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