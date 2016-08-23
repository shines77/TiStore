#pragma once

#include "TiStore/basic/cstdint"
#include <string>

namespace TiStore {
namespace fs {

class BlockDevice {
private:
    std::string name_;
    std::string root_;
    std::size_t capacity_;
    std::size_t block_size_;

public:
    BlockDevice(const char * name) : name_(name), root_(""),
        capacity_(0), block_size_(0) {}
    virtual ~BlockDevice() {}

    bool mount() {
        return true;
    }
};

} // namespace fs
} // namespace TiStore
