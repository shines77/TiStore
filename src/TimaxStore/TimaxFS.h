#pragma once

#include "TimaxStore/std_ssize.h"
#include "TimaxStore/fs/FileSystem.h"

namespace timax {

class TimaxFS {
private:
    //

public:
    TimaxFS() {}
    ~TimaxFS() {}

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
