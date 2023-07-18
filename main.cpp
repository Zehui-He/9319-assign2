#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_set>

constexpr const std::uint8_t MAX_CHAR = 0b10000000;


int main() {
    //
    const std::string pattern("a");

    // Open the file 
    const std::string file_name("small2.rlb");
    std::ifstream file(file_name, std::ios::binary);

    std::uint8_t c = 0; // Buffer to read single byte
    std::uint8_t prev = 0; // Buffer to store previous byte 
    unsigned int count = 0; // Buffer to store count 
    unsigned int word_count = 0; // The total number of character read 

    bwtsearch::IntCharArray B_S_array; // Store B and S entries in pairs. First element is count, second element is character. 

    // Read the bwt file and construct B_array and S_array 
    while (1)
    {
        file.read(reinterpret_cast<char*>(&c), sizeof(c));

        if (c < MAX_CHAR) { // Current byte is a char 
            if (c == prev) {
                B_S_array.back().first += 1;
                word_count += 1;
            } else {
                B_S_array.push_back({1, c});
                word_count += 1;
                prev = c;
            }
        } else { // Current byte is a count 
            utility::unsetMsb(c);
            count = c + 3; // Starting from 3 
            B_S_array.back().first = count;
            word_count += count - 1;
            count = 0;
        }  

        // Exit loop if EOF is reached 
        if (file.eof()) {
            B_S_array.back().first -= 1;
            word_count -= 1;
            break;
        }
    }

    // Sort the B_S_array by S to get the B_prime_F_array
    auto B_prime_F_array = B_S_array;
    std::sort(B_prime_F_array.begin(), B_prime_F_array.end(), utility::paircmp);

    // Constuct C table 
    // IMPORTANT NOTE: the C table is 0-based index 
    auto C_table = std::map<char, unsigned int>{};
    unsigned int sum = 0;
    char prev_char = 0;
    for (size_t i = 0; i < B_prime_F_array.size(); i++) {
        if (B_prime_F_array[i].second != prev_char) {
            C_table.insert({B_prime_F_array[i].second, sum});
            prev_char = B_prime_F_array[i].second;
        }
        sum += B_prime_F_array[i].first;
    }

    // for (auto& i :B_prime_F_array) {
    //     std::cout << static_cast<int>(i.first) << "  " << i.second << std::endl;
    // }

    // for (auto& i :C_table) {
    //     std::cout << i.first << "  " << static_cast<int>(i.second) << std::endl;
    // }

    // for (auto& i :B_S_array) {
    //     for (unsigned int j = 0; j < i.first; j++) {
    //         std::cout << i.second << std::endl;
    //     }
    // }

    auto pattern_it = pattern.rbegin();

    // Calculate the first and last for the first character 
    // Find the statring and ending point of the character in C table 
    // IMPORTANT NOTE: The first and last calculated here are 1-based index 
    auto tmp = C_table.find(*pattern_it);
    if (tmp == C_table.end()) {
        std::cout << "Not found" << std::endl;
        return 0;
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
        // C[current character]
        auto starting_idx = C_table[*pattern_it];
        // Why following linse are added? 
        // if (tmp == C_table.end()) {
        //     std::cout << "Not found" << std::endl;
        //     return 0;
        // }

        // std::cout << bwtsearch::occurance(B_S_array, first - 1, *pattern_it) << std::endl;
        // std::cout << bwtsearch::occurance(B_S_array, last, *pattern_it) << std::endl;

        // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
        auto occ_first = bwtsearch::occurance(B_S_array, first - 1, *pattern_it);

        // Calculate the occurance of current letter upto LAST 
        auto occ_last = bwtsearch::occurance(B_S_array, last, *pattern_it);

        if (occ_first == 0 && occ_last == 0) {
            std::cout << "Not found" << std::endl;
            return 0;
        }

        // Update first 
        first = starting_idx + occ_first;

        // Update last 
        if (B_S_array[bwtsearch::rank(B_S_array, last)].second == *pattern_it) { // Check if L[i]==c
            last = starting_idx + last - bwtsearch::select(B_S_array, bwtsearch::rank(B_S_array, last));
        } else {
            last = starting_idx + occ_last;
            last--; // We don't want to count the last row inclusively 
        }

        pattern_it++;

        // Index adjustment 
        first++;
        last++;

        // std::cout << static_cast<int>(first) << std::endl;
        // std::cout << static_cast<int>(last) << std::endl;

        if (first > last) {
            std::cout << "Not found" << std::endl;
            return 0;
        }
    }

    // Construct the entry mapping 
    // First is the entry, second is the associated index of '['
    std::map<unsigned int, unsigned int> entry_mapping{};
    std::unordered_set<unsigned int> entry_index{}; // Store all the index invoved with the entries
    {
        auto idx = C_table[']'] + 1; // 1-base adjustment 
        while (bwtsearch::at(B_prime_F_array, idx) == ']') {
            auto current_idx = idx;
            char decode = 0;
            std::string entry_num{};
            entry_index.insert(idx);
            while (decode != '[') {
                decode = bwtsearch::at(B_S_array, current_idx);
                entry_num += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(B_S_array, current_idx, decode);
                current_idx = starting_idx + occ;
                entry_index.insert(current_idx);
            }
            entry_num.pop_back(); // Remove the tailing '['
            std::reverse(entry_num.begin(), entry_num.end());
            entry_mapping.insert({std::stoul(entry_num), idx});
            // std::cout << entry_num << std::endl;
            idx++;
        }
    }

    // Decode from first to last 
    std::unordered_set<unsigned int> seen_idx{}; // Store the seen index 
    std::vector<unsigned int> final_idx{}; // Store the result index 
    {
        for (unsigned int idx = first; idx <= last; idx++) {
            auto current_idx = idx;
            char decode = 0;
            std::string msg;
            while (decode != '[') {
                // Mark the index as seen if it is not associated with entry 
                if (entry_index.find(current_idx) == entry_index.end()) {
                    // No need to continue for seen index 
                    if (!seen_idx.insert(current_idx).second) {
                        break;
                    }
                }
                decode = bwtsearch::at(B_S_array, current_idx);
                msg += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(B_S_array, current_idx, decode);
                current_idx = starting_idx + occ;
            }
            if (msg.back() != '[') {
                continue;
            }
            std::reverse(msg.begin(), msg.end());
            std::cout << msg;

            // Continue to decode from the back 
            auto num_str = std::string(msg.begin() += 1, std::find(msg.begin() += 1, msg.end(), ']'));
            auto entry_ptr = entry_mapping.find(std::stoul(num_str) + 1);
            if (entry_ptr == entry_mapping.end()) {
                current_idx = entry_mapping.begin()->second;
            } else {
                current_idx = entry_ptr->second;
            }
            decode = 0;
            auto msg_cont = std::string();
            while (seen_idx.find(current_idx) == seen_idx.end()) {
                if (entry_index.find(current_idx) == entry_index.end()) {
                    seen_idx.insert(current_idx);
                }
                decode = bwtsearch::at(B_S_array, current_idx);
                msg_cont += decode;
                auto starting_idx = C_table[decode];
                auto occ = bwtsearch::occurance(B_S_array, current_idx, decode);
                current_idx = starting_idx + occ;
            }

            // Remove tailing index 
            std::reverse(msg_cont.begin(), msg_cont.end());
            while (msg_cont.back() != '[') {
                msg_cont.pop_back();
            }
            msg_cont.pop_back();
            
            std::cout << msg_cont << std::endl;
        }
    }

    return 0;
}