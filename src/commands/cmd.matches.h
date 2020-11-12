#ifndef CMD_MATCHES_H
#define CMD_MATCHES_H

#include "../DataObject.h"
#include "../DateTimeUtils.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../ShareCode.h"
#include "../VersionAndConstants.h"
#include "../csgo/CSGOMatchList.h"

#include <ranges>

bool requestRecentMatches(DataObject &data, bool &verbose);
void printMatches(DataObject &data);

#endif