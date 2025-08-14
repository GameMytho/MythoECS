#pragma once

#if MYTHO_ASSERTS_ENABLED
    #include <cassert>
    #define ASSERT(x, msg) do { if ((x)) { assert(false); }} while(false)
    #define ASSURE(x, msg) ASSERT((!(x)), msg)
#else
    #define ASSERT(x, msg)
    #define ASSURE(x, msg)
#endif