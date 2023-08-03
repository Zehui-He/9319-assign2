#include "bwtsearch.h"
#include "utility.h"
#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <memory>

namespace bwtsearch
{
    unsigned int occurance(unsigned int position, char target, std::ifstream& index_read, std::ifstream& file, unsigned int chunk_size) {
        // Reset the file pointer
        index_read.clear();
        file.clear();
        // Shift the file pointer to the starting point of an entry 
        unsigned int entry_index = position / chunk_size - 1;
        unsigned int entry_offset = 0;
        if (entry_index != static_cast<unsigned int>(-1)) {
            entry_offset = 130 * entry_index * sizeof(unsigned int); // 130 is size of entry 
        }
        index_read.seekg(entry_offset, std::ios::beg);

        unsigned int rlb_pos = 0;
        unsigned int char_offset = 0;
        unsigned int target_occurance = 0;
        if (entry_index == static_cast<unsigned int>(-1)) {
            char_offset = 0;
            target_occurance = 0; // why????
        } else {
            // Read the RLB position 
            index_read.read(reinterpret_cast<char*>(&rlb_pos), sizeof(unsigned int));
            // Read the character off set 
            index_read.read(reinterpret_cast<char*>(&char_offset), sizeof(unsigned int));
            // Read the occurance table 
            // Can be optimized by reading only the target character 
            // Shift the file pointer to the target character 
            index_read.seekg(static_cast<unsigned int>(target) * sizeof(unsigned int), std::ios::cur);
            index_read.read(reinterpret_cast<char*>(&target_occurance), sizeof(unsigned int));
            // unsigned int occ_table[128] = { 0 };
            // for (unsigned int i = 0; i < 128; i++) {
            //     index_read.read(reinterpret_cast<char*>(&occ_table[i]), sizeof(unsigned int));
            // }
            // target_occurance = occ_table[static_cast<unsigned int>(target)];
        }

        // Set up the current position 
        unsigned int curr_postion = (entry_index + 1) * chunk_size;

        // Deal with the first RLB entry 
        file.seekg(rlb_pos, std::ios::beg);
        auto current_entry = utility::readRlbEntry(file);
        // The target position is within the range of the first RLB entry 
        if (curr_postion + current_entry.count - char_offset >= position) {
            if (current_entry.character == target) {
                return target_occurance + position - curr_postion;
            } else {
                return target_occurance;
            }
        } else { // The target position is not within the range of the first RLB entry
            if (current_entry.character == target) {
                target_occurance += current_entry.count - char_offset;
            }
            curr_postion += current_entry.count - char_offset;
        }

        // Read unitl the target position 
        while (curr_postion != position) {
            current_entry = utility::readRlbEntry(file);
            if (curr_postion + current_entry.count >= position) {
                if (current_entry.character == target) {
                    target_occurance += position - curr_postion;
                }
                break;
            } else {
                if (current_entry.character == target) {
                    target_occurance += current_entry.count;
                }
            }
            curr_postion += current_entry.count;
        }
        
        return target_occurance;
    }

    char at(unsigned int position, std::ifstream& index_read, std::ifstream& file, unsigned int chunk_size) {
        // Reset the file pointer
        index_read.clear();
        file.clear();
        // Shift the file pointer to the starting point of an entry 
        unsigned int entry_index = position / chunk_size - 1;
        unsigned int entry_offset = 0;
        if (entry_index != static_cast<unsigned int>(-1)) {
            entry_offset = 130 * entry_index * sizeof(unsigned int); // 130 is size of entry 
        }
        index_read.seekg(entry_offset, std::ios::beg);

        unsigned int rlb_pos = 0;
        unsigned int char_offset = 0;
        if (entry_index == static_cast<unsigned int>(-1)) {
            char_offset = 0; // Why????
        } else {
            // Read the RLB position 
            index_read.read(reinterpret_cast<char*>(&rlb_pos), sizeof(unsigned int));
            // Read the character off set 
            index_read.read(reinterpret_cast<char*>(&char_offset), sizeof(unsigned int));
        }

        // Set up the current position 
        unsigned int curr_postion = (entry_index + 1) * chunk_size;

        // Deal with the first RLB entry 
        file.seekg(rlb_pos, std::ios::beg);
        auto current_entry = utility::readRlbEntry(file);
        // The target position is within the range of the first RLB entry 
        if (curr_postion + current_entry.count - char_offset >= position) {
            return current_entry.character;
        } else { // The target position is not within the range of the first RLB entry
            curr_postion += current_entry.count - char_offset;
        }

        // Read unitl the target position 
        while (curr_postion != position) {
            current_entry = utility::readRlbEntry(file);
            if (curr_postion + current_entry.count >= position) {
                return current_entry.character;
            }
            curr_postion += current_entry.count;
        }
        
        return static_cast<char>(-1);
    }

    std::pair<BwtIndex, BwtIndex> search(CTable* C_table, std::string const& pattern, std::ifstream& file, std::ifstream& index_read, unsigned int chunk_size) {
        // Reset the file pointer
        index_read.clear();
        file.clear();
        
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
            auto occ_first = bwtsearch::occurance(first - 1, *pattern_it, index_read, file, chunk_size);

            // Calculate the occurance of current letter upto LAST 
            auto occ_last = 0;
            if (first == last) {
                occ_last = occ_first + 1;
            } else {
                occ_last = bwtsearch::occurance(last, *pattern_it, index_read, file, chunk_size);
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
