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

namespace detail {

static inline
void get_posinfo(std::uint32_t bit_pos,
                 std::uint32_t & index,
                 std::uint32_t & offset)
{
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        index  = bit_pos / 64;
        offset = bit_pos % 64;
#else
        index  = bit_pos / 32;
        offset = bit_pos % 32;
#endif
}

} // namesoace detail

//
// M is the number of the total keys, M ==> kNumTotalKeys = N * K / B.
// B is the bits of per key, B ==> kBitsPerKey = (N * kNumProbes) / M.
//
// N is the bits of one hash (a probe) bitmap, N ==> kBitsOfPerProbe = (kNumTotalKeys * kBitsPerKey) / kNumProbes.
// K is the number of the different kind hash functions(probes), K ==> kNumProbes = B * ln(2) = kBitsPerKey * 0.69
//
class StandardBloomFilter {
private:
    std::unique_ptr<unsigned char> bitmap_;

    std::size_t bytes_per_probe_;
    std::size_t bits_per_probe_;
    std::size_t num_probes_;

    std::size_t bytes_total_;
    std::size_t num_total_keys_;
    std::size_t bits_per_key_;

    bool verbose_;

public:
    StandardBloomFilter(std::size_t num_total_keys, std::size_t bits_per_key, bool verbose = true)
        : bytes_per_probe_(0), bits_per_probe_(0), num_probes_(0), bytes_total_(0),
          num_total_keys_(num_total_keys), bits_per_key_(bits_per_key), verbose_(verbose) {
        initFilter(num_total_keys, bits_per_key);
    }
    ~StandardBloomFilter() {}

private:
    void initFilter(std::size_t num_total_keys, std::size_t bits_per_key) noexcept {
        bits_per_key_ = bits_per_key;
        num_probes_ = static_cast<std::size_t>((double)bits_per_key * 0.69);
        if (num_probes_ < 1)
            num_probes_ = 1;
        if (num_probes_ > 30)
            num_probes_ = 30;
        std::size_t bytes = num_total_keys * bits_per_key / num_probes_;
        bytes = BITS_ALIGNED_TO(bytes, CACHE_LINE_SIZE);
        bytes_per_probe_ = bytes;
        bits_per_probe_ = bytes * 8;
        
        if (getVerbose()) {
            // Basic information
            printf("num_total_keys      = %zu keys\n"
                   "bits_per_key        = %zu\n"
                   "bits_per_probe      = %zu bits\n"
                   "bytes_per_probe     = %zu bytes\n"
                   "num_probes          = %zu\n\n",
                    num_total_keys_, bits_per_key_, bits_per_probe_,
                    bytes_per_probe_, num_probes_);
        }

        bytes_total_ = ALIGNED_TO(bytes_per_probe_ * num_probes_, CACHE_LINE_SIZE);
        if (getVerbose()) {
            // Alloc information
            printf("size_of_bitmap      = %zu bytes\n", bytes_total_);
            printf("bits_of_bitmap      = %zu bits\n\n", bytes_total_ * 8);
            // The maximum capacity of the ideal number of key.
            printf("total_keys_capacity = %zu keys\n"
                   "bits_per_key        = %0.3f\n",
                   (std::size_t)((double)bits_per_probe_ * 0.69),
                   (double)(bytes_total_ * 8) / ((double)bits_per_probe_ * 0.69));
        }
        alignas(8) unsigned char * new_bitmap = new (std::nothrow) unsigned char [bytes_total_];
        if (new_bitmap) {
            ::memset((void *)new_bitmap, 0, bytes_total_ * sizeof(unsigned char));
            bitmap_.reset(new_bitmap);
        }
        else {
            bitmap_.reset(nullptr);
        }
        if (getVerbose())
            printf("\n");
    }

public:
    bool getVerbose() const { return verbose_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

    std::size_t getFilterSize() const { return bytes_total_; }

    // StandardBloomFilter
    void setOption(std::size_t num_total_keys, std::size_t bits_per_key, bool verbose = true) {
        setVerbose(verbose);
        initFilter(num_total_keys, bits_per_key);
    }

    // StandardBloomFilter
    void reset() {
        unsigned char * bitmap = bitmap_.get();
        assert(bitmap != nullptr);
        if (bitmap) {
            assert(bytes_total_ != 0);
            ::memset((void *)bitmap, 0, bytes_total_ * sizeof(unsigned char));
        }
    }

    // StandardBloomFilter
    inline void setBit(std::uint32_t probes, std::uint32_t bit_pos) {
        assert(probes < num_probes_);
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = std::size_t(1) << offset;
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get() + probes * bytes_per_probe_) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        bits_val |= bit_mask;
        *probe_bits = bits_val;
    }

