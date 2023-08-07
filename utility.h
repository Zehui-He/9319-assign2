#pragma once
#include <fstream>
#include <vector>
#include <memory>

class FileBuffer;

struct RlbEntry {
    unsigned char character;
    unsigned int count;

    RlbEntry(unsigned char c, unsigned int cnt) : character(c), count(cnt) {}
};

namespace utility
{
    // Unset the most significant bit of a byte in place 
    void unsetMsb(unsigned int&);

    // Read a single RLB entry from a file 
    RlbEntry readRlbEntry(FileBuffer& file_buffer, unsigned int& buff_pos);

    bool fileExists(const std::string& filePath);
}

class BufferEntry {
public:
    unsigned int count;
    unsigned int buf_pos;
    unsigned int* occ_table;

    BufferEntry() {
        this->count = 0;
        this->occ_table = static_cast<unsigned int*>(calloc(128, sizeof(unsigned int)));
        buf_pos = 0;
    }

};


const unsigned int MEGA_BYTE = 1024000; 
// The first uint is the number of blocks 
// The rest uint are the occurance table 
constexpr const int OVER_HEAD = -129; 
constexpr const unsigned int ENTRY_SIZE = 1 + 1 + 128; // The size of each entry in index file

class FileBuffer {
public:
    char* buffer;
    unsigned int size;
    unsigned int count_starting_point;
    unsigned int* count_array;
    std::ifstream file;
    std::ifstream index_read;
    unsigned int block_num;
    unsigned int* occ_table;
    BufferEntry* buffer_entry_list;
    unsigned int buffer_entry_list_size;

    static unsigned int const None = -1;

    FileBuffer(std::ifstream&& file, std::ifstream&& index_read, unsigned int* count_array, unsigned int block_num) {
        this->block_num = block_num;
        this->count_array = count_array;
        this->file = std::move(file);
        this->index_read = std::move(index_read);
        occ_table = new unsigned int[128];
        buffer = static_cast<char*>(calloc(MEGA_BYTE, sizeof(char)));
        size = 0;
        count_starting_point = None;
        buffer_entry_list = new BufferEntry[10];
        buffer_entry_list_size = 0;
    }
    
    ~FileBuffer() {
        delete[] buffer;
        delete[] occ_table;
        delete[] buffer_entry_list;
    }

    // Read the file block accordingly and update the occurance table
    void read_block(unsigned int position) {
        // Reset the file pointer
        file.clear();
        index_read.clear();
        // Find the block containing the position 
        unsigned int block_idx = None;
        for (unsigned int i = 0; i < block_num; i++) {
            if (position <= count_array[i]) {
                block_idx = i - 1;
                break;
            }
        }
        block_idx++;
        // Find the file position and size of the block 
        unsigned int rlb_position = 0;
        unsigned int block_size = 0;
        if (block_idx == 0) {
            // Change block when necessary 
            if (count_starting_point == 0) {
                return;
            }
            count_starting_point = 0;
            rlb_position = 0;
            index_read.seekg(0 + sizeof(unsigned int), std::ios::beg);
            index_read.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
            for (unsigned char i = 0; i < 128; i++) {
                occ_table[i] = 0;
            }
        } else {
            // Change block when necessary 
            if (count_starting_point == count_array[block_idx - 1]) {
                return;
            }
            count_starting_point = count_array[block_idx - 1]; // The count starting point is the ending point of the previous block 
            index_read.seekg(block_idx * ENTRY_SIZE * sizeof(unsigned int), std::ios::beg);
            index_read.read(reinterpret_cast<char*>(&rlb_position), sizeof(rlb_position));
            index_read.read(reinterpret_cast<char*>(&block_size), sizeof(block_size));
            // Read the occurance table
            index_read.seekg(((block_idx - 1) * ENTRY_SIZE + 2) * sizeof(unsigned int), std::ios::beg);
            index_read.read(reinterpret_cast<char*>(occ_table), 128 * sizeof(unsigned int));
        }
        // Read the block
        // printf("Read block %u from position: %u with size: %u\n", block_idx ,rlb_position, block_size);
        // printf("Seeking position: %u, Current starting point: %u\n", position, count_starting_point);
        file.seekg(rlb_position, std::ios::beg);
        file.read(buffer, block_size);
        size = block_size;

        // create_buffer_entry();

        // Output the block 
        // auto output = std::ofstream("output.txt");
        // for (unsigned int i = 0; i < block_size; i++) {
        //     output.write(reinterpret_cast<char*>(&buffer[i]), sizeof(char));
        //     printf("%c", buffer[i]);
        // }
        // printf("\n");
    }

private:
    void create_buffer_entry() {
        unsigned int entry_idx = 0;
        // Copy the first entry 
        buffer_entry_list[entry_idx].count = count_starting_point;
        for (unsigned char i = 0; i < 128; i++) {
            buffer_entry_list[entry_idx].occ_table[i] = occ_table[i];
        }
        entry_idx++;
        // Read through the buffer 
        unsigned int buffer_spacing = size / 10;
        unsigned int buffer_pos = 0;
        unsigned int prev_buffer_pos = 0;
        while (buffer_pos < size) {
            auto current_entry = utility::readRlbEntry(*this, buffer_pos);
            count_starting_point += current_entry.count;
            occ_table[static_cast<unsigned int>(current_entry.character)] += current_entry.count;
            // Record if spacing is reached 
            if (buffer_pos - prev_buffer_pos >= buffer_spacing) {
                buffer_entry_list[entry_idx].count = count_starting_point;
                for (unsigned char i = 0; i < 128; i++) {
                    buffer_entry_list[entry_idx].occ_table[i] = occ_table[i];
                }
                entry_idx++;
                buffer_entry_list[entry_idx].buf_pos = buffer_pos;
                prev_buffer_pos = buffer_pos;
                if (entry_idx == 10) {
                    break;
                }
            }
        }
        buffer_entry_list_size = entry_idx;
        
        // Print the buffer entry
        for (unsigned int i = 0; i < buffer_entry_list_size; i++) {
            printf("Buffer entry %u: %u\n", i, buffer_entry_list[i].count);
            for (unsigned char j = 0; j < 128; j++) {
                if (buffer_entry_list[i].occ_table[j] > 0) {
                    printf("%c: %u\n", j, buffer_entry_list[i].occ_table[j]);
                }
            }
        }
    }
};

