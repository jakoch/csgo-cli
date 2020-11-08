#ifndef Table_H
#define Table_H

// common header for tabulate

#include <tabulate/table.hpp>
using namespace tabulate;

// MSVC std::variant sane constructor conversions are not implemented, yet
#if __cplusplus >= 201703L
#include <variant>
using std::variant;
#else
#include <nonstd/variant.hpp>
using nonstd::variant;
#endif

using row_t = std::vector<variant<std::string, Table>>;

#endif