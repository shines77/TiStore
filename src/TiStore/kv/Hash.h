#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/basic/cstdssize"
#include "TiStore/kv/Slice.h"

#include <string>

namespace rocksdb {

namespace port {

static bool kLittleEndian = true;

} // namespace port

namespace hash {

inline uint32_t DecodeFixed32(const char* ptr) {
    if (port::kLittleEndian) {
        // Load the raw bytes
        uint32_t result;
        memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
        return result;
    }
    else {
        return ((static_cast<uint32_t>(static_cast<unsigned char>(ptr[0])))
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[1])) << 8)
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[2])) << 16)
            | (static_cast<uint32_t>(static_cast<unsigned char>(ptr[3])) << 24));
    }
}

inline uint64_t DecodeFixed64(const char* ptr) {
    if (port::kLittleEndian) {
        // Load the raw bytes
        uint64_t result;
        memcpy(&result, ptr, sizeof(result));  // gcc optimizes this to a plain load
        return result;
    }
    else {
        uint64_t lo = DecodeFixed32(ptr);
        uint64_t hi = DecodeFixed32(ptr + 4);
        return (hi << 32) | lo;
    }
}

uint32_t Hash(const char * data, size_t n, uint32_t seed) {
    // Similar to murmur hash
    const uint32_t m = 0xc6a4a793;
    const uint32_t r = 24;
    const char* limit = data + n;
    uint32_t h = static_cast<uint32_t>(seed ^ (n * m));

    // Pick up four bytes at a time
    while (data + 4 <= limit) {
        uint32_t w = DecodeFixed32(data);
        data += 4;
        h += w;
        h *= m;
        h ^= (h >> 16);
    }

    // Pick up remaining bytes
    switch (limit - data) {
        // Note: It would be better if this was cast to unsigned char, but that
        // would be a disk format change since we previously didn't have any cast
        // at all (so gcc used signed char).
        // To understand the difference between shifting unsigned and signed chars,
        // let's use 250 as an example. unsigned char will be 250, while signed char
        // will be -6. Bit-wise, they are equivalent: 11111010. However, when
        // converting negative number (signed char) to int, it will be converted
        // into negative int (of equivalent value, which is -6), while converting
        // positive number (unsigned char) will be converted to 250. Bitwise,
        // this looks like this:
        // signed char 11111010 -> int 11111111111111111111111111111010
        // unsigned char 11111010 -> int 00000000000000000000000011111010
    case 3:
        h += static_cast<uint32_t>(static_cast<signed char>(data[2]) << 16);
        // fall through
    case 2:
        h += static_cast<uint32_t>(static_cast<signed char>(data[1]) << 8);
        // fall through
    case 1:
        h += static_cast<uint32_t>(static_cast<signed char>(data[0]));
        h *= m;
        h ^= (h >> r);
        break;
    }
    return h;
}

} // namespace hash
} // namespace rocksdb

namespace TiStore {

static std::size_t   kDefaultHashSeed   = 0x00000000BC9F1D34ULL;
static std::uint32_t kDefaultHashSeed32 =         0xBC9F1D34UL;

/**************************************************************************

    About hash algorithm

See:
    BKDRHash, APHash, JSHash, RSHash, SDBMHash, PJWHash, ELFHash
    http://blog.csdn.net/icefireelf/article/details/5796529

    Good: BKDRHash, APHash. Mid: DJBHash, JSHash, RSHash, SDBMHash. Bad: PJWHash, ELFHash.
    http://blog.csdn.net/pingnanlee/article/details/8232372

    Blizzard: "One Way Hash"
    http://blog.chinaunix.net/uid-20775243-id-2554977.html

    Another hash algorithm: hashpjw(PHP), lh_strhash(OpenSSL), calc_hashnr(MySQL)
    http://blog.chinaunix.net/uid-21457204-id-3061239.html
    http://blog.csdn.net/nhczp/article/details/3040546

 **************************************************************************/

namespace hash {

//
// BKDR Hash Function -- Times31, Times33, Times131 ...
// SDBM Hash Function (seed = 65599)
//
//   hash = hash * seed^4 + a * seed^3 + b * seed^2 + c * seed + d;
//
uint32_t BKDRHash(const char * key, std::size_t len)
{
    static const uint32_t seed = 131U;   // 31, 33, 131, 1313, 13131, 131313, etc ...
    static const uint32_t seed_2 = seed * seed;
    static const uint32_t seed_3 = seed_2 * seed;
    static const uint32_t seed_4 = seed_2 * seed_2;

    register const unsigned char * src = (const unsigned char *)key;
    register const unsigned char * end = src + len;
    uint32_t hash = 0;
 
#if 1
    register const unsigned char * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src != limit) {
        hash = hash * seed_4 + src[0] * seed_3 + src[1] * seed_2 + src[2] * seed + src[3];
        src += 4;
    }
#endif
    while (src != end) {
        hash = hash * seed + (uint32_t)(*src);
        src++;
    }
 
