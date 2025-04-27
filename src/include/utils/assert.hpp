#pragma once
#include <cassert>

#ifndef ASSERT
    #define ASSERT(x, msg) \
        do { \
            if ((x)) { \
                assert(false); \
            } \
        } while(false)
#endif

#ifndef ASSURE
    #define ASSURE(x, msg) ASSERT((!(x)), msg)
#endif