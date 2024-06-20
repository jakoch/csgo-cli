// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_STEAMID_H_
#define SRC_STEAMID_H_

#include <string>
#include <steam/steamtypes.h>
#include <sstream>
#include <cmath>

// 76561197960493477 -> 113874
uint32 calcSteamID32(uint64 steamID64);

// steam3ID: 76561197960493477 -> [U:1:227749]
std::string toSteamID32(uint64 steamID64);

// steamID32: STEAM_0:1:113874
std::string toSteamIDClassic(uint64 steamID64);

#endif // SRC_STEAMID_H_