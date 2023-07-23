#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>

namespace bwtsearch
{
    using IntCharArray = std::vector<std::pair<unsigned int, char>>;

    using EntryIndex = unsigned int;

    using BwtIndex = unsigned int;

    constexpr const unsigned int NOT_FOUND = -1;

    // Search a given pattern. Return a pair of {first, last}. 
    std::pair<BwtIndex, BwtIndex> search(unsigned int **, unsigned int*, std::string const&, unsigned int const&, unsigned int*);

    // Return the number of new characters up to a given position. Returns 1-based index. 
    unsigned int rank(IntCharArray const&, unsigned int);

    // Return the position of when N new characters are found. 
    unsigned int select(IntCharArray const& , unsigned int);

    // Find the occurance of a character up to a given position. 
    unsigned int occurance(unsigned int position, char target, unsigned int ** magic_map, unsigned int* occ_count);

    // unsigned int occurance2(IntCharArray const& pair_array, unsigned int position, char target);

    // Find the character at given position. 
    char at(IntCharArray const&, unsigned int);
}
