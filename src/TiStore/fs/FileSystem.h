#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/MetaData.h"

#if defined(_WIN32) || defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef MAX_PATH
#ifndef PATH_MAX
#define MAX_PATH    260
#else
#define MAX_PATH    PATH_MAX
#endif // PATH_MAX
#endif // MAX_PATH

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define std_strcpy  strcpy_s
#else
#define std_strcpy  strcpy
#endif

namespace timax {
namespace fs {

enum fs_mask_t {
    FSM_NONE        = 0,
    FSM_READ        = 1,
    FSM_WRITE       = 2,
    FSM_APPEND      = 4,
    FSM_APPENDTOEND = 8,
    FSM_TRUNC       = 16,
    FSM_BINARY      = 0x00008000UL,
    FSM_DIRECTORY   = 0x40000000UL,
    FSM_READ_WRITE  = FSM_READ | FSM_WRITE,
    FSM_DEFAULT     = FSM_READ | FSM_WRITE | FSM_APPEND | FSM_BINARY,
    FSM_MAX         = 0xFFFFFFFFUL
};

enum fs_stat_t {
    FST_NONE    = 0,
    FST_OPEN    = 1,
    FST_MAX     = 0xFFFFFFFF
};

#ifndef FS_REMOVE_MASK
#define FS_REMOVE_MASK(val, mask, return_type, mask_type) \
        ((return_type)((val) & (~(mask_type)(mask))))
#endif

#ifndef FS_ADD_MASK
#define FS_ADD_MASK(val, mask, return_type, mask_type) \
        ((return_type)((val) | ((mask_type)(mask))))
#endif

#if defined(_WIN32) || defined(WIN32)
typedef HANDLE      native_fd;
#define NULL_FD     NULL
#else
typedef int         native_fd;
#define NULL_FD     0
#endif

class File {
private:
    native_fd fd_;
    int id_;
    uint32_t flag_;
    uint32_t mode_;
    size_t offset_;
    size_t size_;
    size_t capacity_;
    size_t fragment_size_;
    char filename_[MAX_PATH];
    char fragment_[MAX_PATH];

public:
    File() : fd_(NULL_FD), id_(-1), flag_(FST_NONE), mode_(FSM_DEFAULT),
        offset_(0), size_(0), capacity_(0), fragment_size_(0) {
        std_strcpy(filename_, "");
        std_strcpy(fragment_, "");
    }

    File(const char * filename) : File() {
        std_strcpy(filename_, filename);
    }

    virtual ~File() {
        close();
    }

    bool is_open() {
        return ((flag_ | FST_OPEN) != 0);
    }

    bool open(const char * filename, int mode = FSM_DEFAULT) {
        return true;
    }

    bool close() {
        flag_ = FS_REMOVE_MASK(flag_, FST_OPEN, uint32_t, uint32_t);
        return true;
    }

    bool isDirectory() const {
        return ((flag_ & FSM_DIRECTORY) != 0);
    }
};

struct Directory {
    //
};

} // namespace fs
} // namespace timax
