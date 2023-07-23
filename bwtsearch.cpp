#include "bwtsearch.h"
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <memory>

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

    unsigned int findLessThanOrEqual(unsigned int* sortedArray, unsigned int const& target, unsigned int const size) {
        unsigned int left = 0;
        unsigned right = size - 1;
        unsigned int resultIndex = size; // Initialize result index to size of the array

        // edge cases
        if(sortedArray[0] > target) {
            return 0;
        }

        if (sortedArray[size - 1] < target) {
            return size;
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

    unsigned int occurance(unsigned int position, char target, unsigned int ** magic_map, unsigned int* occ_count) {
        auto& temp = magic_map[(unsigned char)target];
        return findLessThanOrEqual(temp, position, occ_count[(unsigned char)target]);
    }

    unsigned int occurance2(IntCharArray const& pair_array, unsigned int position, char target) {
        unsigned int num_row = 0;
        unsigned int count = 0;
        for (auto& row : pair_array) {
            if (row.second == target) {
                count += row.first;
            }
            num_row += row.first;
            if (num_row >= position) {
                if (row.second == target) {
                    auto tmp = num_row - position;
                    count -= tmp;
                }
                return count;
            }
        }
        return 0;
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

    std::pair<BwtIndex, BwtIndex> search(unsigned int ** magic_map, unsigned int* C_table, std::string const& pattern, unsigned int const& word_count, unsigned int* occ_count) {
        auto pattern_it = pattern.rbegin();
        // Calculate the first and last for the first character 
        // Find the statring and ending point of the character in C table 
        // IMPORTANT NOTE: The first and last calculated here are 1-based index 
        // auto tmp = C_table.find(*pattern_it);
        // if (tmp == C_table.end()) {
        //     return {NOT_FOUND, NOT_FOUND};
        // }
        unsigned int first_idx = C_table[static_cast<unsigned int>(*pattern_it)];

        unsigned int first = 0;
        first = first_idx + 1;

        // tmp++;
        unsigned int last = 0;
        // Find next occuring character in C table
        unsigned char tmp = 0;
        for (unsigned int i = static_cast<unsigned int>(*pattern_it) + 1; i < 128; i++) {
            if (C_table[i] > 0) {
                tmp = i;
                break;
            }
        }

        if (tmp == 0) {
            last = word_count;
        } else {
            // last_idx = tmp->second;
            // last = bwtsearch::select(B_S_array, last_idx);
            last = C_table[static_cast<unsigned int>(tmp)] - 1 + 1;
            // last = tmp->second - 1 + 1;
        }

        pattern_it++;

        while (pattern_it != pattern.rend()) {
            // Exit if the current character doesn't in C table 
            if (occ_count[static_cast<unsigned int>(*pattern_it)] == 0) {
                return {NOT_FOUND, NOT_FOUND};
            }

            // C[current character]
            auto starting_idx = C_table[static_cast<unsigned int>(*pattern_it)];

            // std::cout << bwtsearch::occurance(B_S_array, first - 1, *pattern_it) << std::endl;
            // std::cout << bwtsearch::occurance(B_S_array, last, *pattern_it) << std::endl;

            // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
            auto occ_first = bwtsearch::occurance(first - 1, *pattern_it, magic_map, occ_count);

            // Calculate the occurance of current letter upto LAST 
            auto occ_last = 0;
            if (first == last) {
                occ_last = occ_first + 1;
            } else {
                occ_last = bwtsearch::occurance(last, *pattern_it, magic_map, occ_count);
            }

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
