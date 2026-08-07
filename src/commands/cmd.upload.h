// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_COMMANDS_CMD_UPLOAD_H_
#define SRC_COMMANDS_CMD_UPLOAD_H_

#include "../csgostats/ShareCodeCache.h"
#include "../csgostats/ShareCodeUpload.h"
#include "../DataObject.h"
#include "../ErrorHandler.h"

#include <string>

void uploadReplayShareCodes(DataObject& data, bool const & verbose);
void uploadSingleShareCode(std::string& sharecode, bool const & verbose);

#endif // SRC_COMMANDS_CMD_UPLOAD_H_
