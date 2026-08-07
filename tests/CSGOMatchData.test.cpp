// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_all.hpp>

#include "../src/csgo/CSGOMatchData.h"

#include <array>
#include <string>

TEST_CASE("[CSGOMatchData] getMapname returns map name from game_map when available", "[getMapname]")
{
    CSGOMatchData match;
    match.map    = "de_dust2";
    match.map_id = 0;

    REQUIRE(match.getMapname() == "de_dust2");
}

TEST_CASE("[CSGOMatchData] getMapname resolves map name from map_id when game_map is empty", "[getMapname]")
{
    CSGOMatchData match;
    match.map    = "";
    match.map_id = 520; // de_dust2

    REQUIRE(match.getMapname() == "de_dust2");
}

TEST_CASE("[CSGOMatchData] getMapname resolves all known map IDs", "[getMapname]")
{
    struct TestCase
    {
        uint32 map_id;
        std::string expected;
    };

    std::array<TestCase, 12> const cases{{
        {.map_id = 520, .expected = "de_dust2"},
        {.map_id = 1032, .expected = "de_train"},
        {.map_id = 4104, .expected = "de_inferno"},
        {.map_id = 8200, .expected = "de_nuke"},
        {.map_id = 16392, .expected = "de_vertigo"},
        {.map_id = 65544, .expected = "cs_office"},
        {.map_id = 32776, .expected = "de_mirage"},
        {.map_id = 1048584, .expected = "de_cache"},
        {.map_id = 67108872, .expected = "de_workout"},
        {.map_id = 33554440, .expected = "de_zoo"},
        {.map_id = 134217736, .expected = "cs_agency"},
        {.map_id = 268435464, .expected = "de_overpass"},
    }};

    for (auto const & tc : cases) {
        CSGOMatchData match;
        match.map    = "";
        match.map_id = tc.map_id;

        REQUIRE(match.getMapname() == tc.expected);
    }
}

TEST_CASE("[CSGOMatchData] getMapname returns '?' when both map and map_id are empty", "[getMapname]")
{
    CSGOMatchData match;
    match.map    = "";
    match.map_id = 0;

    REQUIRE(match.getMapname() == "?");
}

TEST_CASE("[CSGOMatchData] getMapname prefers game_map over map_id", "[getMapname]")
{
    CSGOMatchData match;
    match.map    = "de_inferno";
    match.map_id = 520; // de_dust2

    REQUIRE(match.getMapname() == "de_inferno");
}

TEST_CASE("[CSGOMatchData] sharecode field can be set and read", "[sharecode]")
{
    CSGOMatchData match;
    match.sharecode = "CSGO-AAAA-AAAA-AAAA-AAAA-AAAA";

    REQUIRE(match.sharecode == "CSGO-AAAA-AAAA-AAAA-AAAA-AAAA");
    REQUIRE(match.sharecode.starts_with("CSGO-"));
    REQUIRE(match.sharecode.length() > 10);
}
