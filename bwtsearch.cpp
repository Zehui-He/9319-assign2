#include "bwtsearch.h"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <string>

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

    unsigned int occurance(IntCharArray const& pair_array, unsigned int position, char target) {
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
}
