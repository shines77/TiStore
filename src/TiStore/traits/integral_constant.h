#pragma once

#include "TiStore/traits/common.h"

namespace TiStore {
namespace traits {

// template class integral_constant
template <typename T, T Value>
struct integral_constant
{
    typedef T value_type;
    typedef integral_constant<T, Value> type;

    // convenient template for integral constant types
    static __CONST_EXPR value_type value = Value;

    __CONST_FUNC operator value_type() const __NOEXCEPT {
        // return stored value
        return (value);
    }

    __CONST_FUNC value_type operator()() const __NOEXCEPT {
        // return stored value
        return (value);
    }
};

} // namespace traits
} // namespace TiStore
