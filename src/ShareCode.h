// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_SHARECODE_H_
#define SRC_SHARECODE_H_

#include <intrin.h>
#include <iostream>
#include <ostream>
#include <regex>
#include <string>

void quotientAndRemainder(uint64_t& a0, uint64_t& a1, uint16_t& a2, uint16_t m, uint16_t& r);

/**
 * Generate a demo share code from its object data and return its string representation.
 * Required fields should come from a CDataGCCStrike15_v2_MatchInfo protobuf message.
 * https://github.com/SteamDatabase/Protobufs/blob/3a112d8ece0baa8facf6b503cccb0ad6af826b8f/csgo/cstrike15_gcmessages.proto#L942
 *
 * Steps to generate the share code:
 *
 * Required data from the protobuf message are:
 * - uint64 match_id
 * - uint64 reservationid from the last repeated entry in roundstatsall, or if that doesn�t exist from roundstats_legacy
 * - uint16 low bits of uint32 tv_port
 *
 * 1. From these 3 items, we generate a big 144-bit number.
 * 2. The big number has to be run with a base57 encoding process
 *    against the string "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789".
 * 3. Construct ShareCode string
 *
 * @param matchId {Object|Long} matchId
 * @param reservationId {Object|Long} reservationId
 * @param tvPort number tvPort
 * @return {string} Share code as string
 */
std::string getShareCode(uint64_t matchid, uint64_t reservationid, uint32_t tvport);

/*
std::string fromDemoShareCode(std::string sharecode);
*/

#endif  // SRC_SHARECODE_H_