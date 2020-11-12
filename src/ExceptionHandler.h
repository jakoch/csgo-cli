#ifndef ExceptionHandler_H
#define ExceptionHandler_H

#include <exception>
#include <string>

class ExceptionHandler : public std::exception
{
public:
    ExceptionHandler(const std::string &what);
    virtual const char *what() const throw();

private:
    std::string m_what;
};

#endif
