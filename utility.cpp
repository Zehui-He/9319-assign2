#include "utility.h"
#include <iostream>

namespace utility {
    void unsetMsb(std::uint8_t& num) {
        std::uint8_t mask = 1 << 7;
        num &= ~mask;
    }

    bool paircmp(std::pair<uint8_t, char> const& a, std::pair<uint8_t, char> const& b) {
        return a.second < b.second;
    }

}
