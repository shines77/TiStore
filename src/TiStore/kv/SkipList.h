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
    Key(const std::string & key) : Slice(key) {}
    ~Key() {}

    Key & operator = (const Key & rhs) {
        this->data_ = rhs.data_;
        this->size_ = rhs.size_;
        return *this;
    }
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

template <typename KeyT, typename ValueT>
class Record {
public:
    typedef typename traits::remove_const<KeyT>::type       key_type;
    typedef typename traits::const_type<KeyT>::type         const_key_type;
    typedef typename traits::remove_const<ValueT>::type     value_type;
    typedef typename traits::const_type<ValueT>::type       const_value_type;

private:
    size_t size_;
    key_type key_;
    value_type value_;

public:
    Record() : size_(0) {}
    ~Record() {}

    size_t read(key_type & key, value_type & value) {
        key = key_;
        value = value_;
        return size_;
    }

    void write(const key_type & key, const value_type & value) {
        key_ = key;
        value_ = value;
        size_ = 0 + key.size() + value.size();
    }

    key_type key() const { return key_; }
    value_type value() const { return value_; }

    const key_type const_key() const { return key_; }
    const value_type const_value() const { return value_; }
};

template <typename KeyT, typename ValueT>
class MapSet {
public:
    typedef typename traits::remove_const<KeyT>::type           key_type;
    typedef typename traits::const_type<KeyT>::type             const_key_type;
    typedef typename traits::remove_const<ValueT>::type         value_type;
    typedef typename traits::const_type<ValueT>::type           const_value_type;

    typedef std::map<key_type, value_type>                      node_type;
    typedef typename std::map<key_type, value_type>::iterator   node_iterator;
    typedef typename std::map<key_type, value_type>::const_iterator const_node_iterator;
    typedef std::pair<key_type, value_type>                     node_pair_type;

    static const std::size_t kMaxLevel = MaxLevel;

private:
    std::size_t size_;
    std::size_t capacity_;
    node_type nodes_;

public:
    MapSet() : size_(0), capacity_(0) {}
    ~MapSet() {}

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
            nodes_.insert(std::make_pair<key_type, value_type>(node.key(), node.value()));
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
    typedef SkipListNode<Key, Value>    node_type;
    typedef typename node_type *        iterator;
    typedef typename const node_type *  const_iterator;

    struct SkipListNodeItem {
        SkipListNode<Key, Value> * next;
        SkipListNode<Key, Value> * prev;
    };

    Value *     value;
    Key *       key;
    size_t      level;
    SkipListNodeItem * items[1];
};

template <typename Key, typename Value, size_t MaxLevel>
class SkipLinkedList {
public:
    typedef Key     key_type;
    typedef Value   value_type;

    typedef SkipListNode<Key, Value>            node_type;
    typedef typename node_type::iterator        iteration;
    typedef typename node_type::const_iterator  const_iterator;

    static const std::size_t max_level = MaxLevel;

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

template <typename KeyT, typename ValueT, size_t MaxLevel = 10U>
class SkipList {
public:
    typedef typename traits::remove_const<KeyT>::type       key_type;
    typedef typename traits::const_type<KeyT>::type         const_key_type;
    typedef typename traits::remove_const<ValueT>::type     value_type;
    typedef typename traits::const_type<ValueT>::type       const_value_type;

    typedef SkipList<KeyT, ValueT, MaxLevel>        this_type;
    typedef SkipListNode<key_type, value_type>      node_type;
    typedef typename node_type::iterator            node_iterator;
    typedef typename node_type::const_iterator      const_node_iterator;
    typedef std::pair<key_type, value_type>         node_pair_type;

    typedef node_type *                             iterator;
    typedef const node_type *                       const_iterator;

    static const size_t kMaxLevel = MaxLevel;
    static const size_t kMaxKeySize = 1024;
    // Notes: kSeparateSize must be multiply of 16.
    static const size_t kSeparateSize = 128;
    static const size_t kAdditiveIndex = (kSeparateSize / 16) - (kSeparateSize / 32);
    static const size_t kMaxKeyIndex = (kMaxKeySize / 32) + kAdditiveIndex + 1;

    enum {
        kFoundTheKey,
        kFoundInsertNode,
        kNotFound
    };

private:
    std::size_t max_level_;
    std::size_t size_;
    std::size_t capacity_;
    node_type * head_[kMaxKeyIndex][kMaxLevel];
    SkipLinkedList<key_type, value_type, kMaxLevel> linked_list_;

public:
    SkipList() : max_level_(kMaxLevel), size_(0), capacity_(0) {
        init();
    }
    ~SkipList() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t max_level() const { return max_level_; }

