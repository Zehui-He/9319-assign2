#pragma once
#include <fstream>

struct RlbEntry {
    unsigned char character;
    unsigned int count;

    RlbEntry(unsigned char c, unsigned int cnt) : character(c), count(cnt) {}
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

namespace utility
{
    // Unset the most significant bit of a byte in place 
    void unsetMsb(unsigned int&);

    // Read a single RLB entry from a file 
    RlbEntry readRlbEntry(std::ifstream&);

    bool fileExists(const std::string& filePath);
}
