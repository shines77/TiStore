#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/Common.h"
#include "TiStore/kv/Hash.h"
#include "TiStore/lang/TypeInfo.h"

#include <string.h>
#include <stdio.h>
#include <string>
#include <memory>

#define ALIGNED_TO_TYPE(N, alignment, type)     (type)(((type)(N) + (alignment) - 1) / (alignment) * (alignment))
#define ALIGNED_TO_SIZE(N, alignment)           ALIGNED_TO_TYPE(N, alignment, size_t)
#define ALIGNED_TO(N, alignment)                ALIGNED_TO_SIZE(N, alignment)

#define BITS_ALIGNED_TO_TYPE(bits, alignment, type) \
    (type)ALIGNED_TO_TYPE((((bits) + 7) / 8), alignment, type)

#define BITS_ALIGNED_TO_SIZE(bits, alignment) \
    BITS_ALIGNED_TO_TYPE(bits, alignment, size_t)

#define BITS_ALIGNED_TO(bits, alignment) \
    BITS_ALIGNED_TO_SIZE(bits, alignment)

#define BITS_ALIGNED_TO_BITS(bits, alignment) \
    (BITS_ALIGNED_TO_SIZE(bits, alignment) * 8)

namespace TiStore {

//
// Inside The Bloom Filter
//
// By Yebangyu, from alibaba(Ants Jinfu)
//
// See: http://www.yebangyu.org/blog/2016/01/23/insidethebloomfilter/
// See: http://www.yebangyu.org/blog/2016/01/29/insidethebloomfilter/
//

//
// N is the bits of one hash (a probe) bitmap, N = (kSizeOfTotalKeys * B) / K, (B = kBitsPerKey).
// B is the bits of per key, B = kBitsPerKey.
// K is the number of the different kind hash functions(probes), K = B * ln(2) = kBitsPerKey * 0.69, (ln(2) ~= 0.69)
//
// kSizeOfTotalKeys is the size of total keys. kSizeOfTotalKeys = N * K / B
//
template <std::size_t N, std::size_t B = 10, std::size_t K = 2>
class StandardBloomFilter {
private:
    // The bits of one hash (a probe) bitmap.
    static const std::size_t kBitsOfPerProbe = BITS_ALIGNED_TO_BITS(N, CACHE_LINE_SIZE);
    // The bits of per key.
    static const std::size_t kBitsPerKey = B;
    // The number of the different kind hash functions (probes).
    static const std::size_t kNumProbes = K;

    // The size of total keys.
    static const std::size_t kSizeOfTotalKeys = (std::size_t)((double)(kBitsOfPerProbe) * 0.69);

    std::unique_ptr<unsigned char> bitmap_;

    std::size_t bytes_per_probe_;
    std::size_t bits_per_key_;
    std::size_t num_probes_;
    std::size_t size_of_bitmap_;

    bool verbose_;

public:
    StandardBloomFilter(bool verbose = true)
        : bytes_per_probe_((kBitsOfPerProbe + 7) / 8), bits_per_key_(kBitsPerKey),
          num_probes_(kNumProbes), size_of_bitmap_(0), verbose_(verbose) {
        initBloomFilter();
    }
    ~StandardBloomFilter() {}

