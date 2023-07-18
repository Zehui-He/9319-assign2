#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <map>

constexpr const std::uint8_t MAX_CHAR = 0b10000000;


int main() {
    //
    const std::string pattern("industry");

    // Open the file 
    const std::string file_name("dummy.rlb");
    std::ifstream file(file_name, std::ios::binary);

    std::uint8_t c; // Buffer to read single byte
    std::uint8_t prev; // Buffer to store previous byte 
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

    for (auto& i :B_S_array) {
        for (unsigned int j = 0; j < i.first; j++) {
            std::cout << i.second << std::endl;
        }
    }

    auto pattern_it = pattern.rbegin();

    // Calculate the first and last for the first character 
    // Find the statring and ending point of the character in C table 
    // IMPORTANT NOTE: The first and last calculated here are 1-based index 
    auto tmp = C_table.find(*pattern_it);
    unsigned int first_idx = tmp->second;

    unsigned int first = 0;
    first = first_idx + 1;

    tmp++;
    unsigned int last;
    if (tmp == C_table.end()) {
        last = word_count;
    } else {
        // last_idx = tmp->second;
        // last = bwtsearch::select(B_S_array, last_idx);
        last = tmp->second - 1 + 1;
    }

    pattern_it++;

    std::cout << static_cast<int>(first) << std::endl;
    std::cout << static_cast<int>(last) << std::endl;

    while (pattern_it != pattern.rend()) {
        // C[current character]
        auto starting_idx = C_table[*pattern_it];

        std::cout << bwtsearch::occurance(B_S_array, first - 1, *pattern_it) << std::endl;
        std::cout << bwtsearch::occurance(B_S_array, last, *pattern_it) << std::endl;

        // Calculate the occurance of current letter upto the PREVIOUS row of FIRST 
        auto occ_first = bwtsearch::occurance(B_S_array, first - 1, *pattern_it);

        // Calculate the occurance of current letter upto LAST 
        auto occ_last = bwtsearch::occurance(B_S_array, last, *pattern_it);

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

        std::cout << static_cast<int>(first) << std::endl;
        std::cout << static_cast<int>(last) << std::endl;

        if (first > last) {
            std::cout << "Not found" << std::endl;
            break;
        }
    }

    return 0;
}