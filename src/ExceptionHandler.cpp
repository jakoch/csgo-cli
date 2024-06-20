// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ExceptionHandler.h"

ExceptionHandler::ExceptionHandler(std::string const & what) : m_what(what) { }

char const * ExceptionHandler::what() const throw()
{
    return m_what.c_str();
}
