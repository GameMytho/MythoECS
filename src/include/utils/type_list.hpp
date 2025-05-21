#pragma once
#include <tuple>

namespace mytho::utils {
    template<typename... Ts>
    struct type_list {};

    namespace interal {
        template<typename T>
        struct is_type_list_t : std::false_type {};

        template<typename... Ts>
        struct is_type_list_t<type_list<Ts...>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_type_list_v = is_type_list_t<T>::value;
    }

    template<typename T>
    concept ListType = interal::is_type_list_v<T>;

    template<typename... Ts>
    struct type_list_cat;

    template<typename... Ts>
    struct type_list_cat<type_list<Ts...>> {
        using list = type_list<Ts...>;
    };

    template<typename... Ts1, typename... Ts2, typename... Rs>
    struct type_list_cat<type_list<Ts1...>, type_list<Ts2...>, Rs...> {
        using list = typename type_list_cat<type_list<Ts1..., Ts2...>, Rs...>::list;
    };

    template<typename T>
    struct list_to_tuple;

    template<typename... Ts>
    struct list_to_tuple<type_list<Ts...>> {
        using tuple = std::tuple<Ts...>;
    };
}