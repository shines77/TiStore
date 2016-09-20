#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/Common.h"
#include "TiStore/kv/Hash.h"
#include "TiStore/traits.h"

#include <string.h>
#include <stdio.h>
#include <string>
#include <memory>

//
// See: https://github.com/Winnerhust/Code-of-Book/blob/master/Large-Scale-Distributed-Storage-System/skiplist/src/skiplist.h
// Reference from article: http://blog.csdn.net/qq910894904/article/details/37883953
//

namespace TiStore {

template <typename T, std::size_t MaxLevel = 10U>
class SkipList {
public:
    typedef typename traits::remove_const<T>::type  value_type;
    typedef typename traits::const_type<T>::type    const_value_type;

    static const std::size_t kMaxLevel = MaxLevel;

private:
    std::size_t max_level_;
    std::size_t size_;
    std::size_t capacity_;

public:
    SkipList() : max_level_(kMaxLevel), size_(0), capacity_(0) {}
    ~SkipList() {}

    bool build() {
        return true;
    }

    value_type & insert(value_type & value) {
        value_type node;
        return value;
    }

    value_type & remove(value_type & value) {
        return value;
    }
};

} // namespace TiStore
