#include "utility.h"
#include "bwtsearch.h"
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <set>
#include <memory>

constexpr const unsigned char MAX_CHAR = 128;

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
        // printf("File size: %u\n", file_size);

        // Reset the file pointer
        file.seekg(0, std::ios::beg);

        unsigned char char_buff = 0; // Buffer to read single byte 
        unsigned char prev_char_buff = 0; // Buffer to store previous character 
        std::streampos prev_char_pos = 0; // Buffer to store the position of previous character 
        unsigned int count_assemble = 0; // Buffer to asseble the multi-byte count 
        auto count_buff = std::make_unique<std::vector<unsigned int>>(); // Buffer to store the multi-byte count 
        count_buff->reserve(3);

        auto counter = std::make_unique<Counter>();
        
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
                counter->count_char(prev_char_buff, count_assemble, prev_char_pos, index, count_buff->size() + 1);
                // printf("Read a character:%c, count:%u \n", prev_char_buff, count_assemble);
                count_assemble = 0;
                // Reset the count buffer 
                count_buff->clear();
            } else {
                counter->count_char(prev_char_buff, 1, prev_char_pos, index, 1);
                // printf("Read a character:%c \n", prev_char_buff);
            }
        }

        // Clean up the remaining RLB entry 
        if (counter->current_chunk_size > 0) {
            // output position to the index file 
            index.write(reinterpret_cast<char*>(&(counter->prev_file_pos)), sizeof(counter->prev_file_pos));
            // output the size of the block 
            index.write(reinterpret_cast<char*>(&(counter->current_chunk_size)), sizeof(counter->current_chunk_size));
            // ouput the current occurance table
            for (unsigned char i = 0; i < counter->occ_count.size(); i++) {
                index.write(reinterpret_cast<char*>(&(counter->occ_count[i])), sizeof(unsigned int));
            }
            printf("Index entry: %u, %u, Ending count: %u\n", counter->prev_file_pos, counter->current_chunk_size, counter->count);
        }

        // Write the count array into index file
        counter->count_array.push_back(counter->count);
        for (unsigned int i = 0; i < counter->count_array.size(); i++) {
            index.write(reinterpret_cast<char*>(&counter->count_array[i]), sizeof(unsigned int));
        }
        // for (unsigned int i = 0; i < counter->count_array.size(); i++) {
        //     printf("Block:%u  Ending count: %u\n", i, counter->count_array[i]);
        // }

        // Write the number of blocks to the index file
        unsigned int num_blocks = counter->count_array.size();
        index.write(reinterpret_cast<char*>(&num_blocks), sizeof(unsigned int));

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

    // Read the number of chunks from index file 
    unsigned int block_num = 0;
    index_read.read(reinterpret_cast<char*>(&block_num), sizeof(unsigned int));
    printf("Block num: %u\n", block_num);

    // Read the final occurance table from index file 
    auto final_occ_table = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));
    for (unsigned char i = 0; i < 128; i++) {
        index_read.read(reinterpret_cast<char*>(&final_occ_table[i]), sizeof(unsigned int));
        // printf("%c: %u\n", i, final_occ_table[i]);
    }

    // Read the count array from index file
    auto count_array = static_cast<unsigned int*>(calloc(block_num, sizeof(unsigned int)));
    index_read.clear();
    index_read.seekg((OVER_HEAD - static_cast<int>(block_num)) * sizeof(unsigned int), std::ios::end);
    for (unsigned int i = 0; i < block_num; i++) {
        index_read.read(reinterpret_cast<char*>(&count_array[i]), sizeof(unsigned int));
        // printf("Count array: %u\n", count_array[i]);
        // printf("Block:%u  Ending count: %u\n", i, count_array[i]);
    }

    // Construct C table 
    CTable* C_table = new CTable(final_occ_table);

    // Initialize the file buffer 
    FileBuffer file_buffer(std::move(file), std::move(index_read), count_array, block_num);

    // Test the file buffer
    file_buffer.read_block(2);

    // print C table
    // C_table->print();

    // Test occurance function 
    // for (unsigned int i = 0; i < 55; i++) {
    //     auto occ = bwtsearch::occurance(i, ']', file_buffer);
    //     printf("Occurance at position %u is %u times.\n", i, occ);
    // }

    // Test at function
    // auto output = std::ofstream("output.txt");
    // for (unsigned int i = 1; i <= 25435; i++) {
    //     auto c = bwtsearch::at(i,file_buffer);
    //     output.write(reinterpret_cast<char*>(&c), sizeof(char));
    //     printf("%c", c);
    // }
    // printf("\n");

    auto search_res = bwtsearch::search(C_table, pattern, file_buffer);
    auto first = search_res.first;
    auto last = search_res.second;
    printf("First: %u, Last: %u\n", first, last);

    std::set<bwtsearch::EntryIndex> entry{}; // Store the result index 
    // For each index between first and last, we decode it until we get '[' 
    {
        for (bwtsearch::BwtIndex idx = first; idx <= last; idx++) {
            auto current_idx = idx;
            char decode = 0;
            std::string msg;
            while (decode != '[') {
                decode = bwtsearch::at(current_idx, file_buffer);
                msg += decode;
                auto starting_idx = (*C_table)[C_table->getIndex(decode)].begin;
                auto occ = bwtsearch::occurance(current_idx, decode, file_buffer);
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
        auto current_idx = bwtsearch::search(C_table, idx_pattern, file_buffer).first;
        // Deal with the final entry
        if (current_idx == bwtsearch::NOT_FOUND) {
            temp -= final_occ_table[static_cast<unsigned int>('[')];
            std::string idx_pattern{'['};
            idx_pattern += std::to_string(temp);
            idx_pattern += ']';
            current_idx = bwtsearch::search(C_table, idx_pattern, file_buffer).first;
        }
        char decode = 0;
        std::string msg;
        while (decode != '[') {
            decode = bwtsearch::at(current_idx, file_buffer);
            msg += decode;
            auto starting_idx = (*C_table)[C_table->getIndex(decode)].begin;
            auto occ = bwtsearch::occurance(current_idx, decode, file_buffer);
            current_idx = starting_idx + occ;
        }
        std::reverse(msg.begin(), msg.end());
        printf("%s\n", msg.c_str());
    }

    return 0;
}