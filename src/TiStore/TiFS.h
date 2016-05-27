#pragma once

#include "TiStore/basic/cstdint"
#include "TiStore/fs/FileSystem.h"

namespace timax {

namespace fs {
class BlockDevice;
}

class GUID;

class TiFS {
private:
    uint32_t page_size_;
    uint32_t block_size_;

public:
    TiFS() {}
    ~TiFS() {}

    int add_device(fs::BlockDevice * device);

    int make_fs(const GUID & uuid) {
        // 
    }

    int make_fs(const char * uuid) {
        // 
    }

    std::ssize_t open(const char * filename, uint32_t mode) {
        return 0;
    }

    std::ssize_t read(char * buf, std::size_t len) {
        return 0;
    }

    std::ssize_t write(char * buf, std::size_t len) {
        return 0;
    }
};

} // namespace timax
