// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_COMMANDS_CMD_MATCHES_H_
#define SRC_COMMANDS_CMD_MATCHES_H_

#include "../csgo/CSGOMatchList.h"
#include "../DataObject.h"
#include "../DateTimeUtils.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../ShareCode.h"
#include "../VersionAndConstants.h"
#include <string>

#include <ranges>

bool requestRecentMatches(DataObject& data, bool& verbose);
void printMatches(DataObject& data);

#endif  // SRC_COMMANDS_CMD_MATCHES_H_