#pragma once
#include <cstdint>
#include <utility>

namespace utility
{
    void unsetMsb(uint8_t&);
    bool paircmp(std::pair<uint8_t, char> const&, std::pair<uint8_t, char> const&);
}
