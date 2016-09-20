#pragma once

#include <stdint.h>

namespace TiStore {
namespace traits {

template <typename T>
struct const_type {
    typedef const T type;
};

template <typename T>
struct const_type<const T> {
    typedef const T type;
};

template <typename T>
struct const_type<T[]> {
    typedef const T type[];
};

template <typename T, size_t N>
struct const_type<T[N]> {
    typedef const T type[N];
};

template <typename T>
struct const_type<T *> {
    typedef T const * type;
};

template <typename T>
struct const_type<T const *> {
    typedef T const * const type;
};

template <typename T>
struct const_type<T * const> {
    typedef T const * const type;
};

template <typename T>
struct const_type<T const * const> {
    typedef T const * const type;
};

template <>
struct const_type<int32_t> {
    typedef const int32_t type;
};

template <>
struct const_type<uint32_t> {
    typedef const uint32_t type;
};

template <>
struct const_type<int64_t> {
    typedef const int64_t type;
};

template <>
struct const_type<uint64_t> {
    typedef const uint64_t type;
};

} // namespace traits
} // namespace TiStore
