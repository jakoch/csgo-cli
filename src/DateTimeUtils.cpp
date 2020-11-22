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

std::string format_duration_get_minutes(int msecs)
{
    using namespace std::chrono;
    auto ms = milliseconds(msecs);
    auto secs = duration_cast<seconds>(ms);
    ms -= duration_cast<milliseconds>(secs);
    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);
    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    // ss << hour.count() << " Hours : "
    // << mins.count() << " Minutes : "
    // << secs.count() << " Seconds : "
    // << ms.count()   << " Milliseconds";
    ss << mins.count() << "m " << secs.count() << "s";
    return ss.str();
}
