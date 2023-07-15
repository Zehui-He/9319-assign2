#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <unordered_map>

constexpr const std::uint8_t MAX_CHAR = 0b10000000;


int main() {
    //
    const std::string pattern("in");

    // Open the file 
    const std::string file_name("local.rlb");
    std::ifstream file(file_name, std::ios::binary);

    std::uint8_t c; // Buffer to read single byte
    std::uint8_t prev; // Buffer to store previous byte 
    std::uint8_t count = 0; // Buffer to store count 

    std::vector<std::pair<uint8_t, char>> B_S_array; // Store B and S entries in pairs. First element is count, second element is character. 

    // Read the bwt file and construct B_array and S_array 
    while (1)
    {
        file.read(reinterpret_cast<char*>(&c), sizeof(c));

        if (c < MAX_CHAR) { // Current byte is a char 
            if (c == prev) {
                B_S_array.back().first += 1;
            } else {
                B_S_array.push_back({1, c});
                prev = c;
            }
        } else { // Current byte is a count 
            utility::unsetMsb(c);
            count = c;
            count += 3;
            B_S_array.back().first = count;
            count = 0;
        }  

        // Exit loop if EOF is reached 
        if (file.eof()) {
            break;
        }
    }

    // Sort the B_S_array by S to get the B_prime_F_array
    auto B_prime_F_array = B_S_array;
    std::sort(B_prime_F_array.begin(), B_prime_F_array.end(), utility::paircmp);

    // Constuct C_table 
    auto C_table = std::vector<std::pair<char, uint8_t>>{};
    char curr = 0;
    for (size_t i = 0; i < B_prime_F_array.size(); i++) {
        if (B_prime_F_array[i].second != curr) {
            C_table.push_back({B_prime_F_array[i].second, i});
            curr = B_prime_F_array[i].second;
        }
    }

    for (auto& i :B_prime_F_array) {
        std::cout << static_cast<int>(i.first) << "  " << i.second << std::endl;
    }

    // for (auto& i :C_table) {
    //     std::cout << i.first << "  " << static_cast<int>(i.second) << std::endl;
    // }

    // for (auto& i :B_S_array) {
    //     std::cout << static_cast<size_t>(i.first) << "  " << i.second << std::endl;
    // }

    auto pattern_it = pattern.rbegin();

    // Calculate the first and last for the first character 
    auto tmp = bwtsearch::findCtable(C_table, pattern_it); 
    auto first = tmp->second;
    tmp++;
    auto last = tmp->second - 1;

    pattern_it++;


    while (pattern_it != pattern.rend()) {
        // Find starting index of currenct character in C table 
        auto starting_idx = bwtsearch::findCtable(C_table, pattern_it)->second;

        // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
        auto occ_first = 0;
        for (uint8_t i = 0; i <= first - 1; i++) {
            if (B_S_array[i].second == *pattern_it) {
                occ_first += B_S_array[i].first;
            }
        }

        // Calculate the occurance of current letter upto LAST 
        auto occ_last = 0;
        for (uint8_t i = 0; i < last; i++) {
            if (B_S_array[i].second == *pattern_it) {
                occ_last += B_S_array[i].first;
            }
        }
        
        // std::cout << static_cast<int>(occ_first) << std::endl;
        // std::cout << static_cast<int>(occ_last) << std::endl;

        // 
        first = starting_idx + occ_first;

        // Occurance of the last 
        last = starting_idx + occ_last;
        last -= 1;

        pattern_it++;
    }

    std::cout << static_cast<int>(first) << std::endl;
    std::cout << static_cast<int>(last) << std::endl;

    return 0;
}