    return hash;
}

//
// BKDR Hash Function (seed = 31) -- Times31, use on Java string hashCode.
//
//   hash = hash * seed^4 + a * seed^3 + b * seed^2 + c * seed + d;
//
uint32_t BKDRHash_31(const char * key, std::size_t len)
{
    static const uint32_t seed = 31U;   // 31, 33, 131, 1313, 13131, 131313, etc ...
    static const uint32_t seed_2 = seed * seed;
    static const uint32_t seed_3 = seed_2 * seed;
    static const uint32_t seed_4 = seed_2 * seed_2;

    register const unsigned char * src = (const unsigned char *)key;
    register const unsigned char * end = src + len;
    register uint32_t hash = 0;
 
#if 1
    register const unsigned char * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src < limit) {
        hash = hash * seed_4 + src[0] * seed_3 + src[1] * seed_2 + src[2] * seed + src[3];
        src += 4;
    }
#endif
    while (src != end) {
        hash = hash * seed + (uint32_t)(*src);
        src++;
    }
 
    return hash;
}

//
// APHash Hash Function
//
uint32_t APHash(const char * key, std::size_t len)
{
    const unsigned char * src = (const unsigned char *)key;
    const unsigned char * end = src + len;

    uint32_t hash = 0;

#if 1
    const unsigned char * limit = src + (len & std::size_t(~(std::size_t)3U));
    while (src != limit) {
        //if (*src == '\0')
        //    break;
        hash ^=   ((hash <<  7U) ^ ((uint32_t)src[0]) ^ (hash >> 3U));
        hash ^= (~((hash << 11U) ^ ((uint32_t)src[1]) ^ (hash >> 5U)));
        hash ^=   ((hash <<  7U) ^ ((uint32_t)src[2]) ^ (hash >> 3U));
        hash ^= (~((hash << 11U) ^ ((uint32_t)src[3]) ^ (hash >> 5U)));
        src += 4;
    }
#endif
    uint32_t i = 0;
    while (src != end) {
        //if (*src == '\0')
        //    break;
        if ((i & 1) == 0)
            hash ^=   ((hash <<  7U) ^ ((uint32_t)(*src)) ^ (hash >> 3U));
        else
            hash ^= (~((hash << 11U) ^ ((uint32_t)(*src)) ^ (hash >> 5U)));
        i++;
        src++;
    }
    return hash;
}

//
// DJB Hash Function
//
uint32_t DJBHash(const char * key, std::size_t len)
{
    uint32_t hash = 5381U;
    const unsigned char * src = (const unsigned char *)key;
    const unsigned char * end = src + len;
 
    while (src != end) {
        if (*src == '\0')
            break;
        hash += (hash << 5U) + (uint32_t)(*src);
        src++;
    }
 
    return hash;
}

} // namespace hash

template <typename T = std::uint32_t>
class BloomFilterHash {
public:
    typedef T hash_type;

    static hash_type PrimeHash(const char * key, std::size_t len, std::size_t seed) {
        // Similar to murmur hash
        static const std::size_t _m = (std::size_t)0x00000000C6A4A793ULL;
        static const std::size_t half_bits = sizeof(hash_type) * 8 / 2;
        static const std::size_t align_mask = sizeof(hash_type) - 1;

        register const char * data = key;
        const char * end  = data + len;

        static const hash_type m = static_cast<hash_type>(_m);
        hash_type n = static_cast<hash_type>(len);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));
        
        if ((std::size_t(data) & align_mask) == 0) {
            // The data address is aligned to sizeof(hash_type) bytes.
            register const char * limit = (const char *)(std::size_t(end) & std::size_t(~(std::size_t)align_mask));
            while (data < limit) {
                hash_type val = *(hash_type *)data;
                hash += val;
                hash *= m;
                hash ^= (hash >> half_bits);
                data += sizeof(hash_type);
            }
            hash_type remain = (hash_type)(end - data);
            if (remain == 0)
                return hash;
            // Filter the extra bits
            hash_type val = *(hash_type *)data;
            static const hash_type val_mask = (hash_type)(-1);
            val &= (val_mask >> ((sizeof(hash_type) - remain) * 8));
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            return hash;
        }
        else {
            // The data address is not aligned to sizeof(hash_type) bytes.
        }
        return hash;
    }

    static hash_type PrimeHash(const Slice & key, std::size_t seed) {
        return PrimeHash(key.data(), key.size(), seed);
    }

    template <std::size_t N>
    static hash_type PrimeHash(const char (&key)[N], std::size_t seed) {
        return PrimeHash(key, N, seed);
    }

    static hash_type SecondaryHash(const char * key, std::size_t len) {
        return static_cast<hash_type>(hash::BKDRHash_31(key, len));
    }

    static hash_type SecondaryHash(const Slice & key) {
        return SecondaryHash(key.data(), key.size());
    }

    template <std::size_t N>
    static hash_type SecondaryHash(const char (&key)[N]) {
        return SecondaryHash(key, N, seed);
    }
};

} // namespace TiStore
