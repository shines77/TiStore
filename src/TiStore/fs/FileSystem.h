#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/Common.h"
#include "TiStore/fs/MetaData.h"

namespace TiStore {
namespace fs {

enum fs_mask_t {
    FS_MARK_NONE        = 0,
    FS_MARK_READ        = 1,
    FS_MARK_WRITE       = 2,
    FS_MARK_APPEND      = 4,
    FS_MARK_APPENDTOEND = 8,
    FS_MARK_TRUNC       = 16,
    FS_MARK_BINARY      = 0x00008000UL,
    FS_MARK_DIRECTORY   = 0x40000000UL,
    FS_MARK_READ_WRITE  = FS_MARK_READ | FS_MARK_WRITE,
    FS_MARK_DEFAULT     = FS_MARK_READ | FS_MARK_WRITE | FS_MARK_APPEND | FS_MARK_BINARY,
    FS_MARK_MAX         = 0xFFFFFFFFUL
};

enum fs_stat_t {
    FS_STAT_UNKNOWN    = 0,
    FS_STAT_OPEN    = 1,
    FS_STAT_MAX     = 0xFFFFFFFF
};

#ifndef FS_REMOVE_MASK
#define FS_REMOVE_MASK(val, mask, return_type, mask_type) \
        ((return_type)((val) & (~(mask_type)(mask))))
#endif

#ifndef FS_ADD_MASK
#define FS_ADD_MASK(val, mask, return_type, mask_type) \
        ((return_type)((val) | ((mask_type)(mask))))
#endif

typedef Inode * tsfs_fd;

class File {
private:
    native_fd native_fd_;
    tsfs_fd fd_;
    int id_;
    bool create_new_;
    uint32_t flag_;
    uint32_t mode_;
    size_t offset_;
    size_t size_;
    size_t capacity_;
    size_t fragment_size_;
    char filename_[MAX_PATH];
    char fragment_[MAX_PATH];

public:
    File() : native_fd_(null_fd), fd_(nullptr), id_(-1), create_new_(true),
        flag_(FS_STAT_UNKNOWN), mode_(FS_MARK_DEFAULT),
        offset_(0), size_(0), capacity_(0), fragment_size_(0) {
        std_strcpy(filename_, "");
        std_strcpy(fragment_, "");
    }

    File(const char * filename, int mode = FS_MARK_DEFAULT) : File() {
        std_strcpy(filename_, filename);
        open(filename, mode);
    }

    virtual ~File() {
        close();
    }

    bool is_open() {
        return ((flag_ | FS_STAT_OPEN) != 0);
    }

    bool open(const char * filename, int mode = FS_MARK_DEFAULT) {
        int err_code;
        if (fd_ == null_fd)
            fd_ = MetaData::get().open_file(this, filename, err_code);
        return true;
    }

    bool close() {
        flag_ = FS_REMOVE_MASK(flag_, FS_STAT_OPEN, uint32_t, uint32_t);
        return true;
    }

    bool is_file() const {
        return ((flag_ & FS_MARK_DIRECTORY) == 0);
    }

    bool is_directory() const {
        return ((flag_ & FS_MARK_DIRECTORY) != 0);
    }
};

struct Directory {
    //
};

} // namespace fs
} // namespace TiStore
