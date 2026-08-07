// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_all.hpp>

#include "../src/ShareCode.h"

#include <regex>
#include <string>

TEST_CASE("[ShareCode] getShareCode returns valid format", "[getShareCode]")
{
    uint64_t const matchId       = 3106049990460440633ULL;
    uint64_t const reservationId = 3106056003414655216ULL;
    uint32_t const tvPort        = 11842;

    std::string const code = getShareCode(matchId, reservationId, tvPort);

    // Share codes have the format: CSGO-XXXXX-XXXXX-XXXXX-XXXXX-XXXXX
    // where each X group is 5 characters from the base57 charset.
    // Base57 charset: ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789
    // (excludes I, l, 0, and 1)
    std::regex const pattern(
        "^CSGO-[A-HJ-Za-km-z2-9]{5}-[A-HJ-Za-km-z2-9]{5}-[A-HJ-Za-km-z2-9]{5}-[A-HJ-Za-km-z2-9]{5}-[A-HJ-Za-km-z2-9]{5}"
        "$");
    REQUIRE(std::regex_match(code, pattern));
}

TEST_CASE("[ShareCode] getShareCode produces consistent output for same inputs", "[getShareCode]")
{
    uint64_t const matchId       = 12345ULL;
    uint64_t const reservationId = 67890ULL;
    uint32_t const tvPort        = 27015;

    std::string const code1 = getShareCode(matchId, reservationId, tvPort);
    std::string const code2 = getShareCode(matchId, reservationId, tvPort);

    REQUIRE(code1 == code2);
}

TEST_CASE("[ShareCode] getShareCode produces different output for different inputs", "[getShareCode]")
{
    std::string const code1 = getShareCode(111ULL, 222ULL, 333);
    std::string const code2 = getShareCode(999ULL, 888ULL, 777);

    REQUIRE(code1 != code2);
}
