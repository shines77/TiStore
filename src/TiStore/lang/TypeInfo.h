#pragma once

#include "TiStore/basic/cstdint"

namespace TiStore {

static std::uint32_t g_typeinfo_typeid = 0;

static const std::size_t   kTypeInfoHashSeed   = 0x00000000BC9F1D34ULL;
static const std::uint32_t kTypeInfoHashSeed32 =         0xBC9F1D34UL;

template <typename T, typename HashType = std::uint32_t>
class TypeInfo {
public:
    typedef HashType hash_type;

private:
    static bool has_typeid_;
    static std::uint32_t typeid_;

    static bool has_hashcode_;
    static hash_type hashcode_;

private:
    static hash_type get_hash_code(std::uint32_t key, std::size_t seed) {
        // Similar to murmur hash
        static const hash_type m = static_cast<hash_type>(0x00000000C6A4A793ULL);
        static const std::uint8_t half_bits = sizeof(hash_type) * 8 / 2;

        register hash_type val = static_cast<hash_type>(key);
        register hash_type n = static_cast<hash_type>(key & 0x0FU);
        register hash_type hash = static_cast<hash_type>((hash_type)seed ^ (m * n));
        
        n += 4;
        while (n > 0) {
            hash += val;
            hash *= m;
            hash ^= (hash >> half_bits);
            val += sizeof(hash_type) + n;
            n--;
        }
        return hash;
    }

public:
    static std::uint32_t register_type() {
        return TypeInfo<T, HashType>::type_id();
    }

    static std::uint32_t type_id() {
        if (has_typeid_) {
            return typeid_;
        }
        else {
            has_typeid_ = true;
            typeid_ = g_typeinfo_typeid++;
            return typeid_;
        }
    }

    static hash_type hash_code() {
        if (has_hashcode_) {
            return hashcode_;
        }
        else {
            has_hashcode_ = true;
            std::uint32_t _type_id = TypeInfo<T, HashType>::type_id();
            hashcode_ = get_hash_code(_type_id, kTypeInfoHashSeed);
            return hashcode_;
        }
    }
};

template <typename T, typename HashType>
bool TypeInfo<T, HashType>::has_typeid_ = false;

template <typename T, typename HashType>
uint32_t TypeInfo<T, HashType>::typeid_ = std::uint32_t(-1);

template <typename T, typename HashType>
bool TypeInfo<T, HashType>::has_hashcode_ = false;

template <typename T, typename HashType>
typename TypeInfo<T, HashType>::hash_type TypeInfo<T, HashType>::hashcode_ = 0U;

} // namespace TiStore
