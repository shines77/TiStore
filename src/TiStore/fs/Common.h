#pragma once

#if defined(_WIN32) || defined(WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef MAKE_VERSION
#define MAKE_VERSION(major, minor)  (((major) << 16) | (minor))
#endif
#define TISTORE_VERSION     MAKE_VERSION(0, 1)

#ifndef CACHE_LINE_SIZE
#define CACHE_LINE_SIZE     64
#endif

#ifndef MAX_PATH
#ifndef PATH_MAX
#define MAX_PATH    260
#else
#define MAX_PATH    PATH_MAX
#endif // PATH_MAX
#endif // MAX_PATH

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define std_strcpy      strcpy_s
#define std_strncpy     strncpy_s
#else
#define std_strcpy      std::strcpy
#define std_strncpy     std::strncpy
#endif

namespace TiStore {
namespace fs {

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
typedef HANDLE      native_fd;
#define NULL_FD     NULL
#define null_fd     NULL_FD
#else
typedef int         native_fd;
#define NULL_FD     0
#define null_fd     NULL_FD
#endif

} // namespace fs
} // namespace TiStore
