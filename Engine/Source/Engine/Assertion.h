#pragma once

#include <stdint.h>

#define USE_CUSTOM_ASSERT 1

#ifdef NDEBUG
#define OCT_ASSERT(expr) 
#elif USE_CUSTOM_ASSERT
void SYS_Assert(const char* exprString, const char* fileString, uint32_t lineNumber);
#define OCT_ASSERT(expr) (void)(                            \
            (!!(expr)) ||                                   \
            (SYS_Assert(#expr, __FILE__, __LINE__), 0) \
        )
#else
#include <assert.h>
#define OCT_ASSERT(expr) assert(expr)
#endif
