
#include <sstream>
#include <iostream>
#include "ccexce.h"

namespace CC
{
    Exce::Exce(int line, const char* file, const char* info) noexcept
        : _line(line), _file(file), _info(info) {}

    const char* Exce::what() const noexcept
    {
        std::ostringstream oss;
        oss << getType() << std::endl
            << getInfo();

        wharBuffer = oss.str();
        return wharBuffer.c_str();
    }

    const char* Exce::getType() const noexcept
    {
        return "APExce";
    }

    std::string Exce::getInfo() const noexcept
    {
        std::ostringstream oss;
        oss << "[EXCE]" << _info << std::endl
            << "[File]" << _file << std::endl
            << "[Line]" << _line;
        return oss.str();
    }
}