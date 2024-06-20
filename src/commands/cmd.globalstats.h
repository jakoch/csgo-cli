// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_COMMANDS_CMD_GLOBALSTATS_H_
#define SRC_COMMANDS_CMD_GLOBALSTATS_H_

#include "../csgo/CSGOMMHello.h"
#include "../DataObject.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"
#include <string>

bool requestGlobalStats(DataObject& data, bool& verbose);
void printGlobalStats(DataObject& data);

#endif // SRC_COMMANDS_CMD_GLOBALSTATS_H_