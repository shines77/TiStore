#pragma once

#include "TimaxStore/fs/SuperBlock.h"

namespace timax {
namespace fs {

class MetaData {
private:
    bool inited_;
    SuperBlock super_block_;

public:
    MetaData() : inited_(false) { init(); }
    ~MetaData() { close(); }

    bool inited() const {
        return inited_;
    }

    static MetaData & get() {
        static MetaData meta;
        return meta;
    }

    bool init() {
        return open();
    }

    bool open() {
        bool success = false;
        if (!inited_) {
            success = open_meta();
            inited_ = success;
        }
        return success;
    }

    void close() {
        if (inited_) {
            flush();
            inited_ = false;
        }
    }

    void flush() {
        //
    }

    bool open_meta() {
        return true;
    }
};

} // namespace fs
} // namespace timax
