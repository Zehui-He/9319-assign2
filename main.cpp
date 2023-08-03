#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <memory>

constexpr const unsigned char MAX_CHAR = 0b10000000;
// The first uint is the block size 
// The rest uint are the occurance table 
constexpr const int OVER_HEAD = -129; 
constexpr const unsigned int ENTRY_SIZE = 1 + 1 + 128; // The size of each entry in index file

class Counter {
public:
    Counter(unsigned int chunk_size) : CHUNK_SIZE(chunk_size) {
        count = 0;
        occ_count = std::vector<unsigned int>(128, 0);
    }
    
    unsigned int count;
    std::vector<unsigned int> occ_count;
    unsigned int CHUNK_SIZE;
    

    unsigned int& operator[](unsigned int idx) {
        return occ_count[idx];
    }

    // Count the occurance of a character 'c' by 'n' times 
    // If the count reaches the chunk size, write an entry to the index file 
    // The entry consists of the position of the current CHARACTER, the offset of current CHARACTER and the occurance table 
    void count_char(unsigned char c, unsigned int n, std::streampos char_pos, std::ofstream & index) {
        for (unsigned int i = 0; i < n; i++) {
            occ_count[static_cast<unsigned int>(c)] += 1;
            count += 1;
            if (count % CHUNK_SIZE == 0 && count != 0) {
                // print file position
                auto file_pos = static_cast<unsigned int>(char_pos);
                // output position to the index file 
                index.write(reinterpret_cast<char*>(&file_pos), sizeof(file_pos));
                // output offset to the index file
                auto temp = i + 1; // Note: 1-based index 
                index.write(reinterpret_cast<char*>(&temp), sizeof(i));
                for (unsigned char i = 0; i < occ_count.size(); i++) {
                    index.write(reinterpret_cast<char*>(&occ_count[i]), sizeof(unsigned int));
                }
            }
        }
    }

};

