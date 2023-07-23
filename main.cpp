#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <memory>

constexpr const unsigned char MAX_CHAR = 0b10000000;

int main(int argc, char** argv) {
    auto pattern = argv[3];
    auto file_name = argv[1];

    // Open the file 
    std::ifstream file(file_name, std::ios::binary);

    unsigned char c = 0; // Buffer to read single byte
    unsigned char prev = 0; // Buffer to store previous byte 
    unsigned int count = 0; // Buffer to store count 
    auto count_buff = std::make_unique<std::vector<unsigned int>>();
    count_buff->reserve(3);
    unsigned int word_count = 0; // The total number of character read 

    std::vector<unsigned char> position_array; // Store B and S entries in pairs. First element is count, second element is character. 
    
    auto occ_count = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));; // Count the occurance of each character 

    // Read the bwt file and construct B_array and S_array 
    while (1)
    {
        file.read(reinterpret_cast<char*>(&c), sizeof(c));

        // Exit loop if EOF is reached 
        // Construct the final count before exiting the loop 
        if (file.eof()) {
            if (!count_buff->empty()) {
                    for (unsigned int i = 0; i < count_buff->size(); i++) {
                        utility::unsetMsb((*count_buff)[i]);
                        unsigned int shifts = 7 * i;
                        (*count_buff)[i] = (*count_buff)[i] << shifts;
                        count += (*count_buff)[i];
                    }
                    count += 3; // Starting from 3 
                    for (unsigned int i = 0; i < count - 1; i++) {
                        position_array.push_back(position_array.back());
                    }
                    word_count += count - 1;
                    occ_count[static_cast<unsigned int>(prev)] += count - 1;
                    count = 0;
                    count_buff->clear();
                }
            break;
        }

        if (c < MAX_CHAR) { // Current byte is a char 
            if (c == prev) {
                position_array.push_back(c);
                occ_count[static_cast<unsigned int>(c)] += 1;
                word_count += 1;
            } else {
                // Construct the count if buffer is not empty 
                if (!count_buff->empty()) {
                    for (unsigned int i = 0; i < count_buff->size(); i++) {
                        utility::unsetMsb((*count_buff)[i]);
                        unsigned int shifts = 7 * i;
                        (*count_buff)[i] = (*count_buff)[i] << shifts;
                        count += (*count_buff)[i];
                    }
                    count += 3; // Starting from 3 
                    for (unsigned int i = 0; i < count - 1; i++) {
                        position_array.push_back(position_array.back());
                    }
                    word_count += count - 1;
                    occ_count[static_cast<unsigned int>(prev)] += count - 1;
                    count = 0;
                    count_buff->clear();
                }

                // Insert a new entry 
                position_array.push_back(c);
                word_count += 1;
                occ_count[static_cast<unsigned int>(c)] += 1;
                prev = c;
            }
        } else { // Current byte is a count 
            count_buff->push_back(c);
        }  
    }
    file.close();
    position_array.shrink_to_fit();
    count_buff.reset();

    // Construct the occurance map 
    unsigned int counter = 0;
    unsigned int* magic_map[128];
    for(unsigned int i = 0; i < MAX_CHAR; i++){
        if (occ_count[i] > 0) {
            // allocate the array with 0 
            magic_map[i] = static_cast<unsigned int*>(calloc(occ_count[i], sizeof(unsigned int)));
        }
    }

    auto occ_fill_count = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));

    for (auto& row : position_array) {
        counter++;
        magic_map[static_cast<unsigned int>(row)][occ_fill_count[static_cast<unsigned int>(row)]] = counter;
        occ_fill_count[static_cast<unsigned int>(row)] += 1;
    }

    free(occ_fill_count);

    // print occurance map
    // for (unsigned int i = 0; i < 128; i++) {
    //     if (occ_count[i] > 0) {
    //         printf("%c: ", i);
    //         for (unsigned int j = 0; j < occ_count[i]; j++) {
    //             printf("%u ", magic_map[i][j]);
    //         }
    //         printf("\n");
    //     }
    // }

    // Construct C table 
    auto C_table = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));
    counter = 0;
    for (unsigned int i = 0; i < 128; i++) {
        if (occ_count[i] > 0) {
            C_table[i] = counter;
            counter += occ_count[i];
        }
    }

    // print C table
    // for (unsigned int i = 0; i < 128; i++) {
    //     if (occ_count[i] > 0) {
    //         printf("%c: %u\n", i, C_table[i]);
    //     }
    // }

    auto search_res = bwtsearch::search(magic_map, C_table, pattern, word_count, occ_count);
    auto first = search_res.first;
    auto last = search_res.second;

    std::set<bwtsearch::EntryIndex> entry{}; // Store the result index 
    // For each index between first and last, we decode it until we get '[' 
    {
        for (bwtsearch::BwtIndex idx = first; idx <= last; idx++) {
            auto current_idx = idx;
            char decode = 0;
            std::string msg;
            while (decode != '[') {
                // decode = bwtsearch::at(B_S_array, current_idx);
                decode = position_array[current_idx - 1];
                msg += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(current_idx, decode, magic_map, occ_count);
                current_idx = starting_idx + occ;
            }
            // Record the entry. 
            std::reverse(msg.begin(), msg.end());
            auto num_str = std::string(msg.begin() += 1, std::find(msg.begin() += 1, msg.end(), ']')); // The string of the entry. 

            entry.insert(std::stoul(num_str)); 
        }
    }

    // Decode all entries
    for (auto& target_entry : entry) {
        auto temp = target_entry + 1; // The decoding should starting from the '[' of NEXT entry. 
        std::string idx_pattern{'['};
        idx_pattern += std::to_string(temp);
        idx_pattern += ']';
        auto current_idx = bwtsearch::search(magic_map, C_table, idx_pattern, word_count, occ_count).first;
        // Deal with the final entry
        if (current_idx == bwtsearch::NOT_FOUND) {
            temp -= occ_count[static_cast<unsigned int>('[')];
            std::string idx_pattern{'['};
            idx_pattern += std::to_string(temp);
            idx_pattern += ']';
            current_idx = bwtsearch::search(magic_map, C_table, idx_pattern, word_count, occ_count).first;
        }
        char decode = 0;
        std::string msg;
        while (decode != '[') {
            decode = position_array[current_idx - 1];
            msg += decode;
            auto starting_idx = C_table[decode];
            auto occ = bwtsearch::occurance(current_idx, decode, magic_map, occ_count);
            current_idx = starting_idx + occ;
        }
        std::reverse(msg.begin(), msg.end());
        printf("%s\n", msg.c_str());
        
    }

    return 0;
}