#pragma once

namespace timax {

class error_code {
public:
    enum {
        error_first,
        err_failed = -1,
        no_error = 0,
        err_success = no_error,
        error_last,
        error_max = error_last
    };
};

} // namespace timax
