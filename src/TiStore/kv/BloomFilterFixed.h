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
class StandardBloomFilterFixed {
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
    std::size_t bits_per_probe_;
    std::size_t num_probes_;
    std::size_t size_of_bitmap_;
    std::size_t bits_per_key_;

    bool verbose_;

public:
    StandardBloomFilterFixed(bool verbose = true)
        : bytes_per_probe_((kBitsOfPerProbe + 7) / 8), bits_per_probe_((kBitsOfPerProbe + 7) / 8 * 8),
          num_probes_(kNumProbes), size_of_bitmap_(0), bits_per_key_(kBitsPerKey), verbose_(verbose) {
        initBloomFilter();
    }
    ~StandardBloomFilterFixed() {}

    bool getVerbose() const { return verbose_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

    std::size_t getFilterSize() const { return size_of_bitmap_; }

    void initBloomFilter() noexcept {
#if 1
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
        bits_per_probe_ = bytes_per_probe_ * 8;
        num_probes_ = static_cast<std::size_t>(kBitsPerKey * 0.69);
        bits_per_key_ = kBitsPerKey;
#else
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
        bits_per_probe_ = bytes_per_probe_ * 8;
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
                   (std::size_t)((double)bits_per_probe_ * 0.69),
                   (double)(size_of_bitmap_ * 8) / ((double)bits_per_probe_ * 0.69));
        }
        alignas(8) unsigned char * new_bitmap = new (std::nothrow) unsigned char [size_of_bitmap_];
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
            assert(size_of_bitmap_ != 0);
            ::memset((void *)bitmap, 0, size_of_bitmap_ * sizeof(unsigned char));
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
};

//
// N is the bits of one hash (a probe) bitmap, N = (kSizeOfTotalKeys * B) / K, (B = kBitsPerKey).
// B is the bits of per key, B = kBitsPerKey.
// K is the number of the different kind hash functions(probes), K = B * ln(2) = kBitsPerKey * 0.69, (ln(2) ~= 0.69)
//
// kSizeOfTotalKeys is the size of total keys. kSizeOfTotalKeys = N * K / B
//
template <std::size_t N, std::size_t B = 10, std::size_t K = 2>
class FullBloomFilterFixed {
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

    std::size_t bits_total_;
    std::size_t num_probes_;
    std::size_t bytes_per_probe_;
    std::size_t size_of_bitmap_;
    std::size_t bits_per_key_;

    bool verbose_;

public:
    FullBloomFilterFixed(bool verbose = true)
        : bits_total_((((kBitsOfPerProbe + 7) / 8) * kNumProbes) * 8),
          num_probes_(kNumProbes), bytes_per_probe_((kBitsOfPerProbe + 7) / 8), 
          size_of_bitmap_(((kBitsOfPerProbe + 7) / 8) * kNumProbes), 
          bits_per_key_(kBitsPerKey), verbose_(verbose) {
        initBloomFilter();
    }
    ~FullBloomFilterFixed() {}

public:
    bool getVerbose() const { return verbose_; }
    void setVerbose(bool verbose) { verbose_ = verbose; }

    std::size_t getFilterSize() const { return size_of_bitmap_; }

    void initBloomFilter() noexcept {
#if 1
        num_probes_ = static_cast<std::size_t>(kBitsPerKey * 0.69);
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
        bits_per_key_ = kBitsPerKey;
#else
        num_probes_ = kNumProbes;
        bytes_per_probe_ = BITS_ALIGNED_TO(kBitsOfPerProbe, CACHE_LINE_SIZE);
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
        bits_total_ = size_of_bitmap_ * 8;
        if (getVerbose()) {
            printf("size_of_bitmap_     = %zu bytes\n", size_of_bitmap_);
            printf("bits_of_bitmap_     = %zu bits\n\n", size_of_bitmap_ * 8);
            // The maximum capacity of the ideal number of key.
            printf(/*"kSizeOfTotalKeys    = %zu keys\n"*/
                   "total_keys_capacity = %zu keys\n"
                   "bits_per_key        = %0.3f\n",
                   //kSizeOfTotalKeys,
                   (std::size_t)((double)(bytes_per_probe_ * 8) * 0.69),
                   (double)(bits_total_) / ((double)(bytes_per_probe_ * 8) * 0.69));
        }
        alignas(8) unsigned char * new_bitmap = new (std::nothrow) unsigned char[size_of_bitmap_];
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
            assert(size_of_bitmap_ != 0);
            ::memset((void *)bitmap, 0, size_of_bitmap_ * sizeof(unsigned char));
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

} // namespace TiStore
