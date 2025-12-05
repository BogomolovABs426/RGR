
#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>

using namespace std;

// Функции
extern "C" {
    uint64_t GenerateRandom(uint64_t min, uint64_t max);
    bool GenerateDESKey();
    bool GenerateRC2Key();
}

#endif
