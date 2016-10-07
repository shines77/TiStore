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
                // Bigger than current node
            }
            else if (cmp_result < 0) {
                // Smaller than current node
                node = node->next[0];
            }
            else {
                // Have found the key name
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
    class Node;
    class iterator;
    typedef typename traits::remove_const<KeyT>::type       key_type;
    typedef typename traits::const_type<KeyT>::type         const_key_type;
    typedef typename traits::remove_const<ValueT>::type     value_type;
    typedef typename traits::const_type<ValueT>::type       const_value_type;

    typedef SkipList<KeyT, ValueT, MaxLevel>        this_type;
    //typedef SkipListNode<key_type, value_type>      node_type;
    typedef Node                                    node_type;
    //typedef typename node_type::iterator            node_iterator;
    //typedef typename node_type::const_iterator      const_node_iterator;
    typedef std::pair<key_type, value_type>         node_pair_type;

    //typedef node_type *                             iterator;
    //typedef const node_type *                       const_iterator;
    typedef const iterator                          const_iterator;
    typedef std::nullptr_t                          end_iterator_t;

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

public:
    class Node {
    public:
        Node * next;
        Node * prev;
        key_type * key;
        value_type * value;
    private:
        Node * next_[1];

    public:
        explicit Node(const key_type & k, const value_type & v)
            : key(&k), value(&v) { }
        ~Node() { }

        Node * getNext(int n) {
            assert(n >= 0);
            return next_[n];
        }

        void setNext(int n, Node * node) {
            assert(n >= 0);
            next_[n] = node;
        }
    };

private:
    std::size_t max_level_;
    std::size_t size_;
    std::size_t capacity_;
    node_type * head_[kMaxKeyIndex + 1][kMaxLevel];
    SkipLinkedList<key_type, value_type, kMaxLevel> linked_list_;

public:
    // Iteration over the contents of a skip list
    class iterator {
    public:
        // Initialize an iterator over the specified list.
        // The returned iterator is not valid.
        explicit iterator(const SkipList * list) : list_(list), node_(nullptr) { }

        // Init the iterator
        void init(const SkipList * list, node_type * node) {
            list_ = list;
            node_ = node;
        }

        // Change the underlying skiplist used for this iterator
        // This enables us not changing the iterator without deallocating
        // an old one and then allocating a new one
        void setList(const SkipList * list) {
            list_ = list;
            node_ = nullptr;
        }

        // Returns true iff the iterator is positioned at a valid node.
        bool is_valid() const { return (node_ != nullptr); }

        // Returns the key at the current position.
        // REQUIRES: is_valid()
        const key_type & key() const { return node_->key; }

        node_type & node() const { return (*node_); }

        // Advances to the next position.
        // REQUIRES: is_valid()
        void next() { }

        // Advances to the previous position.
        // REQUIRES: is_valid()
        void prev() { }

        // Advance to the first entry with a key >= target
        void seek(const key_type & target) { }

        // Position at the first entry in list.
        // Final state of iterator is is_valid() if list is not empty.
        void seek_to_first() { }

        // Position at the last entry in list.
        // Final state of iterator is is_valid() if list is not empty.
        void seek_to_last() { }

        node_type & operator * () const {
            return (node_type &)(*node_);
        }

        node_type * operator -> () const {
            return node_;
        }

        // preincrement: ++i;
        iterator & operator ++()
        {
            ++*(iterator *)this;
            return (*this);
        }

        // postincrement: i++;
        iterator operator ++(int)
        {
            iterator tmp = *this;
            ++*this;
            return (tmp);
        }

        // predecrement: --i;
        iterator & operator --()
        {
            --*(iterator *)this;
            return (*this);
        }

        // postdecrement: i--;
        iterator operator --(int)
        {
            iterator tmp = *this;
            --*this;
            return (tmp);
        }

        bool operator == (node_type * node) {
            return (node_ == node);
        }

        bool operator != (node_type * node) {
            return (node_ != node);
        }

        bool operator > (node_type * node) {
            return (node_ > node);
        }

        bool operator < (node_type * node) {
            return (node_ < node);
        }

        iterator & operator = (const iterator & iter) {
            if ((iterator *)&iter != this)
                init(iter->list_, iter->node_);
            return (*this);
        }

    private:
        const SkipList * list_;
        node_type * node_;
        // Intentionally copyable
    };

public:
    SkipList() : max_level_(kMaxLevel), size_(0), capacity_(0) {
        init();
    }
    ~SkipList() {}

    size_t sizes() const { return size_; }
    size_t capacity() const { return capacity_; }
    size_t max_level() const { return max_level_; }

    bool is_valid() const {
        return true;
    }

    node_type * begin() const {
        return nullptr();
    }

    node_type * end() const {
        return nullptr();
    }

private:
    size_t get_random_num() const {
#if (RAND_MAX == 0x7FFF)
        size_t rnd = (rand() << 15) | (rand() & 0x7FFFU);
#else
        size_t rnd = rand();
#endif
    }

    int get_length_index(size_t length) const {
        if (length < kSeparateSize)
            return (int)(length / 16);   // If length < 128, interval is small.
        else {
            if (length < kMaxKeySize)
                return (int)((length / 32) + kAdditiveIndex);
            else
                return (int)((kMaxKeySize / 32) + kAdditiveIndex);     // If length >= 1024, then index = 36.
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

    void prev(int i) {
        //
    }

    void next(int i) {
        //
    }

    iterator find(const key_type & key, int & find_type, int & out_level) {
        iterator iter(this);

        node_type * node;
        size_t key_size = key.size();
        int key_index = get_length_index(key_size);
        assert(key_index <= kMaxKeyIndex);
        // Get the first node of the specified level by key_index.
        node_type ** head_base = &head_[key_index][0];
        int level = 0;
        // Find the first validate linkedlist.
        while (level < kMaxLevel) {
            node = *head_base;
            if (node != nullptr)
                break;
            head_base++;
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
                    cmp_result = -1;
            }
            if (cmp_result > 0) {
                // Bigger than current node
                node = node->getNext(0)->prev;
            }
            else if (cmp_result < 0) {
                // Smaller than current node
                if (!is_first_node) {
                    node = node->getNext(0)->next;
                }
                else {
                    // It's smaller than first node in linkedlist.
                    goto NotFoundKey;
                }
            }
            else {
                // Have found the key name
                find_type = kFoundTheKey;
                return iter;
            }
            level++;
            if (level >= max_level_) {
                //
                return iter;
            }
        }
NotFoundKey:
        find_type = kNotFound;
        return iter;
    }

    void update(iterator & iter, std::string && value) {
        std::string * new_str = new std::string(value);
        value_type * new_value = new value_type(new_str);
        iter.node().value = new_value;
    }

    void update(iterator & iter, const value_type & value) {
        assert(iter != nullptr);
        value_type * new_value = new value_type(value);
        iter.node().value = new_value;
    }

    bool insert(const key_type & key, const value_type & value) {
        int find_type = kNotFound, out_level = -1;
        iterator iter = find(key, find_type, out_level);
        if (iter != nullptr) {
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
        if (iter != nullptr) {
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

template <typename KeyT, typename ValueT, size_t MaxLevel>
inline bool operator == (const typename SkipList<KeyT, ValueT, MaxLevel>::iterator & iter, const std::nullptr_t & null_ptr) {
    return (iter.node_ == null_ptr);
}

template <typename KeyT, typename ValueT, size_t MaxLevel>
inline bool operator != (const typename SkipList<KeyT, ValueT, MaxLevel>::iterator & iter, const std::nullptr_t & null_ptr) {
    return (iter.node_ != null_ptr);
}

} // namespace TiStore
