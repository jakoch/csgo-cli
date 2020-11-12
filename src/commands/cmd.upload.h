#ifndef CMD_UPLOAD_H
#define CMD_UPLOAD_H

#include "../DataObject.h"
#include "../ErrorHandler.h"
#include "../csgostats/ShareCodeCache.h"
#include "../csgostats/ShareCodeUpload.h"

static inline void uploadShareCode(std::string &sharecode, ShareCodeCache *matchCache, ShareCodeUpload *codeUpload);

void uploadReplayShareCodes(DataObject &data, bool &verbose);
void uploadSingleShareCode(std::string &sharecode, bool &verbose);

#endif
