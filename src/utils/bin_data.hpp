#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <span>

namespace BinData {

unsigned long long read_int(std::span<const uint8_t> data, unsigned n_bytes,
                            size_t offset, bool little_endian = true);

// DOES NOT WORK IF THE COMPILER STORES NUMBERS IN BID ENDIAN FORMAT!
// FIX LATER!
template <typename T>
T read_num(std::span<const uint8_t> data, size_t offset,
           bool little_endian = true)
{
    unsigned long long integer =
        read_int(data, sizeof(T), offset, little_endian);

    assert(sizeof(T) < sizeof(integer));

    std::array<uint8_t, sizeof(T)> bytes;
    for (size_t i = 0; i < sizeof(T); ++i) {
        bytes[i] = (integer >> (i * 8)) & 0xff;
    }

    T ret;
    memcpy(&ret, &integer, sizeof(T));

    return ret;
}

} // namespace BinData
