#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

constexpr const std::uint8_t MAX_CHAR = 0b10000000;

int main() {
    //
    const std::string pattern("db");

    // Open the file 
    const std::string file_name("large1.rlb");
    std::ifstream file(file_name, std::ios::binary);

    std::uint8_t c = 0; // Buffer to read single byte
    std::uint8_t prev = 0; // Buffer to store previous byte 
    unsigned int count = 0; // Buffer to store count 
    std::vector<unsigned int> count_buff{};
    unsigned int word_count = 0; // The total number of character read 

    bwtsearch::IntCharArray B_S_array; // Store B and S entries in pairs. First element is count, second element is character. 

    // Read the bwt file and construct B_array and S_array 
    while (1)
    {
        file.read(reinterpret_cast<char*>(&c), sizeof(c));

        // Exit loop if EOF is reached 
        // Construct the final count before exiting the loop 
        if (file.eof()) {
            if (!count_buff.empty()) {
                for (unsigned int i = 0; i < count_buff.size(); i++) {
                    utility::unsetMsb(count_buff[i]);
                    unsigned int shifts = 7 * i;
                    count_buff[i] = count_buff[i] << shifts;
                    count += count_buff[i];
                }
                count += 3; // Starting from 3 
                B_S_array.back().first = count;
                word_count += count - 1;
                count = 0;
                count_buff.clear();
            }
            break;
        }

        if (c < MAX_CHAR) { // Current byte is a char 
            if (c == prev) {
                B_S_array.back().first += 1;
                word_count += 1;
            } else {
                // Construct the count if buffer is not empty 
                if (!count_buff.empty()) {
                    for (unsigned int i = 0; i < count_buff.size(); i++) {
                        utility::unsetMsb(count_buff[i]);
                        unsigned int shifts = 7 * i;
                        count_buff[i] = count_buff[i] << shifts;
                        count += count_buff[i];
                    }
                    count += 3; // Starting from 3 
                    B_S_array.back().first = count;
                    word_count += count - 1;
                    count = 0;
                    count_buff.clear();
                }

                // Insert a new entry 
                B_S_array.push_back({1, c});
                word_count += 1;
                prev = c;
            }
        } else { // Current byte is a count 
            count_buff.push_back(c);
        }  
    }
    file.close();
    
    // Construct the occurance map 
    unsigned int counter = 0;
    std::map<char, std::vector<unsigned int>> magic_map{};
    for (auto& element :B_S_array) {
        for (unsigned int j = 0; j < element.first; j++) {
            counter++;
            if (magic_map.find(element.second) == magic_map.end()) {
                std::vector<unsigned int> temp_vec{counter};
                magic_map.insert({element.second, std::move(temp_vec)});
            } else {
                magic_map[element.second].push_back(counter);
            }
        }
    }

    auto C_table = std::map<char, unsigned int>{};
    counter = 0;
    for (auto it = magic_map.begin(); it!= magic_map.end(); it++) {
        C_table.insert({it->first, counter});
        counter += it->second.size();
    }

    auto search_res = bwtsearch::search(magic_map, C_table, pattern, word_count);
    auto first = search_res.first;
    auto last = search_res.second;

    // TODO: should not be in the final version 
    // This is just for testing  
    std::vector<char> position_array{};
    for (auto& e : B_S_array) {
        for (unsigned int i = 0; i < e.first; i++) {
            position_array.push_back(e.second);
        }
    }

    std::set<bwtsearch::EntryIndex> entry{}; // Store the result index 
    std::map<bwtsearch::EntryIndex, bwtsearch::BwtIndex> entry_bwt_mapping{}; // Store the mapping of entry and index BWT 
    // For each index between first and last, we decode it until we get '[' 
    {
        std::set<bwtsearch::BwtIndex> seen_matching_idx{}; // Store the index of the last character of the pattern. 
        for (bwtsearch::BwtIndex idx = first; idx <= last; idx++) {
            auto current_idx = idx;
            char decode = 0;
            std::string msg;
            // Skip this index if it is seen 
            if (!seen_matching_idx.insert(current_idx).second) {
                continue;
            }
            while (decode != '[') {
                // decode = bwtsearch::at(B_S_array, current_idx);
                decode = position_array[current_idx - 1];
                // If the current character is the last target character, we check if it is seen before. 
                // If it is seen, we don't need to continue because it has been decoded once. 
                // If it is not seen, we continue to decode. 
                // IMPORTANT NOTE: We record the index of this matching in the B' table NOT in the B table. 
                if (decode == pattern.back()) {
                    // Find the index in the B' table 
                    if (!seen_matching_idx.insert(C_table[decode] + bwtsearch::occurance(current_idx, decode, magic_map)).second) {
                        break;
                    }
                }
                msg += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(current_idx, decode, magic_map);
                current_idx = starting_idx + occ;
            }
            // No need to record if the entry is not decoded completely. 
            if (msg.back() != '[') {
                continue;
            }
            // Record the entry. 
            std::reverse(msg.begin(), msg.end());
            auto num_str = std::string(msg.begin() += 1, std::find(msg.begin() += 1, msg.end(), ']')); // The string of the entry. 

            // Buffer the found entry.
            entry_bwt_mapping.insert({std::stoul(num_str), current_idx});

            if (entry.find(std::stoul(num_str)) != entry.end()) {
                std::cout << "Duplicate!" << std::endl;
            }

            entry.insert(std::stoul(num_str)); 
            // std::cout << "<" << num_str << ">" << std::endl;
        }
    }

    // Decode all entries
    for (auto& target_entry : entry) {
        auto temp = target_entry + 1; // The decoding should starting from the '[' of NEXT entry. 
        auto entry_idx_pair = entry_bwt_mapping.find(temp);
        if (entry_idx_pair != entry_bwt_mapping.end()) {
            auto current_idx = entry_idx_pair->second;
            char decode = 0;
            std::string msg;
            while (decode != '[') {
                decode = position_array[current_idx - 1];
                msg += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(current_idx, decode, magic_map);
                current_idx = starting_idx + occ;
            }
            std::reverse(msg.begin(), msg.end());
            std::cout << msg << std::endl;
        } else {
            std::string idx_pattern{'['};
            idx_pattern += std::to_string(temp);
            idx_pattern += ']';
            auto current_idx = bwtsearch::search(magic_map, C_table, idx_pattern, word_count).first;
            // Deal with the final entry
            if (current_idx == bwtsearch::NOT_FOUND) {
                temp -= magic_map['['].size();
                std::string idx_pattern{'['};
                idx_pattern += std::to_string(temp);
                idx_pattern += ']';
                current_idx = bwtsearch::search(magic_map, C_table, idx_pattern, word_count).first;
            }
            char decode = 0;
            std::string msg;
            while (decode != '[') {
                decode = position_array[current_idx - 1];
                msg += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(current_idx, decode, magic_map);
                current_idx = starting_idx + occ;
            }
            std::reverse(msg.begin(), msg.end());
            std::cout << msg << std::endl;
        }
        
    }

    return 0;
}