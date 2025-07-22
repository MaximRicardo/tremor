#include "bin_data.hpp"
#include <cstdint>

unsigned long long BinData::read_int(std::span<const uint8_t> data,
                                     unsigned n_bytes, size_t offset,
                                     bool little_endian)
{
    unsigned long long ret = 0;

    for (unsigned i = 0; i < n_bytes; ++i) {
        uint8_t byte = data[i + offset];

        if (little_endian)
            ret |= byte << (i * 8);
        else
            ret |= byte << ((n_bytes - i - 1) * 8);
    }

    return ret;
}
