#pragma once
#include <type_traits>
#include <tuple>

namespace mytho::utils::internal {
    template<typename T, template<typename...> typename R>
    struct is_template_t : std::false_type {};

    template<template<typename...> typename R, typename... Ts>
    struct is_template_t<R<Ts...>, R> : std::true_type {};

    template<typename T, template<typename...> typename R>
    inline constexpr bool is_template_v = is_template_t<T, R>::value;
}

namespace mytho::utils {
    template<typename... Ts>
    struct type_list {
        inline static constexpr size_t size = sizeof...(Ts);
    };

    template<typename L>
    inline constexpr bool is_type_list_v = internal::is_template_v<L, type_list>;

    namespace internal {
        template<typename... Ls>
        struct type_list_cat;

        template<>
        struct type_list_cat<> {
            using type = type_list<>;
        };

        template<typename... Ts>
        struct type_list_cat<type_list<Ts...>> {
            using type = type_list<Ts...>;
        };

        template<typename... Ts1, typename... Ts2, typename... Ls>
        struct type_list_cat<type_list<Ts1...>, type_list<Ts2...>, Ls...> {
            using type = typename type_list_cat<type_list<Ts1..., Ts2...>, Ls...>::type;
        };
    }

    template<typename... Ls>
    using type_list_cat_t = typename internal::type_list_cat<Ls...>::type;

    namespace internal {
        template<typename L, typename... Fs>
        struct type_list_filter;

        template<typename... Fs>
        struct type_list_filter<type_list<>, Fs...> {
            using type = type_list<>;
        };

        template<typename T, typename... Rs, typename... Fs>
        struct type_list_filter<type_list<T, Rs...>, Fs...> {
            using type = std::conditional_t<
                std::disjunction_v<std::is_same<T, Fs>...>,
                typename type_list_filter<type_list<Rs...>, Fs...>::type,
                type_list_cat_t<type_list<T>, typename type_list_filter<type_list<Rs...>, Fs...>::type>
            >;
        };
    }

    template<typename L, typename... Fs>
    using type_list_filter_t = typename internal::type_list_filter<L, Fs...>::type;

    namespace internal {
        template<typename L, template<typename...> typename... Fs>
        struct type_list_filter_template;

        template<template<typename...> typename... Fs>
        struct type_list_filter_template<type_list<>, Fs...> {
            using type = type_list<>;
        };

        template<typename T, typename... Rs, template<typename...> typename... Fs>
        struct type_list_filter_template<type_list<T, Rs...>, Fs...> {
            using type = std::conditional_t<
                std::disjunction_v<is_template_t<T, Fs>...>,
                typename type_list_filter_template<type_list<Rs...>, Fs...>::type,
                type_list_cat_t<type_list<T>, typename type_list_filter_template<type_list<Rs...>, Fs...>::type>
            >;
        };
    }

    template<typename L, template<typename...> typename... Fs>
    using type_list_filter_template_t = typename internal::type_list_filter_template<L, Fs...>::type;

    namespace internal {
        template<typename L, template<typename...> typename E>
        struct type_list_extract_template;

        template<template<typename...> typename E>
        struct type_list_extract_template<type_list<>, E> {
            using type = type_list<>;
        };

        template<template<typename...> typename E, typename T, typename... Rs>
        struct type_list_extract_template<type_list<T, Rs...>, E> {
            using type = std::conditional_t<
                is_template_v<T, E>,
                type_list_cat_t<type_list<T>, typename type_list_extract_template<type_list<Rs...>, E>::type>,
                typename type_list_extract_template<type_list<Rs...>, E>::type
            >;
        };
    }

    template<typename L, template<typename...> typename E>
    using type_list_extract_template_t = typename internal::type_list_extract_template<L, E>::type;

    namespace internal {
        template<typename L>
        struct list_to_tuple;

        template<typename... Ts>
        struct list_to_tuple<type_list<Ts...>> {
            using type = std::tuple<Ts...>;
        };
    }

    template<typename L>
    using list_to_tuple_t = typename internal::list_to_tuple<L>::type;
}

namespace mytho::utils::internal {
    template<typename T, template<typename...> typename R>
    struct rm_template {
        using type = type_list<T>;
    };

    template<template<typename...> typename R, typename... Ts>
    struct rm_template<R<Ts...>, R> {
        using type = type_list<Ts...>;
    };

    template<typename T, template<typename...> typename R>
    using rm_template_t = typename rm_template<T, R>::type;
}