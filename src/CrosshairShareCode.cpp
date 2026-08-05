// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "CrosshairShareCode.h"

#include "ExceptionHandler.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fmt/format.h>
#include <numeric>
#include <string>

namespace
{
    char const kDictionary[] = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefhijkmnopqrstuvwxyz23456789";

    int uint8ToInt8(int value)
    { return (value << 24) >> 24; }

    std::array<uint8_t, 18> shareCodeToBytes(std::string shareCode)
    {
        shareCode.erase(0, 5); // remove CSGO-
        shareCode.erase(std::remove(shareCode.begin(), shareCode.end(), '-'), shareCode.end());

        if (shareCode.size() != 25) {
            throw ExceptionHandler("Invalid crosshair share code.");
        }

        auto const invalidChar = std::find_if(shareCode.begin(), shareCode.end(), [](char c) {
            return std::strchr(kDictionary, c) == nullptr;
        });
        if (invalidChar != shareCode.end()) {
            throw ExceptionHandler("Invalid crosshair share code.");
        }

        std::reverse(shareCode.begin(), shareCode.end());

        std::array<uint8_t, 18> result = {};

        for (char c : shareCode) {
            std::array<uint8_t, 18> tmp = {};
            int addValue                = static_cast<int>(std::strchr(kDictionary, c) - kDictionary);
            int carry                   = 0;
            int value;

            for (int t = 17; t >= 0; --t) {
                carry = 0;
                for (int s = t; s >= 0; --s) {
                    value = (t == s) ? tmp[s] + result[t] * 57 : 0;
                    value += carry;
                    carry  = value >> 8;
                    tmp[s] = static_cast<uint8_t>(value & 0xFF);
                }
            }

            result = tmp;
            carry  = 0;

            for (int t = 17; t >= 0; --t) {
                value = (t == 17) ? result[t] + addValue : result[t];
                value += carry;
                carry     = value >> 8;
                result[t] = static_cast<uint8_t>(value & 0xFF);
            }
        }

        return result;
    }

    std::string formatNumber(double value)
    { return fmt::format("{:g}", value); }
} // namespace

CrosshairShareCodeInfo decodeCrosshairShareCode(std::string const & shareCode)
{
    auto bytes = shareCodeToBytes(shareCode);

    int checksum = std::accumulate(bytes.begin() + 1, bytes.end(), 0) & 0xFF;
    if (bytes[0] != checksum) {
        throw ExceptionHandler("Invalid crosshair share code checksum.");
    }

    CrosshairShareCodeInfo crosshair;
    crosshair.gap                      = static_cast<double>(uint8ToInt8(bytes[2])) / 10.0;
    crosshair.outline                  = static_cast<double>(bytes[3]) / 2.0;
    crosshair.red                      = bytes[4];
    crosshair.green                    = bytes[5];
    crosshair.blue                     = bytes[6];
    crosshair.alpha                    = bytes[7];
    crosshair.splitDistance            = bytes[8] & 7;
    crosshair.followRecoil             = ((bytes[8] >> 7) & 1) == 1;
    crosshair.fixedCrosshairGap        = static_cast<double>(uint8ToInt8(bytes[9])) / 10.0;
    crosshair.color                    = bytes[10] & 7;
    crosshair.outlineEnabled           = ((bytes[10] >> 3) & 1) == 1;
    crosshair.innerSplitAlpha          = static_cast<double>(bytes[10] >> 4) / 10.0;
    crosshair.outerSplitAlpha          = static_cast<double>(bytes[11] & 0x0F) / 10.0;
    crosshair.splitSizeRatio           = static_cast<double>(bytes[11] >> 4) / 10.0;
    crosshair.thickness                = static_cast<double>(bytes[12]) / 10.0;
    crosshair.centerDotEnabled         = ((bytes[13] >> 4) & 1) == 1;
    crosshair.deployedWeaponGapEnabled = ((bytes[13] >> 5) & 1) == 1;
    crosshair.alphaEnabled             = ((bytes[13] >> 6) & 1) == 1;
    crosshair.tStyleEnabled            = ((bytes[13] >> 7) & 1) == 1;
    crosshair.style                    = (bytes[13] & 0x0F) >> 1;
    crosshair.length                   = static_cast<double>(bytes[14]) / 10.0;

    return crosshair;
}

