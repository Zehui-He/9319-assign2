#include "bwtsearch.h"
#include "utility.h"
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <memory>

namespace bwtsearch
{
    unsigned int occurance(unsigned int position, char target, FileBuffer& file_buffer) {
        // Read the block accordingly 
        file_buffer.read_block(position);

        unsigned int buffer_pos = 0; // Record the position of the buffer 

        // Set up the current position 
        unsigned int curr_position = file_buffer.count_starting_point;

        unsigned int target_occurance = file_buffer.occ_table[static_cast<unsigned int>(target)];
        
        // Read unitl the target position 
        while (curr_position != position) {
            auto current_entry = utility::readRlbEntry(file_buffer, buffer_pos);
            if (curr_position + current_entry.count >= position) {
                if (current_entry.character == target) {
                    target_occurance += position - curr_position;
                }
                break;
            } else {
                if (current_entry.character == target) {
                    target_occurance += current_entry.count;
                }
            }
            curr_position += current_entry.count;
        }
        
        return target_occurance;
    }

    char at(unsigned int position, FileBuffer& file_buffer) {
        // Read the block accordingly 
        file_buffer.read_block(position);

        unsigned int buffer_pos = 0; // Record the position of the buffer 

        // Set up the current position 
        unsigned int curr_position = file_buffer.count_starting_point;

        // auto current_entry = utility::readRlbEntry(file_buffer, buffer_pos);

        // Read unitl the target position 
        while (curr_position != position) {
            auto current_entry = utility::readRlbEntry(file_buffer, buffer_pos);
            if (curr_position + current_entry.count >= position) {
                return current_entry.character;
            }
            curr_position += current_entry.count;
        }
        
        return static_cast<char>(-1);
    }

    std::pair<BwtIndex, BwtIndex> search(CTable* C_table, std::string const& pattern, FileBuffer& file_buffer) {
        auto pattern_it = pattern.rbegin();
        // Calculate the first and last for the first character 
        // Find the statring and ending point of the character in C table 
        // IMPORTANT NOTE: The first and last calculated here are 1-based index 
        unsigned int first_idx = C_table->getIndex(*pattern_it);

        unsigned int first = 0;
        first = (*C_table)[first_idx].begin + 1;

        unsigned int last = 0;
        // Find next occuring character in C table
        if (first_idx + 1 == C_table->C_table_size) {
            last = C_table->total_char;
        } else {
            last = (*C_table)[first_idx + 1].begin;
        }

        pattern_it++;

        while (pattern_it != pattern.rend()) {
            // Exit if the current character doesn't in C table 
            if (C_table->getIndex(*pattern_it) == CTable::NOT_FOUND) {
                return {NOT_FOUND, NOT_FOUND};
            }

            // C[current character]
            // auto starting_idx = C_table[static_cast<unsigned int>(*pattern_it)];
            auto starting_idx = (*C_table)[C_table->getIndex(*pattern_it)].begin;

            // std::cout << bwtsearch::occurance(B_S_array, first - 1, *pattern_it) << std::endl;
            // std::cout << bwtsearch::occurance(B_S_array, last, *pattern_it) << std::endl;

            // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
            auto occ_first = bwtsearch::occurance(first - 1, *pattern_it, file_buffer);

            // Calculate the occurance of current letter upto LAST 
            auto occ_last = 0;
            if (first == last) {
                occ_last = occ_first + 1;
            } else {
                occ_last = bwtsearch::occurance(last, *pattern_it, file_buffer);
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
