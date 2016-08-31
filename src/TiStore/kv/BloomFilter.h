#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/Common.h"
#include "TiStore/kv/Hash.h"
#include "TiStore/lang/TypeInfo.h"

#include <string>
#include <stdio.h>

#define ALIGNED_TO(N, bytes)    (N)

namespace TiStore {

//
// N is the size of bitmap bits, N = kSizeOfTotalKeys * B (kBitsPerKey).
// B is the size of bits per key, B: kBitsPerKey.
// K is the number of the hash functions, K = B (kBitsPerKey) * 0.69 (~= ln 2).
//
template <std::size_t N, std::size_t B = 10, std::size_t K = 2>
class StandardBloomFilter {
private:
    // The size of one hash bitmap bits.
    static const std::size_t kSizeOfBitmap = ALIGNED_TO(N, CACHE_LINE_SIZE);
    // The length of one hash bitmap bits.
    static const std::size_t kBitsPerKey = B;
    // The kind of hash functions.
    static const std::size_t kNumProbes = K;

    std::size_t size_of_bitmap_;
    std::size_t bits_total_;
    std::size_t bits_per_key_;
    std::size_t num_probes_;

    std::unique_ptr<unsigned char> bitmap_;

public:
    StandardBloomFilter()
        : size_of_bitmap_(kSizeOfBitmap), bits_per_key_(kBitsPerKey),
          num_probes_(kNumProbes) {
        initBloomFilter();
    }
    ~StandardBloomFilter() {}

    void initBloomFilter() noexcept {
        bits_total_ = ((kSizeOfBitmap + (CACHE_LINE_SIZE - 1)) / CACHE_LINE_SIZE) * CACHE_LINE_SIZE;
        bits_per_key_ = kBitsPerKey;
        num_probes_ = static_cast<std::size_t>(kBitsPerKey * 0.69);

        unsigned char * new_bitmap = new (std::nothrow_t) unsigned char [bits_total_ * num_probes_];
        if (new_bitmap)
            bitmap_->reset(new_bitmap);
        else
            bitmap_->reset(nullptr);
    }

    inline bitIsInsideBitmap(std::uint32_t probes, std::uint32_t bit_pos)

    uint32_t keyMayMatch(const Slice & key) {
        bool isMatch = false;
        uint32_t primary_hash = BloomFilterHash<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        uint32_t bit_pos = primary_hash % bits_total_;
        bitIsInsideBitmap(0, bit_pos);
        if (num_probes_ > 1) {
            uint32_t secondary_hash;
            secondary_hash = BloomFilterHash<std::uint32_t>::secondaryHash(key.data(), key.size());
        }
        else {
            //
        }
        return hash;
    }

    uint32_t maybe_match(const Slice & key) {
        uint32_t hash = BloomFilterHash<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
#if 0
        printf("key  = %s\n", key.data());
        printf("hash = %11u (0x%08X)\n", hash, hash);
        printf("\n");
#endif
        return hash;
    }

    uint32_t maybe_match2(const Slice & key) {
        uint32_t hash = BloomFilterHash<std::uint32_t>::secondaryHash(key.data(), key.size());
        return hash;
    }

    uint32_t maybe_match_openssl(const Slice & key) {
        uint32_t hash = BloomFilterHash<std::uint32_t>::OpenSSLHash(key.data(), key.size());
        return hash;
    }

    uint32_t rocksdb_maybe_match(const Slice & key) {
        uint32_t hash = rocksdb::hash::Hash(key.data(), key.size(), kDefaultHashSeed32);
        return hash;
    }
};

class BloomFilter {
private:
    std::string name_;
    std::string root_;
    std::size_t capacity_;
    std::size_t block_size_;

public:
    BloomFilter() {};
    BloomFilter(const char * name) : name_(name), root_(""),
        capacity_(0), block_size_(0) {}
    virtual ~BloomFilter() {}

    bool mount() {
        return true;
    }
};

} // namespace TiStore
