#include "bwtsearch.h"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>
#include <map>

namespace bwtsearch
{
    unsigned int rank(IntCharArray const& pair_array, unsigned int position) {
        unsigned int num_rows = 0;
        for (unsigned int i = 0; i < pair_array.size(); i++) {
            num_rows += pair_array[i].first;
            if (position <= num_rows) {
                return i + 1; // Returning 1-based index 
            }
        }
        return 0;
    }

    unsigned int select(IntCharArray const& pair_array, unsigned int N) {
        unsigned int num_rows = 0;
        for (unsigned int i = 0; i < N - 1; i++) {
            num_rows += pair_array[i].first;
        }
        num_rows++;
        return num_rows;
    }

    unsigned int findLessThanOrEqual(std::vector<unsigned int> const& sortedArray, unsigned int const& target) {
        int left = 0;
        int right = sortedArray.size() - 1;
        unsigned int resultIndex = sortedArray.size(); // Initialize result index to size of the array

        // edge cases
        if(sortedArray[0] > target) {
            return 0;
        }

        if (sortedArray.back() < target) {
            return sortedArray.size();
        }

        while (left <= right) {
            int mid = left + (right - left) / 2;

            if (sortedArray[mid] <= target) {
                resultIndex = mid; // Update result index and continue searching right for a larger number
                left = mid + 1;
            } else {
                // If the element at mid is greater than the target, continue searching left
                right = mid - 1;
            }
        }

        return resultIndex + 1;
    }

    unsigned int occurance(unsigned int position, char target, std::map<char, std::vector<unsigned int>> const& magic_map) {
        auto temp = magic_map.find(target)->second;
        // auto count = 0;
        // if (position < temp[0]) {
        //     return 0;
        // }
        // else if (position > temp.back()) {
        //     return temp.size();
        // }

        // for (unsigned int i = 0; i < temp.size(); i++) {
        //     if (temp[i] <= position) {
        //         count++;
        //         continue;
        //     }
        //     break;
        // }
        // return count;
        return findLessThanOrEqual(temp, position);
    }

    char at(IntCharArray const& pair_array, unsigned int position) {
        unsigned int num_row = 0;
        for (auto& row : pair_array) {
            num_row += row.first;
            if (num_row >= position) {
                return row.second;
            }
        }
        return 0;
    }

    std::pair<BwtIndex, BwtIndex> search(std::map<char, std::vector<unsigned int>> const& magic_map, std::map<char, BwtIndex>& C_table, std::string const& pattern, unsigned int const& word_count) {
        auto pattern_it = pattern.rbegin();
        // Calculate the first and last for the first character 
        // Find the statring and ending point of the character in C table 
        // IMPORTANT NOTE: The first and last calculated here are 1-based index 
        auto tmp = C_table.find(*pattern_it);
        if (tmp == C_table.end()) {
            return {NOT_FOUND, NOT_FOUND};
        }
        unsigned int first_idx = tmp->second;

        unsigned int first = 0;
        first = first_idx + 1;

        tmp++;
        unsigned int last = 0;
        if (tmp == C_table.end()) {
            last = word_count;
        } else {
            // last_idx = tmp->second;
            // last = bwtsearch::select(B_S_array, last_idx);
            last = tmp->second - 1 + 1;
        }

        pattern_it++;

        // std::cout << static_cast<int>(first) << std::endl;
        // std::cout << static_cast<int>(last) << std::endl;

        while (pattern_it != pattern.rend()) {
            // Exit if the current character doesn't in C table 
            if (C_table.find(*pattern_it) == C_table.end()) {
                return {NOT_FOUND, NOT_FOUND};
            }

            // C[current character]
            auto starting_idx = C_table[*pattern_it];

            // std::cout << bwtsearch::occurance(B_S_array, first - 1, *pattern_it) << std::endl;
            // std::cout << bwtsearch::occurance(B_S_array, last, *pattern_it) << std::endl;

            // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
            auto occ_first = bwtsearch::occurance(first - 1, *pattern_it, magic_map);
            // auto occ_first2 = bwtsearch::occurance(B_S_array, first - 1, *pattern_it);

            // Calculate the occurance of current letter upto LAST 
            auto occ_last = bwtsearch::occurance(last, *pattern_it, magic_map);
            // auto occ_last2 = bwtsearch::occurance(B_S_array, last, *pattern_it);

            if (occ_first == 0 && occ_last == 0) {
                return {NOT_FOUND, NOT_FOUND};
            }

            // Update first 
            first = starting_idx + occ_first;

            // Update last 
            last = starting_idx + occ_last;
            last--; // We don't want to count the last row inclusively 

            pattern_it++;

            // Index adjustment 
            first++;
            last++;

            if (first > last) {
                return {NOT_FOUND, NOT_FOUND};
            }
        }
        return {first, last};
    }

}
