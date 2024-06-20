// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_ERRORHANDLER_H_
#define SRC_ERRORHANDLER_H_

#include <iostream>
#include <spdlog/spdlog.h>

static inline void printError(char const * title, char const * text)
{
    fprintf(stdout, "\x1B[91m%s:\033[0m %s\n", title, text);
}

#endif // SRC_ERRORHANDLER_H_