// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_DATETIMEUTILS_H_
#define SRC_DATETIMEUTILS_H_

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

std::string getYear();
std::string getDateTime(time_t const & time, char const * time_format = "%Y-%m-%d %H:%M:%S");
std::string format_duration_get_minutes(int milliseconds);

#endif // SRC_DATETIMEUTILS_H_