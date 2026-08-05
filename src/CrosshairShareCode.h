// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_CROSSHAIRSHARECODE_H_
#define SRC_CROSSHAIRSHARECODE_H_

#include <cstdint>
#include <string>

struct CrosshairShareCodeInfo
{
    double length                 = 0.0;
    int red                       = 0;
    int green                     = 0;
    int blue                      = 0;
    double gap                    = 0.0;
    bool alphaEnabled             = false;
    int alpha                     = 0;
    bool outlineEnabled           = false;
    double outline                = 0.0;
    int color                     = 0;
    double thickness              = 0.0;
    bool centerDotEnabled         = false;
    int splitDistance             = 0;
    bool followRecoil             = false;
    double fixedCrosshairGap      = 0.0;
    double innerSplitAlpha        = 0.0;
    double outerSplitAlpha        = 0.0;
    double splitSizeRatio         = 0.0;
    bool tStyleEnabled            = false;
    bool deployedWeaponGapEnabled = false;
    int style                     = 0;
};

CrosshairShareCodeInfo decodeCrosshairShareCode(std::string const & shareCode);
std::string crosshairShareCodeToConVars(CrosshairShareCodeInfo const & crosshair);
void printCrosshairShareCodeInformation(std::string const & shareCode);

#endif // SRC_CROSSHAIRSHARECODE_H_
