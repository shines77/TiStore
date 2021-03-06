#pragma once

#include "TiStore/fs/Common.h"

#include <cstddef>

namespace TiStore {
namespace fs {

class SuperBlock {
private:
    bool inited_;

    uint64_t    version_;
    std::size_t fragment_id_;
    std::size_t offset_;
    std::size_t total_used_;
    std::size_t total_capacity_;
    std::size_t root_nodes_;

public:
    SuperBlock() : inited_(false), version_(TISTORE_VERSION),
        fragment_id_(0), offset_(0),
        total_used_(0), total_capacity_(0), root_nodes_(0) {
        init();
    }
    ~SuperBlock() { close(); }

    void init() {
        if (!inited_) {
            open();
            inited_ = true;
        }
    }

    bool open() {
        //
        return true;
    }

    void close() {
        if (inited_) {
            flush();
            inited_ = false;
        }
    }

    bool verify_version() {
        return (version_ == TISTORE_VERSION);
    }

    bool fsync() {
        return true;
    }

    bool flush() {
        return true;
    }
};

} // namespace fs
} // namespace TiStore

#undef MAKE_VERSION
