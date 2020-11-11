#ifndef CMD_USER_H
#define CMD_USER_H

#include "../DataObject.h"
#include "../VersionAndConstants.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../csgo/CSGOMMHello.h"
#include "../SteamId.h"

bool requestPlayersProfile(DataObject &data, bool &verbose);
void printPlayersProfile(DataObject &data);

#endif