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
    typedef std::map<KeyT, ValueT>                              node_type;
    typedef typename std::map<KeyT, ValueT>::iterator           node_iterator;
    typedef typename std::map<KeyT, ValueT>::const_iterator     const_node_iterator;
    typedef std::pair<KeyT, ValueT>                             node_pair_type;

    static const std::size_t kMaxLevel = MaxLevel;

private:
    std::size_t size_;
    std::size_t capacity_;
    node_type nodes_;

public:
    MapCollect() : size_(0), capacity_(0) {}
    ~MapCollect() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }

    bool insert(const node_type & node) {
        node_iterator it = nodes_.find(node.key());
        if (it != nodes_.end()) {
            std::string & value = *(new std::string(node.value().data()));
            it->second = value;
            return false;
        }
        else {
            nodes_.insert(std::make_pair<KeyT, ValueT>(node.key(), node.value()));
            size_++;
            return true;
        }
    }

    bool remove(const node_type & node) {       
        node_iterator it = nodes_.find(node.key());
        if (it != nodes_.end()) {
            nodes_.erase(it);
            size_--;
            return true;
        }
        return false;
    }

    bool remove(const char * key) {
        return false;
    }
};

template <typename Key, typename Value>
struct SkipListNode {

    struct SkipListNodeItem {
        SkipListNode<Key, Value> * next;
        SkipListNode<Key, Value> * prev;
    };

    Value *     value;
    Key *       key;
    size_t      level;
    SkipListNodeItem * items[1];
};

template <typename Key, typename Value, std::size_t MaxLevel>
class SkipLinkedList {
public:
    typedef Key                         key_type;
    typedef Value                       value_type;
    typedef SkipListNode<Key, Value>    node_type;

    static const std::size_t max_level = MaxLevel;

    enum {
        kFoundTheKey,
        kFoundTheKeyRange,
        kNotFound
    };

private:
    node_type * head_first_;
    node_type * head_;

public:
    SkipLinkedList() : head_first_(nullptr), head_(nullptr) {}
    ~SkipLinkedList() {}

    node_type * find(const key_type & key, int & find_type, unsigned int & out_level) {
        node_type * node = head_first_;
        unsigned int level = (unsigned int)max_level - 1;
        while (node != nullptr) {
            int cmp_result;
            key_type cmp_key = node->key;            
            if (key.size() > cmp_key.size())
                cmp_result = 1;
            else if (key.size() > cmp_key.size())
                cmp_result = -1;
            else
                cmp_result = ::memcmp(key.data(), cmp_key.data(), key.size());

            if (cmp_result > 0) {
                // Is bigger than now node
            }
            else if (cmp_result < 0) {
                // Is smaller than now node
                node = node->next[0];
            }
            else {
                // Find the key name
                find_type = kFoundTheKey;
                return node;
            }
            level--;
        }
        find_type = kNotFound;
        return nullptr;
    }

    bool insert(const key_type & key, const value_type & value) {
        int find_type;
        node_type * node = find(key, find_type);
        if (node != nullptr) {
            // Update the key
            return false;
        }
        else {
            // Insert a new key
            return true;
        }
    }
};

//
// See: http://dsqiu.iteye.com/blog/1705530 (original version)
// See: http://www.cppblog.com/mysileng/archive/2013/04/06/199159.html
//

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
    SkipLinkedList<Key, Value, kMaxLevel> linked_list_[kMaxLevel];

public:
    SkipList() : max_level_(kMaxLevel), size_(0), capacity_(0) {}
    ~SkipList() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t max_level() const { return max_level_; }

    size_t get_random_level() const {
        size_t rnd_level = get_random_num() % kMaxLevel;
        return rnd_level;
    }

private:
    size_t get_random_num() const {
#if (RAND_MAX == 0x7FFF)
        size_t rnd = (rand() << 15) | (rand() & 0x7FFFU);
#else
        size_t rnd = rand();
#endif
    }

public:
    static iterator end_iterator() {
        return reinterpret_cast<iterator>(nullptr);;
    }

    iterator find(const Key & key) {
        iterator it = end_iterator();
        return it;
    }

    bool insert(const value_type & node) {
        iterator it = find(node.key());
        if (it != end_iterator()) {
            std::string & value = *(new std::string(node.value().data()));
            // Update the record
            //update(it);
            return false;
        }
        else {
            size_++;
            return true;
        }
    }

    bool remove(const value_type & node) {       
        iterator it = find(node.key());
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
