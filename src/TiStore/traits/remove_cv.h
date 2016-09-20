#pragma once

#include "TiStore/traits/remove_const.h"
#include "TiStore/traits/remove_volatile.h"

namespace TiStore {
namespace traits {

template <typename T>
struct remove_cv {
    typedef typename traits::remove_volatile<typename traits::remove_const<T>::type>::type type;
};

} // namespace traits
} // namespace TiStore
