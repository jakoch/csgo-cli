// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_COMMANDS_CMD_USER_H_
#define SRC_COMMANDS_CMD_USER_H_

#include "../csgo/CSGOMMHello.h"
#include "../csgo/CSGORankUpdate.h"
#include "../DataObject.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../SteamId.h"
#include "../VersionAndConstants.h"

#include <string>

bool requestPlayersProfile(DataObject& data, bool& verbose);
bool requestPlayersRankInfo(DataObject& data, bool& verbose);
void printPlayersProfile(DataObject& data);

#endif // SRC_COMMANDS_CMD_USER_H_