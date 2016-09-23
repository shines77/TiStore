#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/Common.h"
#include "TiStore/kv/Hash.h"
#include "TiStore/traits.h"

#include <string.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <map>

//
// See: https://github.com/Winnerhust/Code-of-Book/blob/master/Large-Scale-Distributed-Storage-System/skiplist/src/skiplist.h
// Reference from article: http://blog.csdn.net/qq910894904/article/details/37883953
//

namespace TiStore {

class Key : public Slice {
public:
    Key() {}
    Key(const char * key) : Slice(key) {}
    ~Key() {}
};

class Value: public Slice {
public:
    Value() {}
    Value(const char * value) : Slice(value) {}
    Value(const std::string & value) : Slice(value) {}
    ~Value() {}

    Value & operator = (const Value & rhs) {
        this->data_ = rhs.data_;
        this->size_ = rhs.size_;
        return *this;
    }
};

class Record {
public:
    Record() : size_(0) {}
    ~Record() {}

    size_t read(Key & key, Value & value) {
        key = key_;
        value = value_;
        return size_;
    }

    void write(const Key & key, const Value & value) {
        key_ = key;
        value_ = value;
        size_ = 0 + key.size() + value.size();
    }

    Key key() const { return key_; }
    Value value() const { return value_; }

private:
    size_t size_;
    Key key_;
    Value value_;
};

template <typename KeyT, typename ValueT>
class MapCollect {
public:
    typedef typename traits::remove_const<KeyT>::type           key_type;
    typedef typename traits::const_type<KeyT>::type             const_key_type;
    typedef typename traits::remove_const<ValueT>::type         value_type;
    typedef typename traits::const_type<ValueT>::type           const_value_type;
    typedef std::map<KeyT, ValueT>                              record_type;
    typedef typename std::map<KeyT, ValueT>::iterator           record_iterator;
    typedef typename std::map<KeyT, ValueT>::const_iterator     const_record_iterator;
    typedef std::pair<KeyT, ValueT>                             record_pair_type;

    static const std::size_t kMaxLevel = MaxLevel;

private:
    std::size_t size_;
    std::size_t capacity_;
    record_type collect_;

public:
    MapCollect() : size_(0), capacity_(0) {}
    ~MapCollect() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }

    bool insert(const record_type & record) {
        record_iterator it = collect_.find(record.key());
        if (it != collect_.end()) {
            std::string & value = *(new std::string(record.value().data()));
            it->second = value;
            return false;
        }
        else {
            collect_.insert(std::make_pair<KeyT, ValueT>(record.key(), record.value()));
            size_++;
            return true;
        }
    }

    bool remove(const record_type & record) {       
        record_iterator it = collect_.find(record.key());
        if (it != collect_.end()) {
            collect_.erase(it);
            size_--;
            return true;
        }
        return false;
    }

    bool remove(const char * key) {
        return false;
    }
};

template <typename T, std::size_t MaxLevel = 10U>
class SkipList {
public:
    typedef typename traits::remove_const<T>::type  value_type;
    typedef typename traits::const_type<T>::type    const_value_type;
    typedef value_type *                            iterator;
    typedef const_value_type *                      const_iterator;

    static const std::size_t kMaxLevel = MaxLevel;

private:
    std::size_t max_level_;
    std::size_t size_;
    std::size_t capacity_;
    std::map<Key, Value> collect_;

public:
    SkipList() : max_level_(kMaxLevel), size_(0), capacity_(0) {}
    ~SkipList() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t max_level() const { return max_level_; }

    bool build() {
        return true;
    }

    static iterator end_iterator() {
        return reinterpret_cast<iterator>(nullptr);;
    }

    iterator find(const Key & key) {
        iterator it = end_iterator();
        return it;
    }

    bool insert(const value_type & record) {
        iterator it = find(record.key());
        if (it != end_iterator()) {
            std::string & value = *(new std::string(record.value().data()));
            // Update the record
            //update(it);
            return false;
        }
        else {
            size_++;
            return true;
        }
    }

    bool remove(const value_type & record) {       
        iterator it = find(record.key());
        if (it != end_iterator()) {
            // Erase the record
            //erase(it);
            size_--;
            return true;
        }
        return false;
    }

    bool remove(const char * key) {
        return false;
    }
};

} // namespace TiStore
