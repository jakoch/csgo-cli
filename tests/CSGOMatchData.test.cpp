#include <catch2/catch_all.hpp>

#include "../src/csgo/CSGOMatchData.h"

TEST_CASE("[CSGOMatchData] getMapname returns map name from game_map when available", "[getMapname]")
{
    CSGOMatchData match;
    match.map = "de_dust2";
    match.map_id = 0;

    REQUIRE(match.getMapname() == "de_dust2");
}

TEST_CASE("[CSGOMatchData] getMapname resolves map name from map_id when game_map is empty", "[getMapname]")
{
    CSGOMatchData match;
    match.map = "";
    match.map_id = 520;  // de_dust2

    REQUIRE(match.getMapname() == "de_dust2");
}

TEST_CASE("[CSGOMatchData] getMapname resolves all known map IDs", "[getMapname]")
{
    struct TestCase {
        uint32 map_id;
        std::string expected;
    };

    TestCase cases[] = {
        {520,       "de_dust2"},
        {1032,      "de_train"},
        {4104,      "de_inferno"},
        {8200,      "de_nuke"},
        {16392,     "de_vertigo"},
        {65544,     "cs_office"},
        {32776,     "de_mirage"},
        {1048584,   "de_cache"},
        {67108872,  "de_workout"},
        {33554440,  "de_zoo"},
        {134217736, "cs_agency"},
        {268435464, "de_overpass"},
    };

    for (auto const& tc : cases) {
        CSGOMatchData match;
        match.map = "";
        match.map_id = tc.map_id;

        REQUIRE(match.getMapname() == tc.expected);
    }
}

TEST_CASE("[CSGOMatchData] getMapname returns '?' when both map and map_id are empty", "[getMapname]")
{
    CSGOMatchData match;
    match.map = "";
    match.map_id = 0;

    REQUIRE(match.getMapname() == "?");
}

TEST_CASE("[CSGOMatchData] getMapname prefers game_map over map_id", "[getMapname]")
{
    CSGOMatchData match;
    match.map = "de_inferno";
    match.map_id = 520;  // de_dust2

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
