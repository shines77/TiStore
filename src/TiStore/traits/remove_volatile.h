#pragma once

namespace TiStore {
namespace traits {

template <typename T>
struct remove_volatile {
    typedef T type;
};

template <typename T>
struct remove_volatile<volatile T> {
    typedef T type;
};

} // namespace traits
} // namespace TiStore
