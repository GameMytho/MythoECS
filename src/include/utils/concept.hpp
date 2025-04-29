#pragma once

namespace mytho::utils {
    template<typename T>
    concept UnsignedIntegralType = std::is_integral_v<T> && std::is_unsigned_v<T>;
}