#include "string.hpp"
#include <algorithm>

std::string String::str_tolower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(),
                   [](char c) { return std::tolower(c); });

    return str;
}
