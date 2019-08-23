#pragma once
#include <regex>
#include <ostream>
#include <iostream>
#include <intrin.h>

void quotientAndRemainder(
    uint64_t& a0, uint64_t& a1, uint16_t& a2,
    uint16_t m, uint16_t& r) {
    uint64_t q0 = 0, q1 = 0;
    uint16_t q2 = 0;
    const unsigned int ull_bitnum = sizeof(uint64_t) * 8;
    const unsigned int us_bitnum = sizeof(uint16_t) * 8;
    r = 0;
    for (int i = 2 * ull_bitnum + us_bitnum - 1; i >= 0; --i) {
        r <<= 1;
        bool a_ith_bit;
        if (i >= ull_bitnum + us_bitnum) {
            a_ith_bit = a0 & (1ull << (i - ull_bitnum - us_bitnum));
        } else if (i >= us_bitnum) {
            a_ith_bit = a1 & (1ull << (i - us_bitnum));
        } else {
            a_ith_bit = a2 & (1u << i);
        }
        r += a_ith_bit;
        if (r >= m) {
            r -= m;
            if (i >= ull_bitnum + us_bitnum) {
                q0 |= (1ull << (i - ull_bitnum - us_bitnum));
            } else if (i >= us_bitnum) {
                q1 |= (1ull << (i - us_bitnum));
            } else {
                q2 |= (1u << i);
            }
        }
    }
    a0 = q0; a1 = q1; a2 = q2;
}

/**
* Generate a demo share code from its object data and return its string representation.
* Required fields should come from a CDataGCCStrike15_v2_MatchInfo protobuf message.
* https://github.com/SteamRE/SteamKit/blob/master/Resources/Protobufs/csgo/cstrike15_gcmessages.proto#L773
*
* Steps to generate the share code:
*
* Required data from the protobuf message are:
* - uint64 match_id
* - uint64 reservationid from the last repeated entry in roundstatsall, or if that doesn’t exist from roundstats_legacy
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
std::string toDemoShareCode(uint64_t matchid, uint64_t reservationid, uint32_t tvport)
{
    // charset for base57
    std::string dictionary = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789";

    std::string code;
    uint64_t matchid_reversed = _byteswap_uint64(matchid);
    uint64_t reservationid_reversed = _byteswap_uint64(reservationid);
    uint16_t tvport_reversed = _byteswap_ushort(*(uint16_t*)(&tvport));
    uint16_t r = 0, dl = dictionary.length();

    for (int i = 0; i < 25; ++i) {
        quotientAndRemainder(matchid_reversed, reservationid_reversed, tvport_reversed, dl, r);
        code += dictionary[r];
        //std::cout << "i " << i << " r " << r << " code " << code << std::endl;
    }

    char shareCode[35]; // Char buffer for ShareCode

    // example: "CSGO-GADqf-jjyJ8-cSP2r-smZRo-TO2xK"
    sprintf(shareCode, "CSGO-%s-%s-%s-%s-%s\0",
        code.substr(0, 5).c_str(),
        code.substr(5, 5).c_str(),
        code.substr(10, 5).c_str(),
        code.substr(15, 5).c_str(),
        code.substr(20).c_str()
    );

    return shareCode;
}

/*
std::string fromDemoShareCode(std::string sharecode)
{
    // std::string sharecode("CSGO-U6MWi-hYFWJ-opPwD-JciHm-qOijD");
    // 3106049990460440633
    // 3106056003414655216
    // 11842

    sharecode = std::regex_replace(sharecode, std::regex("CSGO|-"), "");
    sharecode = std::string(sharecode.rbegin(), sharecode.rend());

    const std::string dictionary = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789";
    std::array<uint8_t, 18> result = {};

    for (char cur_char : sharecode)
    {
        std::array<uint8_t, 18> tmp = {};

        int addval = static_cast<int>(dictionary.find(cur_char));
        int carry = 0;
        int v = 0;

        for (int t = 17; t >= 0; t--) {
            carry = 0;
            for (int s = t; s >= 0; s--) {
                if (t - s == 0) {
                    v = tmp[s] + result[t] * 57;
                }
                else {
                    v = 0;
                }
                v = v + carry;
                carry = v >> 8;
                tmp[s] = v & 0xFF;
            }
        }

        result = tmp;
        carry = 0;

        for (int t = 17; t >= 0; t--) {
            if (t == 17) {
                v = result[t] + addval;
            }
            else {
                v = result[t];
            }
            v = v + carry;
            carry = v >> 8;
            result[t] = v & 0xFF;
        }
    }

    uint64_t matchid = *reinterpret_cast<uint64_t*>(result.data());
    uint64_t outcomeId = *reinterpret_cast<uint64_t*>(result.data() + 8);
    uint16_t tokenId = *reinterpret_cast<uint16_t*>(result.data() + 16);
}
*/