class Counter {
public:
    Counter() {
        count = 0;
        current_chunk_size = 0;
        count_array = std::vector<unsigned int>();
        occ_count = std::vector<unsigned int>(128, 0);
        prev_file_pos = 0;
    }
    
    unsigned int count;
    unsigned int current_chunk_size;
    std::vector<unsigned int> occ_count;
    std::vector<unsigned int> count_array;
    unsigned int prev_file_pos;
    

    unsigned int& operator[](unsigned int idx) {
        return occ_count[idx];
    }

    // Count the occurance of a character 'c' by 'n' times 
    // If the count reaches the chunk size, write an entry to the index file 
    // The entry consists of the position of the current CHARACTER, the offset of current CHARACTER and the occurance table 
    void count_char(unsigned char c, unsigned int n, std::streampos char_pos, std::ofstream & index, unsigned int rlb_entry_size) {
        count += n;
        for (unsigned int i = 0; i < n; i++) {
            occ_count[static_cast<unsigned int>(c)] += 1;
        }
        // Saperate the index file into chunks of APPROXIMATELY 1MB
        current_chunk_size += rlb_entry_size;
        // if (current_chunk_size >= 1000*1000) {
        if (current_chunk_size >= 500) {
            // output position to the index file 
            index.write(reinterpret_cast<char*>(&prev_file_pos), sizeof(prev_file_pos));
            // output the size of the block 
            index.write(reinterpret_cast<char*>(&current_chunk_size), sizeof(current_chunk_size));
            // ouput the current occurance table
            for (unsigned char i = 0; i < occ_count.size(); i++) {
                index.write(reinterpret_cast<char*>(&occ_count[i]), sizeof(unsigned int));
                if (occ_count[i] > 0) {
                    printf("%c: %u\n", i, occ_count[i]);
                }
            }
            printf("Index entry: %u, %u, Ending count: %u\n", prev_file_pos, current_chunk_size ,count);
            prev_file_pos += current_chunk_size;
            // Reset the counter
            count_array.push_back(count);
            current_chunk_size = 0;
        } else if (current_chunk_size > MEGA_BYTE) {
            printf("Error: current chunk size is larger than 1024\n");
            exit(1);
        }
    }
};

class CTable {
public:
    struct CTableEntry {
        unsigned char character;
        unsigned int begin;
    };

    static const unsigned int NOT_FOUND = -1;

    CTable(unsigned int* final_occ_table) {
        unsigned int num_char = 0;
        for (unsigned int i = 0; i < 128; i++) {
            if (final_occ_table[i] > 0) {
                num_char++;
            }
        }

        C_table_size = num_char;
        C_table = new CTableEntry[num_char];

        unsigned int sum = 0;
        unsigned int fill_idx = 0;
        for (unsigned int i = 0; i < 128; i++) {
            if (final_occ_table[i] > 0) {
                C_table[fill_idx] = CTableEntry{ static_cast<unsigned char>(i), sum };
                sum += final_occ_table[i];
                fill_idx++;
            }
        }

        total_char = sum;
    }

    ~CTable() {
        delete[] C_table;
    }

    void print() {
        for (unsigned int i = 0; i < C_table_size; i++) {
            printf("Character:%c, begin:%u\n", C_table[i].character, C_table[i].begin);
        }
    }

    // Get the begin position of a given character with binary search 
    unsigned int getIndex(unsigned char c) {
        unsigned int left = 0;
        unsigned int right = C_table_size - 1;
        unsigned int mid = 0;

        while (left <= right) {
            mid = (left + right) / 2;
            if (C_table[mid].character == c) {
                return mid;
            } else if (C_table[mid].character < c) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }

        return NOT_FOUND;
    }

    CTableEntry& operator[] (unsigned int idx) {
        return C_table[idx];
    }

    CTableEntry* C_table;
    unsigned int C_table_size;
    unsigned int total_char;
};
