// SPDX-FileCopyrightText: Copyright © 2018-present Jens A. Koch
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SRC_EXCEPTIONHANDLER_H_
#define SRC_EXCEPTIONHANDLER_H_

#include <exception>
#include <string>

class ExceptionHandler : public std::exception
{
public:
    explicit ExceptionHandler(std::string what);
    char const * what() const noexcept override;

private:
    std::string m_what;
};

#endif // SRC_EXCEPTIONHANDLER_H_
