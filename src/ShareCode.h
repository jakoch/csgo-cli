#pragma once

// for GMP
extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; }

#include <regex>
#include <ostream>
#include <iostream>
#include "dependencies\mpir\include\mpir.h"

// combines uint32_t low- and highbytes to uint64_t
uint64_t combine(uint32_t lowbyte, uint32_t highbyte)
{
    return (((uint64_t)highbyte) << 32) | ((uint64_t)lowbyte);
}

// get ulonglong from mpz_t struct
unsigned long long mpz2ull(mpz_t z)
{
    unsigned long long result = 0;
    mpz_export(&result, 0, -1, sizeof result, 0, 0, z);
    return result;
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
    //uint32_t highbyte = 752192506;
    //uint32_t lowbyte = -2147483492;
    //uint64_t matchid = 3230642215713767580; //combine(lowbyte, highbyte);
    //uint32_t highbyte = 752193760;
    //uint32_t lowbyte = 143;
    //uint64_t reservationid = 3230647599455273103; //combine(lowbyte, highbyte);
    //uint32_t tvport = 599906796;

    uint8_t matchid_reversed[8];
    for (int i = 0; i < 8; ++i) {
        // forward order = result[i]
        // reversed order = result[7-i]
        matchid_reversed[7 - i] = uint8_t((matchid >> 8 * (7 - i)) & 0xFF);
    }
    uint8_t reservationid_reversed[8];
    for (int i = 0; i < 8; ++i) {
        // forward order = result[i]
        // reversed order = result[7-i]
        reservationid_reversed[7 - i] = uint8_t((reservationid >> 8 * (7 - i)) & 0xFF);
    }
    uint8_t tvport_reversed[8];
    for (int i = 0; i < 8; ++i) {
        // forward order = result[i]
        // reversed order = result[7-i]
        tvport_reversed[7 - i] = uint8_t((tvport >> 8 * (7 - i)) & 0xFF);
    }

    /*std::cout << matchid << std::endl;
    std::cout << reservationid << std::endl;
    std::cout << tvport << std::endl;
    std::cout << "---" << std::endl;
    std::cout << "Bytes: \n156,0,0,128,250,139,213,44,143,0,0,0,224,144,213,44,236,217" << std::endl;
    std::cout << "bytesHex: \n9c000080fa8bd52c8f000000e090d52cecd9" << std::endl;
    std::cout << "---" << std::endl;
    */

    uint8_t bytes[18];
    for (int i = 0; i < 8; ++i) {
        bytes[i] = matchid_reversed[i];
        //printf("%" PRIu8 "\n", matchid_reversed[i]);
    }
    for (int i = 0; i < 8; ++i) {
        bytes[i + 8] = reservationid_reversed[i];
        //printf("%" PRIu8 "\n", reservationid_reversed[i]);
    }
    for (int i = 0; i < 8; ++i) {
        bytes[i + 16] = tvport_reversed[i];
        //printf("%" PRIu8 "\n", tvport_reversed[i]);
    }

    /*for (int i = 0; i < 18; ++i) {
        printf("%" PRIu8 ",", bytes[i]);
    }*/

    // mpz_import?

    // bytesToHex
    std::stringstream hex_stream;
    for (int i = 0; i < 18; ++i) {
        hex_stream << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(bytes[i]);
    }
    const std::string hex_str = hex_stream.str();

    // hexToBigInt
    MP_INT bigInt; // dividend
    mpz_init(&bigInt);
    mpz_init_set_str(&bigInt, hex_str.c_str(), 16);
    mpz_out_str(stdout, 10, &bigInt);

    // charset for base57
    std::string dictionary = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789";
    unsigned long int dl = dictionary.length();

    std::string code;
    MP_INT remainder;
    mpz_init(&remainder);
    MP_INT quotient;
    mpz_init(&quotient);

    for (int i = 0; i < 25; ++i) {
        mpz_divmod_ui(&quotient, &remainder, &bigInt, dl);
        unsigned long long r = mpz2ull(&remainder);
        code += dictionary[r];  // floor?
        bigInt = quotient;
        //std::cout << "i " << i << " r " << r << " code " << code << std::endl;
    }
    //mpz_clear(&quotient);
    //mpz_clear(&remainder);
    //mpz_clear(&bigInt);

    char shareCode[34]; // Char buffer for ShareCode

    // example: "CSGO-GADqf-jjyJ8-cSP2r-smZRo-TO2xK"
    sprintf(shareCode, "CSGO-%s-%s-%s-%s-%s",
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