// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ExceptionHandler.h"

#include <string>
#include <utility>

ExceptionHandler::ExceptionHandler(std::string what) : m_what(std::move(what)) { }

char const * ExceptionHandler::what() const noexcept
{ return m_what.c_str(); }