int main(int argc, char** argv) {
    auto pattern = argv[3];
    auto file_name = argv[1];

    const auto index_file = std::string("index.idx");

    // Open the input file 
    std::ifstream file(file_name, std::ios::binary);

    // If the index file exists, read the index file. 
    // Otherwise, construct the index file. 
    if (!utility::fileExists(index_file)) {
        std::ofstream index(index_file, std::ios::binary);
        // Get the total size of the file
        file.seekg(0, std::ios::end);
        const unsigned int file_size = static_cast<unsigned int>(file.tellg());
        printf("File size: %u\n", file_size);

        // Calculate the chunk size 
        // int chunk_num = (file_size / 4 - 129) / 130;
        // unsigned int chunk_size = file_size / chunk_num;
        unsigned int chunk_size = 1000;

        // Reset the file pointer
        file.seekg(0, std::ios::beg);

        unsigned char char_buff = 0; // Buffer to read single byte 
        unsigned char prev_char_buff = 0; // Buffer to store previous character 
        std::streampos prev_char_pos = 0; // Buffer to store the position of previous character 
        unsigned int count_assemble = 0; // Buffer to asseble the multi-byte count 
        auto count_buff = std::make_unique<std::vector<unsigned int>>(); // Buffer to store the multi-byte count 
        count_buff->reserve(3);

        auto counter = std::make_unique<Counter>(chunk_size);
        
        // Read the rlb file 
        while (file.get(reinterpret_cast<char&>(char_buff)))
        {
            // Exit if reaches the end of file
            if (file.eof()) {
                break;
            }

            prev_char_buff = char_buff;
            prev_char_pos = file.tellg();
            prev_char_pos -= 1;
            // Read until next character 
            while (static_cast<unsigned char>(file.peek()) >= MAX_CHAR) {
                if (file.eof()) {
                    break;
                }
                file.get(reinterpret_cast<char&>(char_buff));
                count_buff->push_back(char_buff);
            }

            if (count_buff->size() > 0) {
                // Assemble the count 
                for (unsigned int i = 0; i < count_buff->size(); i++) {
                    // Unset most significant bit
                    utility::unsetMsb((*count_buff)[i]);
                    count_assemble += (*count_buff)[i] << (7 * i);
                }
                count_assemble += 3; // Count statring from 3 
                counter->count_char(prev_char_buff, count_assemble, prev_char_pos, index);
                // printf("Read a character:%c, count:%u \n", prev_char_buff, count_assemble);
                count_assemble = 0;
                // Reset the count buffer 
                count_buff->clear();
            } else {
                counter->count_char(prev_char_buff, 1, prev_char_pos, index);
                // printf("Read a character:%c \n", prev_char_buff);
            }
        }

        // Write the chunk size into index file
        index.write(reinterpret_cast<char*>(&counter->CHUNK_SIZE), sizeof(unsigned int));

        // Write the final occurance table into index file 
        for (unsigned char i = 0; i < 128; i++) {
            index.write(reinterpret_cast<char*>(&counter->occ_count[i]), sizeof(unsigned int));
        }

        index.close();
    }

    // Open the index file
    std::ifstream index_read(index_file, std::ios::binary);

    // Shift the file pointer to the starting point of over head 
    index_read.seekg(OVER_HEAD * sizeof(unsigned int), std::ios::end);

    // Read the chunk size from index file 
    unsigned int chunk_size = 0;
    index_read.read(reinterpret_cast<char*>(&chunk_size), sizeof(unsigned int));
    printf("Block size: %u\n", chunk_size);

    // Read the final occurance table from index file 
    auto final_occ_table = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));
    for (unsigned char i = 0; i < 128; i++) {
        index_read.read(reinterpret_cast<char*>(&final_occ_table[i]), sizeof(unsigned int));
        // printf("%c: %u\n", i, final_occ_table[i]);
    }

    // Construct C table 
    CTable* C_table = new CTable(final_occ_table);

    // print C table
    // C_table->print();

    // Test occurance function 
    // auto occ = bwtsearch::occurance(1, '[', index_read, file, chunk_size);
    // printf("Occurance: %u\n", occ);

    // Test at function
    // auto output = std::ofstream("output.txt");
    // for (unsigned int i = 1; i <= 55; i++) {
    //     auto c = bwtsearch::at(i, index_read, file, chunk_size);
    //     output.write(reinterpret_cast<char*>(&c), sizeof(char));
    //     printf("%c", c);
    // }
    // printf("\n");
    // printf("%c\n", bwtsearch::at(9, index_read, file, chunk_size));

    auto search_res = bwtsearch::search(C_table, pattern, file, index_read, chunk_size);
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
                decode = bwtsearch::at(current_idx, index_read, file, chunk_size);
                msg += decode;
                auto starting_idx = (*C_table)[C_table->getIndex(decode)].begin;
                auto occ = bwtsearch::occurance(current_idx, decode, index_read, file, chunk_size);
                current_idx = starting_idx + occ;
            }
            // Record the entry. 
            std::reverse(msg.begin(), msg.end());
            // printf("Matching string: %s\n", msg.c_str());
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
        auto current_idx = bwtsearch::search(C_table, idx_pattern, file, index_read, chunk_size).first;
        // Deal with the final entry
        if (current_idx == bwtsearch::NOT_FOUND) {
            temp -= final_occ_table[static_cast<unsigned int>('[')];
            std::string idx_pattern{'['};
            idx_pattern += std::to_string(temp);
            idx_pattern += ']';
            current_idx = bwtsearch::search(C_table, idx_pattern, file, index_read, chunk_size).first;
        }
        char decode = 0;
        std::string msg;
        while (decode != '[') {
            decode = bwtsearch::at(current_idx, index_read, file, chunk_size);
            msg += decode;
            auto starting_idx = (*C_table)[C_table->getIndex(decode)].begin;
            auto occ = bwtsearch::occurance(current_idx, decode, index_read, file, chunk_size);
            current_idx = starting_idx + occ;
        }
        std::reverse(msg.begin(), msg.end());
        printf("%s\n", msg.c_str());

    }

    return 0;
}