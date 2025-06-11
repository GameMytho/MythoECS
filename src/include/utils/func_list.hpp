#pragma once
#include "type_list.hpp"

namespace mytho::utils::internal {
    template<typename T, template<auto...> typename R>
    struct is_location_t : std::false_type {};

    template<template<auto...> typename R, auto... Fs>
    struct is_location_t<R<Fs...>, R> : std::true_type {};

    template<typename T, template<auto...> typename R>
    inline constexpr bool is_location_v = is_location_t<T, R>::value;
}

namespace mytho::utils {
    template<auto... Ts>
    struct func_list {};

    namespace internal {
        template<typename... Ls>
        struct func_list_cat;

        template<>
        struct func_list_cat<> {
            using type = func_list<>;
        };

        template<auto... Funcs>
        struct func_list_cat<func_list<Funcs...>> {
            using type = func_list<Funcs...>;
        };

        template<auto... Funcs1, auto... Funcs2, typename... Ls>
        struct func_list_cat<func_list<Funcs1...>, func_list<Funcs2...>, Ls...> {
            using type = typename func_list_cat<func_list<Funcs1..., Funcs2...>, Ls...>::type;
        };
    }

    template<typename... Ls>
    using func_list_cat_t = typename internal::func_list_cat<Ls...>::type;

    namespace internal {
        template<typename L, template<auto...> typename E>
        struct func_list_extract;

        template<template<auto...> typename E>
        struct func_list_extract<type_list<>, E> {
            using type = type_list<>;
        };

        template<template<auto...> typename E, typename T, typename... Rs>
        struct func_list_extract<type_list<T, Rs...>, E> {
            using type = std::conditional_t<
                is_location_v<T, E>,
                type_list_cat_t<type_list<T>, typename func_list_extract<type_list<Rs...>, E>::type>,
                typename func_list_extract<type_list<Rs...>, E>::type
            >;
        };
    }

    template<typename L, template<auto...> typename E>
    using func_list_extract_t = typename internal::func_list_extract<L, E>::type;
}

namespace mytho::utils::internal {
    template<typename T, template<auto...> typename R>
    struct rm_location;

    template<template<auto...> typename R, auto... Ts>
    struct rm_location<R<Ts...>, R> {
        using type = func_list<Ts...>;
    };

    template<typename T, template<auto...> typename R>
    using rm_location_t = typename internal::rm_location<T, R>::type;
}