    constexpr node_type * null_node() const {
        return nullptr;
    }

    constexpr iterator null_iterator() const {
        return nullptr;
    }

    constexpr iterator end_iterator() const {
        return null_iterator();
    }

private:
    size_t get_random_num() const {
#if (RAND_MAX == 0x7FFF)
        size_t rnd = (rand() << 15) | (rand() & 0x7FFFU);
#else
        size_t rnd = rand();
#endif
    }

    size_t get_length_index(size_t length) const {
        if (length < kSeparateSize)
            return (length / 16);   // If length < 128, interval is small.
        else {
            if (length < kMaxKeySize)
                return (length / 32) + kAdditiveIndex;
            else
                return (kMaxKeySize / 32) + kAdditiveIndex;     // If length >= 1024, then index = 36.
        }
    }

    void init() {
        for (int i = 0; i < kMaxKeyIndex; i++) {
            for (int j = 0; j < kMaxLevel; j++) {
                head_[i][j] = nullptr;
            }
        }
    }

    bool insert_by_iter(iterator iter, const key_type & key, const value_type & value) {
        //
        size_++;
        return true;
    }

public:
    size_t get_random_level() const {
        size_t rnd_level = get_random_num() % kMaxLevel;
        return rnd_level;
    }

    iterator find(const key_type & key, int & find_type, int & out_level) {
        iterator iter = end_iterator();

        int level = 0;
        node_type * node = nullptr;
        size_t key_size = key.size();
        int key_index = get_length_index(key_size);
        node_type * head_base = head_[key_index][0];
        // Find the first validate linkedlist.
        while (level < kMaxLevel) {
            node = head_base[level];
            if (node != nullptr)
                break;
            level++;
        }
        bool is_first_node = true;
        while (node != nullptr) {
            int cmp_result;
            key_type * cmp_key = node->key;
            size_t cmp_size = cmp_key->size();
            size_t min_size = (key_size >= cmp_size) ? key_size : cmp_size;
            cmp_result = ::memcmp(key.data(), cmp_key->data(), min_size);
            if (cmp_result == 0) {
                if (key.size() > cmp_key->size())
                    cmp_result = +1;
                else if (key.size() > cmp_key->size())
                    cmp_result = -1
            }
            if (cmp_result > 0) {
                // Is bigger than now node
                node = node->items[0]->prev;
            }
            else if (cmp_result < 0) {
                // Is smaller than now node
                if (!is_first_node) {
                    node = node->items[0]->next;
                }
                else {
                    // It's smaller than first node in linkedlist.
                    goto NotFoundKey;
                }
            }
            else {
                // Find the key name
                find_type = kFoundTheKey;
                return node;
            }
            level++;
            if (level >= max_level_) {
                //
                return iter;
            }
        }
NotFoundKey:
        find_type = kNotFound;
        return null_iterator();
    }

    void update(iterator iter, std::string && value) {
        std::string * new_str = new std::string(value);
        value_type * new_value = new value_type(new_str);
        iter->value = new_value;
    }

    void update(iterator iter, const value_type & value) {
        assert(iter != nullptr);
        value_type * new_value = new value_type(value);
        iter->value = new_value;
    }

    bool insert(const key_type & key, const value_type & value) {
        int find_type = kNotFound, out_level = -1;
        iterator iter = find(key, find_type, out_level);
        if (iter != end_iterator()) {
            // Update the record
            update(iter, value);
            return false;
        }
        else {
            // Insert the record
            insert_by_iter(iter, key, value);
            return true;
        }
    }
    
    template <typename U>
    bool insert(U && record) {
        return insert(record.key(), record.value());
    }

    bool insert(const node_type & node) {
        return insert(node.key(), node.value());
    }

    bool remove(const key_type & key) {
        int find_type = kNotFound, out_level = -1;
        iterator iter = find(key, find_type, out_level);
        if (iter != end_iterator()) {
            // Erase the record
            //erase(iter);
            size_--;
            return true;
        }
        return false;
    }

    bool remove(key_type && key) {
        return remove(key);
    }

    bool remove(const char * key) {
        return false;
    }

    template <typename U>
    bool remove_by_record(U && record) {
        return remove(record.const_key());
    }

    bool remove(const node_type & node) {
        return remove(node.key());
    }
};

} // namespace TiStore
