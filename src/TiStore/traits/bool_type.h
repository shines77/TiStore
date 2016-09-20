#pragma once

#include "TiStore/traits/integral_constant.h"

namespace TiStore {
namespace traits {

typedef integral_constant<bool, true>  true_type;
typedef integral_constant<bool, false> false_type;

template <typename T1, typename T2>
struct _true_type
{
    enum { value = true };
};

template <typename T1>
struct _false_type
{
    enum { value = false };
};

} // namespace traits
} // namespace TiStore
