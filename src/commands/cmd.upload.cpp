// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cmd.upload.h"

#include <string>

static inline void uploadShareCode(std::string& sharecode, ShareCodeCache* matchCache, ShareCodeUpload* codeUpload)
{
    if (matchCache->find(sharecode)) {
        auto msg1 = fmt::format(fmt::fg(fmt::color::indian_red), "Skipped.");
        auto msg2 = fmt::format(fmt::fg(fmt::color::green), "The ShareCode \"{}\" was already uploaded.", sharecode);
        fmt::print(" {} {}\n", msg1, msg2);
        return;
    }

    std::string jsonResponse;

    fmt::print(" Uploading ShareCode: {}\n", WinCliColors::formatTerminalYellow(sharecode));

    if (codeUpload->uploadShareCode(sharecode, jsonResponse) == 0) {
        int upload_status = codeUpload->processJsonResponse(jsonResponse);

        if (upload_status == 4 || upload_status == 5) { // queued (in-progress) or complete
            matchCache->insert(sharecode);
        } else if (upload_status <= 3) {
            printError("Error", "Could not parse the response (to the replay sharecode POST request).");
        }

    } else {
        printError("Error", "Could not POST replay sharecode.");
    }
}

void uploadReplayShareCodes(DataObject& data, bool& verbose)
{
    if (!data.has_matches_played) {
        WinCliColors::printRed(" No replay sharecodes to upload.\n");
        return;
    }

    fmt::print(
        "\n Uploading Replay ShareCode{} to https://csgostats.gg/: \n\n", (data.num_matches_played == 1) ? "" : "s");

    ShareCodeCache* matchCache  = new ShareCodeCache(verbose);
    ShareCodeUpload* codeUpload = new ShareCodeUpload(verbose);

    for (auto& match : data.matches) {
        uploadShareCode(match.sharecode, matchCache, codeUpload);
    }
}

void uploadSingleShareCode(std::string& sharecode, bool& verbose)
{
    WinCliColors::printTerminalYellow("\n Uploading Single Replay ShareCode to https://csgostats.gg/: \n\n");

    ShareCodeCache* matchCache  = new ShareCodeCache(verbose);
    ShareCodeUpload* codeUpload = new ShareCodeUpload(verbose);

    uploadShareCode(sharecode, matchCache, codeUpload);
}