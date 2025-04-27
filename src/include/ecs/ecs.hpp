#pragma once

namespace mytho::ecs {
    template<typename T>
    concept IntegralType = std::is_integral_v<T>;

    template<IntegralType T>
    inline int add(T a, T b) {
        return a + b;
    }
}
