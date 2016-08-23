#pragma once

namespace TiStore {

class error_code {
public:
    enum {
        error_first,
        out_of_memory = -2,
        err_failed = -1,
        no_error = 0,
        err_success = no_error,
        error_last,
        error_max = error_last
    };
};

} // namespace TiStore
