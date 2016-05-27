#pragma once

#include "TiStore/fs/MetaData.h"

namespace timax {
namespace fs {

class Initor {
public:
    Initor() { create(); }
    ~Initor() { destroy(); }

    bool create() {
        MetaData & meta = MetaData::get();
        return meta.inited();
    }

    void destroy() {
        //
    }
};

} // namespace fs
} // namespace timax
