#include "DateTimeUtils.h"

std::string getYear()
{
    auto now       = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y");
    return ss.str();
}

std::string getDateTime(const time_t &time, const char *time_format)
{
    std::stringstream ss;
    ss << std::put_time(localtime(&time), time_format);
    return ss.str();
}
