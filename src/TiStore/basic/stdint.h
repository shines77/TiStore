#pragma once

#include <stdint.h>
#include <stddef.h>

#if defined(__linux__) || defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
#include <sys/types.h>  // For ssize_t
#else
#include "TiStore/basic/ssize.h"
#endif
