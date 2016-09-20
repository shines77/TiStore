#pragma once

#define CXX_SUPPORT_CONSTEXPR   1
#define CXX_SUPPORT_NOEXCEPT    1

#if defined(CXX_SUPPORT_CONSTEXPR) && (CXX_SUPPORT_CONSTEXPR != 0)
#define __CONST_EXPR    constexpr
#define __CONST_FUNC    constexpr
#else
#define __CONST_EXPR    const
#define __CONST_FUNC
#endif

#if defined(CXX_SUPPORT_NOEXCEPT) && (CXX_SUPPORT_NOEXCEPT != 0)
#define __NOEXCEPT      noexcept
#else
#define __NOEXCEPT
#endif
