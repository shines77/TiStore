#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/kv/Hash.h"

#include <string>
#include <stdio.h>

#define aligned_to(N, bytes)    (N)

namespace TiStore {

template <std::size_t N, std::size_t K>
class StandardBloomFilter {
private:
    // The length of one hash bitmap bits.
    static const std::size_t kLenOfBitmap = aligned_to(N, CACHE_LINE_SIZE);
    // The kind of hash functions.
    static const std::size_t kKindOfHash = K;

    std::size_t lenOfBitmap_;
    std::size_t kindOfHash_;

public:
    StandardBloomFilter() : lenOfBitmap_(kLenOfBitmap), kindOfHash_(kKindOfHash) {
    }
    ~StandardBloomFilter() {}

    uint32_t maybe_match(const Slice & key) {
        uint32_t hash = BloomFilterHash<std::uint32_t>::PrimeHash(key.data(), key.size(), kDefaultHashSeed);
        //printf("key  = %s\n", key.data());
        //printf("hash = %11u (0x%08X)\n", hash, hash);
        //printf("\n");
        return hash;
    }

    uint32_t maybe_match2(const Slice & key) {
        uint32_t hash = BloomFilterHash<std::uint32_t>::SecondaryHash(key.data(), key.size());
        //printf("key  = %s\n", key.data());
        //printf("hash = %11u (0x%08X)\n", hash, hash);
        //printf("\n");
        return hash;
    }

    uint32_t rocksdb_maybe_match(const Slice & key) {
        uint32_t hash = rocksdb::hash::Hash(key.data(), key.size(), kDefaultHashSeed32);
        //printf("key  = %s\n", key.data());
        //printf("hash = %11u (0x%08X)\n", hash, hash);
        //printf("\n");
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
    BloomFilter(const char * name) : name_(name), root_(""),
        capacity_(0), block_size_(0) {}
    virtual ~BloomFilter() {}

    bool mount() {
        return true;
    }
};

} // namespace TiStore
