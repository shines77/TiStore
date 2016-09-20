#pragma once

#include <stdint.h>

#include "TiStore/traits/bool_type.h"

namespace TiStore {
namespace traits {

template <typename T1, typename T2>
struct is_same : false_type
{
    // determine whether _Ty1 and _Ty2 are the same type
};

template <typename T1>
struct is_same<T1, T1> : true_type
{
    // determine whether _Ty1 and _Ty2 are the same type
};

} // namespace traits
} // namespace TiStore
