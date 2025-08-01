#pragma once
#include <type_traits>

namespace mytho::utils {
    template<typename T>
    concept UnsignedIntegralType = std::is_integral_v<T> && std::is_unsigned_v<T>;

    template<typename T>
    concept PureValueType = !std::is_const_v<T> && !std::is_reference_v<T> && !std::is_pointer_v<T>;
}