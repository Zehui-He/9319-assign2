#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

namespace bwtsearch
{
    // Find the occurance of character c up to a given position in a B_S_array 
    uint8_t occurance(std::vector<std::pair<uint8_t, char>> const& B_S_array, char const& c,uint8_t const& position);
    
    // Return the element in C table 
    std::vector<std::pair<char, uint8_t>>::iterator findCtable(std::vector<std::pair<char, uint8_t>>& C_table, std::reverse_iterator<std::string::const_iterator>& str_it);
}
