#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

namespace bwtsearch
{
    using IntCharArray = std::vector<std::pair<unsigned int, char>>;

    // Return the number of new characters up to a given position. Returns 1-based index. 
    unsigned int rank(IntCharArray const&, unsigned int);

    // Return the position of when N new characters are found. 
    unsigned int select(IntCharArray const& , unsigned int);

    // Find the occurance of a character up to a given position. 
    unsigned int occurance(IntCharArray const&, unsigned int, char);
}
