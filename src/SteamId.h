#ifndef SteamID_H
#define SteamID_H

#include <sstream>
#include <steam/steamtypes.h>

// 76561197960493477 -> 113874
uint32 calcSteamID32(uint64 steamID64);

// steam3ID: 76561197960493477 -> [U:1:227749]
std::string toSteamID32(uint64 steamID64);

// steamID32: STEAM_0:1:113874
std::string toSteamIDClassic(uint64 steamID64);

#endif