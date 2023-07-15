#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

namespace bwtsearch
{
    uint8_t occurance(std::vector<std::pair<uint8_t, char>> const& B_S_array, char const& target,uint8_t const& row) {

        return 1;
    }

    std::vector<std::pair<char, uint8_t>>::iterator findCtable(std::vector<std::pair<char, uint8_t>>& C_table, std::reverse_iterator<std::string::const_iterator>& str_it) {
        auto res = std::find_if(C_table.begin(), C_table.end(), [str_it](std::pair<char, uint8_t> e){
            return e.first == *str_it;
        });
        return res;
    }

}
