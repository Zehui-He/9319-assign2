#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <limits>

namespace bwtsearch
{
    using IntCharArray = std::vector<std::pair<unsigned int, char>>;

    using EntryIndex = unsigned int;

    using BwtIndex = unsigned int;

    constexpr const unsigned int NOT_FOUND = std::numeric_limits<unsigned int>::max();

    // Search a given pattern. Return a pair of {first, last}. 
    std::pair<BwtIndex, BwtIndex> search(IntCharArray const&, std::map<char, std::vector<unsigned int>> const&, std::map<char, BwtIndex>&, std::string const&, unsigned int const&);

    // Return the number of new characters up to a given position. Returns 1-based index. 
    unsigned int rank(IntCharArray const&, unsigned int);

    // Return the position of when N new characters are found. 
    unsigned int select(IntCharArray const& , unsigned int);

    // Find the occurance of a character up to a given position. 
    unsigned int occurance(IntCharArray const&, unsigned int, char);

    // Find the occurance of a character up to a given position. 
    unsigned int occurance2(IntCharArray const&, unsigned int, char, std::map<char,std::vector<unsigned int>> const&);

    // Find the character at given position. 
    char at(IntCharArray const&, unsigned int);
}
