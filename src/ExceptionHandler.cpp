#include "ExceptionHandler.h"

ExceptionHandler::ExceptionHandler(const std::string& what)
    :m_what(what)
{

}

const char* ExceptionHandler::what() const throw()
{
    return m_what.c_str();
}

