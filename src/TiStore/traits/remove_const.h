#pragma once

#include <stdint.h>

namespace TiStore {
namespace traits {

template <typename T>
struct remove_const {
    typedef T type;
};

template <typename T>
struct remove_const<const T> {
    typedef T type;
};

template <typename T>
struct remove_const<const T[]> {
    typedef T type[];
};

template <typename T, size_t N>
struct remove_const<const T[N]> {
    typedef T type[N];
};

template <typename T>
struct remove_const<T *> {
    typedef T * type;
};

template <typename T>
struct remove_const<T const *> {
    typedef T const * type;
};

template <typename T>
struct remove_const<T * const> {
    typedef T * type;
};

template <typename T>
struct remove_const<T const * const> {
    typedef T const * type;
};

template <>
struct remove_const<int32_t> {
    typedef int32_t type;
};

template <>
struct remove_const<uint32_t> {
    typedef uint32_t type;
};

template <>
struct remove_const<int64_t> {
    typedef int64_t type;
};

template <>
struct remove_const<uint64_t> {
    typedef uint64_t type;
};

} // namespace traits
} // namespace TiStore
