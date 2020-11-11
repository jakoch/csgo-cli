#ifndef CMD_MATCHES_H
#define CMD_MATCHES_H

#include "../DataObject.h"
#include "../VersionAndConstants.h"
#include "../ExceptionHandler.h"
#include "../ErrorHandler.h"
#include "../DateTimeUtils.h"
#include "../csgo/CSGOMatchList.h"
#include "../ShareCode.h"

#include <ranges>

bool requestRecentMatches(DataObject &data, bool &verbose);
void printMatches(DataObject &data);

#endif