std::string crosshairShareCodeToConVars(CrosshairShareCodeInfo const & crosshair)
{
    return fmt::format(
        "cl_crosshair_drawoutline \"{}\"\n"
        "cl_crosshair_dynamic_maxdist_splitratio \"{}\"\n"
        "cl_crosshair_dynamic_splitalpha_innermod \"{}\"\n"
        "cl_crosshair_dynamic_splitalpha_outermod \"{}\"\n"
        "cl_crosshair_dynamic_splitdist \"{}\"\n"
        "cl_crosshair_outlinethickness \"{}\"\n"
        "cl_crosshair_t \"{}\"\n"
        "cl_crosshairalpha \"{}\"\n"
        "cl_crosshaircolor \"{}\"\n"
        "cl_crosshaircolor_b \"{}\"\n"
        "cl_crosshaircolor_g \"{}\"\n"
        "cl_crosshaircolor_r \"{}\"\n"
        "cl_crosshairdot \"{}\"\n"
        "cl_crosshairgap \"{}\"\n"
        "cl_crosshairgap_useweaponvalue \"{}\"\n"
        "cl_crosshairsize \"{}\"\n"
        "cl_crosshairstyle \"{}\"\n"
        "cl_crosshairthickness \"{}\"\n"
        "cl_crosshairusealpha \"{}\"\n"
        "cl_fixedcrosshairgap \"{}\"\n"
        "cl_crosshair_recoil \"{}\"\n",
        static_cast<int>(crosshair.outlineEnabled),
        formatNumber(crosshair.splitSizeRatio),
        formatNumber(crosshair.innerSplitAlpha),
        formatNumber(crosshair.outerSplitAlpha),
        crosshair.splitDistance,
        formatNumber(crosshair.outline),
        static_cast<int>(crosshair.tStyleEnabled),
        crosshair.alpha,
        crosshair.color,
        crosshair.blue,
        crosshair.green,
        crosshair.red,
        static_cast<int>(crosshair.centerDotEnabled),
        formatNumber(crosshair.gap),
        static_cast<int>(crosshair.deployedWeaponGapEnabled),
        formatNumber(crosshair.length),
        crosshair.style,
        formatNumber(crosshair.thickness),
        static_cast<int>(crosshair.alphaEnabled),
        formatNumber(crosshair.fixedCrosshairGap),
        static_cast<int>(crosshair.followRecoil));
}

void printCrosshairShareCodeInformation(std::string const & shareCode)
{
    auto const crosshair = decodeCrosshairShareCode(shareCode);

    auto const printAligned = [](std::string const & label, std::string const & value = "") {
        fmt::print(" {0:<26} {1}\n", label, value);
    };

    fmt::print("\n Crosshair Share Code:\n\n");
    printAligned("Share Code:", shareCode);
    printAligned("Length:", formatNumber(crosshair.length));
    printAligned("Gap:", formatNumber(crosshair.gap));
    printAligned("Outline:", formatNumber(crosshair.outline));
    printAligned("RGB:", fmt::format("{}, {}, {}", crosshair.red, crosshair.green, crosshair.blue));
    printAligned("Alpha:", std::to_string(crosshair.alpha));
    printAligned("Color:", std::to_string(crosshair.color));
    printAligned("Thickness:", formatNumber(crosshair.thickness));
    printAligned("Style:", std::to_string(crosshair.style));
    printAligned("Split Distance:", std::to_string(crosshair.splitDistance));
    printAligned("Fixed Gap:", formatNumber(crosshair.fixedCrosshairGap));
    printAligned("Inner Split Alpha:", formatNumber(crosshair.innerSplitAlpha));
    printAligned("Outer Split Alpha:", formatNumber(crosshair.outerSplitAlpha));
    printAligned("Split Size Ratio:", formatNumber(crosshair.splitSizeRatio));
    printAligned("Outline Enabled:", crosshair.outlineEnabled ? "yes" : "no");
    printAligned("Alpha Enabled:", crosshair.alphaEnabled ? "yes" : "no");
    printAligned("Center Dot Enabled:", crosshair.centerDotEnabled ? "yes" : "no");
    printAligned("T Style Enabled:", crosshair.tStyleEnabled ? "yes" : "no");
    printAligned("Weapon Gap Enabled:", crosshair.deployedWeaponGapEnabled ? "yes" : "no");
    printAligned("Follow Recoil:", crosshair.followRecoil ? "yes" : "no");

    fmt::print("\n Suggested ConVars:\n\n");
    fmt::print("{}", crosshairShareCodeToConVars(crosshair));
}
