#pragma once
#include <vector>
#include "utils/concept.hpp"
#include "utils/type_list.hpp"
#include "ecs/entity.hpp"

namespace mytho::ecs {
    template<mytho::utils::PureValueType... Ts>
    //requires (!std::disjunction_v<mytho::utils::is_entity_v<Ts>...>)
    struct mut {};

    template<mytho::utils::PureValueType... Ts>
    struct with {};

    template<mytho::utils::PureValueType... Ts>
    struct without {};
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_mut_v = internal::is_template_v<T, mytho::ecs::mut>;

    template<typename T>
    inline constexpr bool is_with_v = internal::is_template_v<T, mytho::ecs::with>;

    template<typename T>
    inline constexpr bool is_without_v = internal::is_template_v<T, mytho::ecs::without>;

    namespace internal {
        template<typename T>
        struct rm_mut {
            using type = type_list<const T&>;
        };

        template<typename T, typename U>
        struct rm_mut<mytho::ecs::basic_entity<T, U>> {
            using type = type_list<const mytho::ecs::basic_entity<T, U>>;
        };

        template<typename... Ts>
        struct rm_mut<mytho::ecs::mut<Ts...>> {
            using type = type_list<Ts&...>;
        };
    }

    template<typename T>
    using rm_mut_t = typename internal::rm_mut<T>::type;

    template<typename T>
    using rm_with_t = internal::rm_template_t<T, mytho::ecs::with>;

    template<typename T>
    using rm_without_t = internal::rm_template_t<T, mytho::ecs::without>;

    namespace internal {
        template<typename... Ts>
        struct datatype_list_convert;

        template<typename... Ts>
        struct datatype_list_convert<type_list<Ts...>> {
            using type = type_list_cat_t<rm_mut_t<Ts>...>;
        };
    }

    template<typename L>
    using datatype_list_convert_t = typename internal::datatype_list_convert<L>::type;

    namespace internal {
        template<typename L, template<typename...> typename R>
        struct prototype_list_convert;

        template<template<typename...> typename R>
        struct prototype_list_convert<type_list<>, R> {
            using type = type_list<>;
        };

        template<template<typename...> typename R, typename... Ts>
        struct prototype_list_convert<type_list<Ts...>, R> {
            using type = type_list_cat_t<internal::rm_template_t<Ts, R>...>;
        };
    }

    template<typename L, template<typename...> typename R>
    using prototype_list_convert_t = typename internal::prototype_list_convert<L, R>::type;

    template<typename T>
    concept QueryValueType = PureValueType<T>;

    template<typename T>
    concept PureComponentType = PureValueType<T> && !is_mut_v<T> && !is_with_v<T> && !is_without_v<T>;
}

namespace mytho::ecs {
    namespace internal {
        template<typename... Ts>
        using type_list = mytho::utils::type_list<Ts...>;

        template<typename... Ls>
        using type_list_cat_t = mytho::utils::type_list_cat_t<Ls...>;

        template<typename L, template<typename...> typename... Fs>
        using type_list_filter_template_t = mytho::utils::type_list_filter_template_t<L, Fs...>;

        template<typename L, template<typename...> typename E>
        using type_list_extract_template_t = mytho::utils::type_list_extract_template_t<L, E>;

        template<typename L, typename... Fs>
        using type_list_filter_t = mytho::utils::type_list_filter_t<L, Fs...>;

        template<typename L>
        using list_to_tuple_t = mytho::utils::list_to_tuple_t<L>;

        template<typename L>
        using datatype_list_convert_t = mytho::utils::datatype_list_convert_t<L>;

        template<typename L, template<typename...> typename R>
        using prototype_list_convert_t = mytho::utils::prototype_list_convert_t<L, R>;

        template<mytho::utils::QueryValueType... Ts>
        struct query_types {
            using query_list = internal::type_list<Ts...>;
            using require_list = internal::type_list_filter_template_t<query_list, with, without>;
            using require_prototype_list = internal::prototype_list_convert_t<require_list, mut>;
            using require_datatype_list = internal::datatype_list_convert_t<require_list>;
            using with_list = internal::type_list_extract_template_t<query_list, with>;
            using with_prototype_list = internal::prototype_list_convert_t<with_list, with>;
            using without_list = internal::type_list_extract_template_t<query_list, without>;
            using without_prototype_list = internal::prototype_list_convert_t<without_list, without>;
        };
    }

    template<typename RegistryT, mytho::utils::QueryValueType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_querier final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;
        using query_types = internal::query_types<Ts...>;
        using component_prototype_list = typename query_types::require_prototype_list;
        using component_datatype_list = typename query_types::require_datatype_list;
        using component_contain_list = internal::type_list_cat_t<internal::type_list_filter_t<component_prototype_list, entity_type>, typename query_types::with_prototype_list>;
        using component_not_contain_list = typename query_types::without_prototype_list;
        using component_bundle_type = typename internal::list_to_tuple_t<component_datatype_list>;
        using component_bundle_container_type = std::vector<component_bundle_type>;
        using size_type = typename component_bundle_container_type::size_type;
        using iterator = typename component_bundle_container_type::iterator;

    public:
        basic_querier(const component_bundle_container_type& component_bundles) : _component_bundles(component_bundles) {}

    public:
        size_type size() const noexcept { return _component_bundles.size(); }

        bool empty() const noexcept { return size() == 0; }

        iterator begin() noexcept { return _component_bundles.begin(); }

        iterator end() noexcept { return _component_bundles.end(); }

    private:
        component_bundle_container_type _component_bundles;
    };
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_querier_v = internal::is_template_v<T, mytho::ecs::basic_querier>;
}