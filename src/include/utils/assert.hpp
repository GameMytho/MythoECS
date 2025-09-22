#pragma once

#if MYTHO_ASSERTS_ENABLED
    // this is a temporary solution, we need to implement a better assert macro
    #include <cassert>
    #define ASSURE(x, msg) do { if (!(x)) { assert(false); }} while(false)
#else
    #define ASSURE(x, msg)
#endif