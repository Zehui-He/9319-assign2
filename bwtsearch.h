#pragma once
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <fstream>
#include "utility.h"

namespace bwtsearch
{
    using IntCharArray = std::vector<std::pair<unsigned int, char>>;

    using EntryIndex = unsigned int;

    using BwtIndex = unsigned int;

    constexpr const unsigned int NOT_FOUND = -1;

    // Search a given pattern. Return a pair of {first, last}. 
    std::pair<BwtIndex, BwtIndex> search(CTable* C_table, std::string const& pattern, FileBuffer& file_buffer);

    // Find the occurance of a character up to a given position. 
    unsigned int occurance(unsigned int position, char target, FileBuffer& file_buffer);

    // Find the character at given position in the RLB file. 
    char at(unsigned int position, FileBuffer& file_buffer);
}