    // StandardBloomFilter
    inline void clearBit(std::uint32_t probes, std::uint32_t bit_pos) {
        assert(probes < num_probes_);
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = ~(std::size_t(1) << offset);
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get() + probes * bytes_per_probe_) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        bits_val &= bit_mask;
        *probe_bits = bits_val;
    }

    // StandardBloomFilter
    inline bool insideBitmap(std::uint32_t probes, std::uint32_t bit_pos) const {
        assert(probes < num_probes_);
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = std::size_t(1) << offset;
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get() + probes * bytes_per_probe_) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        return ((bits_val & bit_mask) != 0);
    }

    // StandardBloomFilter
    void addKey(const Slice & key) {
        std::uint32_t primary_hash = HashUtils<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        std::uint32_t bit_pos = primary_hash % ((std::uint32_t)bits_per_probe_ - 0);
        // Note: 0 is first probe index, it's primary_hash function.
        setBit(0, bit_pos);
        if (num_probes_ > 1) {
            std::uint32_t secondary_hash, hash;
            secondary_hash = HashUtils<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % ((std::uint32_t)bits_per_probe_ - 0);
                setBit(i, bit_pos);
                hash += secondary_hash;
            }
        }
        if (getVerbose())
            printf("addKey(): %s\n", key.data());
    }

    // StandardBloomFilter
    bool maybeMatch(const Slice & key) const {
        std::uint32_t primary_hash = HashUtils<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        std::uint32_t bit_pos = primary_hash % ((std::uint32_t)bits_per_probe_ - 0);
        // Note: 0 is first probe index, it's primary_hash function.
        bool isMatch = insideBitmap(0, bit_pos);
        if (!isMatch)
            return false;
        if (num_probes_ > 1) {
            std::uint32_t secondary_hash, hash;
            secondary_hash = HashUtils<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % ((std::uint32_t)bits_per_probe_ - 0);
                isMatch = insideBitmap(i, bit_pos);
                if (!isMatch)
                    return false;
                hash += secondary_hash;
            }
        }
        return true;
    }

    std::uint32_t maybe_match(const Slice & key) {
        std::uint32_t hash = HashUtils<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
#if 0
        printf("key  = %s\n", key.data());
        printf("hash = %11u (0x%08X)\n", hash, hash);
        printf("\n");
#endif
        return hash;
    }

    std::uint32_t maybe_match2(const Slice & key) {
        std::uint32_t hash = HashUtils<std::uint32_t>::secondaryHash(key.data(), key.size());
        return hash;
    }

    std::uint32_t maybe_match_openssl(const Slice & key) {
        std::uint32_t hash = HashUtils<std::uint32_t>::OpenSSLHash(key.data(), key.size());
        return hash;
    }

    std::uint32_t rocksdb_maybe_match(const Slice & key) {
        std::uint32_t hash = rocksdb::hash::Hash(key.data(), key.size(), kDefaultHashSeed32);
        return hash;
    }
};

//
// M is the number of the total keys, M ==> kNumTotalKeys = N * K / B.
// B is the bits of per key, B ==> kBitsPerKey = (N * kNumProbes) / M.
//
// N is the bits of total bitmap, N ==> kBitsOfPerProbe = (kNumTotalKeys * kBitsPerKey) / kNumProbes.
// K is the number of the different kind hash functions(probes), K ==> kNumProbes = B * ln(2) = kBitsPerKey * 0.69
//
class FullBloomFilter {
private:
    std::unique_ptr<unsigned char> bitmap_;

    std::size_t bits_total_;
    std::size_t num_probes_;
    std::size_t bytes_per_probe_;

    std::size_t bytes_total_;
    std::size_t num_total_keys_;
    std::size_t bits_per_key_;

