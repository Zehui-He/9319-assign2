#include "utility.h"
#include <iostream>

namespace utility {
    void unsetMsb(unsigned int& num) {
        unsigned int mask = 1 << 7;
        num &= ~mask;
    }

    bool paircmp(std::pair<unsigned int, char> const& a, std::pair<unsigned int, char> const& b) {
        return a.second < b.second;
    }

}
