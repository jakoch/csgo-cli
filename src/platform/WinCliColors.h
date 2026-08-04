// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_PLATFORM_WINCLICOLORS_H_
#define SRC_PLATFORM_WINCLICOLORS_H_

#ifdef _WIN32
#include "./windows/WinCliColors.h"
#else
#include "./linux/WinCliColors.h"
#endif

#endif // SRC_PLATFORM_WINCLICOLORS_H_
