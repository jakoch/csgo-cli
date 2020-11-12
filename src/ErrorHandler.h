#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H

#include <iostream>
#include <spdlog/spdlog.h>

static inline void printError(const char *title, const char *text)
{
    fprintf(stdout, "\x1B[91m%s:\033[0m %s\n", title, text);
}

#endif