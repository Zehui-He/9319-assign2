#include "utility.h"
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

namespace utility {

    auto count_buff = static_cast<unsigned int *>(calloc(3, sizeof(unsigned int))); // Buffer to store the multi-byte count 

    void unsetMsb(unsigned int& num) {
        unsigned int mask = 1 << 7;
        num &= ~mask;
    }

    bool fileExists(const std::string& filePath) {
        std::ifstream file(filePath);
        return file.good(); // Returns true if the file was successfully opened.
    }

    RlbEntry readRlbEntry(FileBuffer& file_buffer, unsigned int& buff_pos) {
        unsigned char char_buff = 0; // Buffer to read single byte 
        unsigned char prev_char_buff = 0; // Buffer to store previous character 
        unsigned int count_assemble = 0; // Buffer to asseble the multi-byte count 
        unsigned int count_num = 0;

        char_buff = file_buffer.buffer[buff_pos];
        buff_pos++;

        prev_char_buff = char_buff;
        // Read until next character 
        while (static_cast<unsigned char>(file_buffer.buffer[buff_pos]) >= 128) {
            if (buff_pos >= file_buffer.size) {
                break;
            }
            char_buff = file_buffer.buffer[buff_pos];
            count_buff[count_num] = char_buff;
            count_num++;
            buff_pos++;
        }

        if (count_num > 0) {
            // Assemble the count 
            for (unsigned int i = 0; i < count_num; i++) {
                // Unset most significant bit
                utility::unsetMsb(count_buff[i]);
                count_assemble += count_buff[i] << (7 * i);
            }
            count_assemble += 3; // Count statring from 3 
            // Clear the count buffer
            for (unsigned int i = 0; i < count_num; i++) {
                count_buff[i] = 0;
            }
            return RlbEntry(prev_char_buff, count_assemble);
        } else {
            return RlbEntry(prev_char_buff, 1);
        }

        return RlbEntry(-1, -1);
    }

}