    bool verbose_;

public:
    FullBloomFilter(std::size_t num_total_keys, std::size_t bits_per_key, bool verbose = true)
        : bits_total_(0), num_probes_(0), bytes_per_probe_(0), 
          bytes_total_(0), num_total_keys_(num_total_keys), bits_per_key_(bits_per_key),
          verbose_(verbose) {
        initFilter(num_total_keys, bits_per_key);
    }
    ~FullBloomFilter() {}

private:
    void initFilter(std::size_t num_total_keys, std::size_t bits_per_key) noexcept {
        bits_per_key_ = bits_per_key;
        num_probes_ = static_cast<std::size_t>((double)bits_per_key * 0.69);
        if (num_probes_ < 1)
            num_probes_ = 1;
        if (num_probes_ > 30)
            num_probes_ = 30;

        std::size_t bytes = (num_total_keys * bits_per_key + 7) / 8;
        bytes_total_ = ALIGNED_TO(bytes, CACHE_LINE_SIZE);
        bits_total_ = bytes_total_ * 8;

        bytes_per_probe_ = (bytes_total_ / num_probes_) + 1;

        if (getVerbose()) {
            // Basic information
            printf("num_total_keys      = %zu keys\n"
                   "bits_per_key        = %zu\n"
                   "bits_per_probe      = %zu bits\n"
                   "bytes_per_probe     = %zu bytes\n"
                   "num_probes          = %zu\n\n",
                    num_total_keys_, bits_per_key_, bytes_per_probe_ * 8,
                    bytes_per_probe_, num_probes_);
            // Alloc information
            printf("size_of_bitmap_     = %zu bytes\n", bytes_total_);
            printf("bits_of_bitmap_     = %zu bits\n\n", bits_total_);
            // The maximum capacity of the ideal number of key.
            printf("total_keys_capacity = %zu keys\n"
                   "bits_per_key        = %0.3f\n",
                   (std::size_t)((double)(bytes_per_probe_ * 8) * 0.69),
                   (double)(bits_total_) / ((double)(bytes_per_probe_ * 8) * 0.69));
        }
        alignas(8) unsigned char * new_bitmap = new (std::nothrow) unsigned char[bytes_total_];
        if (new_bitmap) {
            ::memset((void *)new_bitmap, 0, bytes_total_ * sizeof(unsigned char));
            bitmap_.reset(new_bitmap);
        }
        else {
            bitmap_.reset(nullptr);
        }
        if (getVerbose())
            printf("\n");
    }

public:
    bool getVerbose() const { return verbose_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

    std::size_t getFilterSize() const { return bytes_total_; }

    // FullBloomFilter
    void setOption(std::size_t num_total_keys, std::size_t bits_per_key, bool verbose = true) {
        setVerbose(verbose);
        initFilter(num_total_keys, bits_per_key);
    }

    // FullBloomFilter
    void reset() {
        unsigned char * bitmap = bitmap_.get();
        assert(bitmap != nullptr);
        if (bitmap) {
            assert(bytes_total_ != 0);
            ::memset((void *)bitmap, 0, bytes_total_ * sizeof(unsigned char));
        }
    }

    // FullBloomFilter
    inline void setBit(std::uint32_t bit_pos) {
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = std::size_t(1) << offset;
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get()) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        bits_val |= bit_mask;
        *probe_bits = bits_val;
    }

    // FullBloomFilter
    inline void clearBit(std::uint32_t bit_pos) {
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = ~(std::size_t(1) << offset);
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get()) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        bits_val &= bit_mask;
        *probe_bits = bits_val;
    }

    // FullBloomFilter
    inline bool insideBitmap(std::uint32_t bit_pos) const {
        std::uint32_t index, offset;
        detail::get_posinfo(bit_pos, index, offset);
        register std::size_t bit_mask = std::size_t(1) << offset;
        std::size_t * probe_bits = (std::size_t *)(bitmap_.get()) + index;
        assert(probe_bits != nullptr);
        register std::size_t bits_val = (*probe_bits);
        return ((bits_val & bit_mask) != 0);
    }

    // FullBloomFilter
    void addKey(const Slice & key) {
        std::uint32_t primary_hash = HashUtils<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        std::uint32_t bit_pos = primary_hash % ((std::uint32_t)bits_total_ - 1);
        // Note: 0 is first probe index, it's primary_hash function.
        setBit(bit_pos);
        if (num_probes_ > 1) {
            std::uint32_t secondary_hash, hash;
            secondary_hash = HashUtils<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % ((std::uint32_t)bits_total_ - 0);
                setBit(bit_pos);
                hash += secondary_hash;
            }
        }
        if (getVerbose())
            printf("addKey(): %s\n", key.data());
    }

    // FullBloomFilter
    bool maybeMatch(const Slice & key) const {
        std::uint32_t primary_hash = HashUtils<std::uint32_t>::primaryHash(key.data(), key.size(), kDefaultHashSeed);
        std::uint32_t bit_pos = primary_hash % ((std::uint32_t)bits_total_ - 1);
        // Note: 0 is first probe index, it's primary_hash function.
        bool isMatch = insideBitmap(bit_pos);
        if (!isMatch)
            return false;
        if (num_probes_ > 1) {
            std::uint32_t secondary_hash, hash;
            secondary_hash = HashUtils<std::uint32_t>::secondaryHash(key.data(), key.size());
            hash = secondary_hash;
            for (int i = 1; i < (int)num_probes_; ++i) {
                bit_pos = hash % ((std::uint32_t)bits_total_ - 0);
                isMatch = insideBitmap(bit_pos);
                if (!isMatch)
                    return false;
                hash += secondary_hash;
            }
        }
        return true;
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
