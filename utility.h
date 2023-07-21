#pragma once
#include <cstdint>
#include <utility>

namespace utility
{
    void unsetMsb(unsigned int&);
    bool paircmp(std::pair<unsigned int, char> const&, std::pair<unsigned int, char> const&);
}
