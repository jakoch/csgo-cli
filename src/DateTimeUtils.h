#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

std::string getYear();
std::string getDateTime(const time_t &time, const char *time_format = "%Y-%m-%d %H:%M:%S");
std::string format_duration_get_minutes(int milliseconds);

#endif