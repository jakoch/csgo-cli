#ifndef CMD_GLOBALSTATS_H
#define CMD_GLOBALSTATS_H

#include "../DataObject.h"
#include "../ErrorHandler.h"
#include "../ExceptionHandler.h"
#include "../VersionAndConstants.h"
#include "../csgo/CSGOMMHello.h"

bool requestGlobalStats(DataObject &data, bool &verbose);
void printGlobalStats(DataObject &data);

#endif