    bool getVerbose() const { return verbose_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

    void initBloomFilter() noexcept {
#if 1
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
        bits_per_key_ = kBitsPerKey;
        num_probes_ = static_cast<std::size_t>(kBitsPerKey * 0.69);
#else
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
        num_probes_ = kNumProbes;
        //bits_per_key_ = static_cast<std::size_t>(((double)(kNumProbes) + 0.01) / 0.69);
        bits_per_key_ = bytes_per_probe_ * 8 * num_probes_ / kSizeOfTotalKeys;
#endif
        if (getVerbose()) {
            printf("bits_per_probe_     = %zu bits\n"
                   "bytes_per_probe_    = %zu bytes\n"
                   "bits_per_key_       = %zu\n"
                   "num_probes_         = %zu\n\n",
                    kBitsOfPerProbe, bytes_per_probe_, bits_per_key_, num_probes_);
        }

        size_of_bitmap_ = ALIGNED_TO(bytes_per_probe_ * num_probes_, CACHE_LINE_SIZE);
        if (getVerbose()) {
            printf("size_of_bitmap_     = %zu bytes\n", size_of_bitmap_);
            printf("bits_of_bitmap_     = %zu bits\n\n", size_of_bitmap_ * 8);
            // The maximum capacity of the ideal number of key.
            printf(/*"kSizeOfTotalKeys    = %zu keys\n"*/
                   "total_keys_capacity = %zu keys\n"
                   "bits_per_key        = %0.3f\n",
                   //kSizeOfTotalKeys,
                   (std::size_t)((double)(bytes_per_probe_ * 8) * 0.69),
                   (double)(size_of_bitmap_ * 8) / ((double)(bytes_per_probe_ * 8) * 0.69));
        }
        unsigned char * new_bitmap = new (std::nothrow) unsigned char [size_of_bitmap_];
        if (new_bitmap) {
            ::memset((void *)new_bitmap, 0, size_of_bitmap_ * sizeof(unsigned char));
            bitmap_.reset(new_bitmap);
        }
        else {
            bitmap_.reset(nullptr);
        }
        if (getVerbose())
            printf("\n");
    }

    void reset() {
        unsigned char * bitmap = bitmap_.get();
        assert(bitmap != nullptr);
        if (bitmap) {
            ::memset((void *)bitmap, 0, size_of_bitmap_ * sizeof(unsigned char));
        }
    }

    inline void setToBitmap(std::uint32_t probes, std::uint32_t bit_pos) {
        assert(probes < num_probes_);
        unsigned char * bitmap = bitmap_.get();
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        std::uint64_t * probe_bits = (std::uint64_t *)(bitmap + probes * bytes_per_probe_);
        std::uint32_t index  = bit_pos / 64;
        std::uint32_t offset = bit_pos % 64;
        std::uint64_t bit_mask = 1U << offset;
        probe_bits += index;
        std::uint64_t bits_val = (*probe_bits);
        bits_val |= bit_mask;
        *probe_bits = bits_val;
#else
        std::uint32_t * probe_bits = (std::uint32_t *)(bitmap + probes * bytes_per_probe_);
        std::uint32_t index  = bit_pos / 32;
        std::uint32_t offset = bit_pos % 32;
        std::uint32_t bit_mask = 1U << offset;
        probe_bits += index;
        std::uint32_t bits_val = (*probe_bits);
        bits_val |= bit_mask;
        *probe_bits = bits_val;
#endif
    }

    inline bool isIsideBitmap(std::uint32_t probes, std::uint32_t bit_pos) const {
        assert(probes < num_probes_);
        unsigned char * bitmap = bitmap_.get();
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        std::uint64_t * probe_bits = (std::uint64_t *)(bitmap + probes * bytes_per_probe_);
        std::uint32_t index  = bit_pos / 64;
        std::uint32_t offset = bit_pos % 64;
        std::uint64_t bit_mask = 1U << offset;
        probe_bits += index;
        std::uint64_t bits_val = (*probe_bits);
#else
        std::uint32_t * probe_bits = (std::uint32_t *)(bitmap + probes * bytes_per_probe_);
        std::uint32_t index  = bit_pos / 32;
        std::uint32_t offset = bit_pos % 32;
        std::uint32_t bit_mask = 1U << offset;
        probe_bits += index;
        std::uint32_t bits_val = (*probe_bits);
#endif
        return ((bits_val & bit_mask) != 0);
    }

    void addKey(const Slice & key) {
        uint32_t primary_hash = BloomFilterHash<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        uint32_t bit_pos = primary_hash % bytes_per_probe_;
        // Note: 0 is first probe index, it's primary_hash function.
        setToBitmap(0, bit_pos);
        if (num_probes_ > 1) {
            uint32_t secondary_hash, hash;
            secondary_hash = BloomFilterHash<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % bytes_per_probe_;
                setToBitmap(i, bit_pos);
                hash += secondary_hash;
            }
        }
        if (getVerbose())
            printf("addKey(): %s\n", key.data());
    }

    bool maybeMatch(const Slice & key) const {
        uint32_t primary_hash = BloomFilterHash<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        uint32_t bit_pos = primary_hash % bytes_per_probe_;
        // Note: 0 is first probe index, it's primary_hash function.
        bool isMatch = isIsideBitmap(0, bit_pos);
        if (!isMatch)
            return false;
        if (num_probes_ > 1) {
            uint32_t secondary_hash, hash;
            secondary_hash = BloomFilterHash<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % bytes_per_probe_;
                isMatch = isIsideBitmap(i, bit_pos);
                if (!isMatch)
                    return false;
                hash += secondary_hash;
            }
        }
        return true;
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

    bool build() {
        return true;
    }
};

} // namespace TiStore
