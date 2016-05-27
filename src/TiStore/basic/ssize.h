#pragma once

#ifndef ssize_t_defined
#if (__WORDSIZE == 32)
typedef long ssize_t;
#elif (__WORDSIZE == 64)
typedef long long ssize_t;
#else
typedef ptrdiff_t ssize_t;
#endif
#define ssize_t_defined
#endif // !ssize_t_defined
