#pragma once

#include "TiStore/fs/Common.h"
#include "TiStore/fs/SuperBlock.h"
#include "TiStore/fs/ErrorCode.h"

#include <string.h>
#include <assert.h>
#include <string>
#include <map>

namespace TiStore {
namespace fs {

class File;

struct Inode {
    int32_t  fragment_id;
    uint32_t offset_;
    uint32_t length_;
    uint32_t capacity_;
    uint32_t mods;
    uint32_t flags;
    uint32_t stats;
    uint64_t last_access;
    uint64_t last_modified;
    // above is 44 bytes
    uint32_t reserve[(64 - 44) / sizeof(uint32_t)];
    uint32_t name_len;
    char     name[MAX_PATH];

    void init(int32_t frag_id = -1) {
        fragment_id = frag_id;
        offset_ = 0;
        length_ = 0;
        capacity_ = 0;
        mods = 0;
        flags = 0;
        stats = 0;
        last_access = 0;
        last_modified = 0;
    }

    void set_name(const char * filename, size_t name_len) {
        assert(filename != nullptr);
        assert(name_len <= sizeof(name) - 1);
        this->name_len = (uint32_t)name_len;
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
        errno_t err = ::strncpy_s(name, filename, name_len + 1);
#else
        ::strncpy(name, filename, name_len + 1);
#endif
    }

    void set_name(const char * filename) {
        assert(filename != nullptr);
        size_t name_len = ::strlen(filename);
        set_name(filename, name_len);
    }
};

typedef std::map<std::string, Inode *>  InodeMap;
typedef InodeMap::iterator              inodemap_iterator;
typedef InodeMap::const_iterator        const_inodemap_iterator;

class MetaData {
private:
    bool inited_;
    SuperBlock super_block_;

    InodeMap inodes_;

    friend class File;

public:
    MetaData() : inited_(false) { init(); }
    ~MetaData() { destroy(); }

    bool inited() const {
        return inited_;
    }

    static MetaData & get() {
        static MetaData meta;
        return meta;
    }

    bool init() {
        return create();
    }

    bool create() {
        bool success = false;
        if (!inited_) {
            success = load_meta();
            inited_ = success;
        }
        return success;
    }

    void destroy() {
        if (inited_) {
            flush_meta();
            inited_ = false;
        }
    }

    void flush() {
        flush_meta();
    }

    bool load_meta() {
        return true;
    }

    bool flush_meta() {
        return true;
    }

    Inode * open_file(File * file, const char * filename, int & err_code) {
        assert(file != nullptr);
        Inode * fd = nullptr;
        const_inodemap_iterator inode_iter = inodes_.find(filename);
        if (inode_iter != inodes_.end()) {
            // file or directory is exists.
            fd = inode_iter->second;
            err_code = error_code::no_error;
        }
        else {
            // file or directory is not exists.
            Inode * inode = new Inode();
            fd = inode;
            if (inode != nullptr) {
                inode->init();
                inode->set_name(filename, ::strlen(filename));
                inodes_.insert(std::make_pair(std::string(filename), inode));
                err_code = error_code::no_error;
            }
            else {
                err_code = error_code::out_of_memory;
            }
        }
        return fd;
    }
};

} // namespace fs
} // namespace TiStore
