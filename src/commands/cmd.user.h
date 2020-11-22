#ifndef CMD_USER_H
#define CMD_USER_H

#include "../DataObject.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../SteamId.h"
#include "../VersionAndConstants.h"
#include "../csgo/CSGOMMHello.h"
#include "../csgo/CSGORankUpdate.h"

bool requestPlayersProfile(DataObject &data, bool &verbose);
bool requestPlayersRankInfo(DataObject &data, bool &verbose);
void printPlayersProfile(DataObject &data);

#endif