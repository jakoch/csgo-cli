// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_all.hpp>

#include "../src/CrosshairShareCode.h"
#include "../src/ExceptionHandler.h"

TEST_CASE("[CrosshairShareCode] decodes known sample", "[decodeCrosshairShareCode]")
{
    auto const crosshair = decodeCrosshairShareCode("CSGO-WsnnD-eHaMw-QNDf9-oxuDh-ydOUD");

    REQUIRE(crosshair.length == Catch::Approx(10.0));
    REQUIRE(crosshair.gap == Catch::Approx(-2.2));
    REQUIRE(crosshair.outline == Catch::Approx(1.0));
    REQUIRE(crosshair.red == 50);
    REQUIRE(crosshair.green == 250);
    REQUIRE(crosshair.blue == 50);
    REQUIRE(crosshair.alpha == 200);
    REQUIRE(crosshair.splitDistance == 3);
    REQUIRE(crosshair.followRecoil == true);
    REQUIRE(crosshair.fixedCrosshairGap == Catch::Approx(3.0));
    REQUIRE(crosshair.color == 1);
    REQUIRE(crosshair.outlineEnabled == true);
    REQUIRE(crosshair.innerSplitAlpha == Catch::Approx(0.0));
    REQUIRE(crosshair.outerSplitAlpha == Catch::Approx(1.0));
    REQUIRE(crosshair.splitSizeRatio == Catch::Approx(1.0));
    REQUIRE(crosshair.thickness == Catch::Approx(0.6));
    REQUIRE(crosshair.centerDotEnabled == false);
    REQUIRE(crosshair.deployedWeaponGapEnabled == true);
    REQUIRE(crosshair.alphaEnabled == true);
    REQUIRE(crosshair.tStyleEnabled == false);
    REQUIRE(crosshair.style == 2);
}

TEST_CASE("[CrosshairShareCode] rejects invalid code", "[decodeCrosshairShareCode]")
{ REQUIRE_THROWS_AS(decodeCrosshairShareCode("CSGO-11111-22222-33333-44444-55555"), ExceptionHandler); }
