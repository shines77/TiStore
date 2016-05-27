#pragma once

namespace timax {
namespace fs {

class BlockDevice {
public:
    BlockDevice() {}
    virtual ~BlockDevice() {}

    bool mount() {
        return true;
    }
};

} // namespace fs
} // namespace timax
