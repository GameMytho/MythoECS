#pragma once
#include <vector>
#include <cstdint>

#include "core/type_list.hpp"
#include "ecs/entity.hpp"

namespace mytho::ecs::internal {
    template<typename... Ts>
    using type_list = mytho::core::type_list<Ts...>;

    template<typename... Ls>
    using type_list_cat_t = mytho::core::type_list_cat_t<Ls...>;

    template<typename L, template<typename...> typename... Fs>
    using type_list_filter_template_t = mytho::core::type_list_filter_template_t<L, Fs...>;

    template<typename L, template<typename...> typename E>
    using type_list_extract_template_t = mytho::core::type_list_extract_template_t<L, E>;

    template<typename L, typename... Fs>
    using type_list_filter_t = mytho::core::type_list_filter_t<L, Fs...>;

    template<typename L>
    using list_to_tuple_t = mytho::core::list_to_tuple_t<L>;

    template<typename T, template<typename...> typename R>
    using rm_template_t = mytho::core::rm_template_t<T, R>;
}

namespace mytho::ecs {
    template<mytho::core::PureValueType... Ts>
    struct mut {};

    template<mytho::core::PureValueType... Ts>
    struct with {};

    template<mytho::core::PureValueType... Ts>
    struct without {};

    template<mytho::core::PureValueType... Ts>
    struct added {};

    template<mytho::core::PureValueType... Ts>
    struct changed {};

    template<typename T>
    inline constexpr bool is_mut_v = mytho::core::is_template_v<T, mut>;

    template<typename T>
    inline constexpr bool is_with_v = mytho::core::is_template_v<T, with>;

    template<typename T>
    inline constexpr bool is_without_v = mytho::core::is_template_v<T, without>;

    template<typename T>
    inline constexpr bool is_added_v = mytho::core::is_template_v<T, added>;

    template<typename T>
    inline constexpr bool is_changed_v = mytho::core::is_template_v<T, changed>;

    template<typename T>
    concept QueryValueType = mytho::core::PureValueType<T>;

    template<typename T>
    concept PureComponentType = mytho::core::PureValueType<T> && !is_mut_v<T> && !is_with_v<T> && !is_without_v<T> && !is_added_v<T> && !is_changed_v<T>;

    namespace internal {
        template<typename T>
        struct rm_mut {
            using type = type_list<const data_wrapper<T>>;
        };

        template<typename T, typename U>
        struct rm_mut<basic_entity<T, U>> {
            using type = type_list<const data_wrapper<basic_entity<T, U>>>;
        };

        template<typename... Ts>
        struct rm_mut<mut<Ts...>> {
            using type = type_list<data_wrapper<Ts>...>;
        };
    }

    template<typename T>
    using rm_mut_t = typename internal::rm_mut<T>::type;

    template<typename T>
    using rm_with_t = internal::rm_template_t<T, with>;

    template<typename T>
    using rm_without_t = internal::rm_template_t<T, without>;

    template<typename T>
    using rm_changed_t = internal::rm_template_t<T, changed>;

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

    namespace internal {
        template<QueryValueType... Ts>
        struct query_types {
            using query_list = type_list<Ts...>;
            using require_list = type_list_filter_template_t<query_list, with, without, added, changed>;
            using require_prototype_list = prototype_list_convert_t<require_list, mut>;
            using require_datatype_list = datatype_list_convert_t<require_list>;
            using with_list = type_list_extract_template_t<query_list, with>;
            using with_prototype_list = prototype_list_convert_t<with_list, with>;
            using without_list = type_list_extract_template_t<query_list, without>;
            using without_prototype_list = prototype_list_convert_t<without_list, without>;
            using added_list = type_list_extract_template_t<query_list, added>;
            using added_prototype_list = prototype_list_convert_t<added_list, added>;
            using changed_list = type_list_extract_template_t<query_list, changed>;
            using changed_prototype_list = prototype_list_convert_t<changed_list, changed>;
        };
    }

    template<typename RegistryT, QueryValueType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_querier final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;
        using query_types = internal::query_types<Ts...>;
        using component_prototype_list = typename query_types::require_prototype_list;
        using component_datatype_list = typename query_types::require_datatype_list;
        using component_added_list = typename query_types::added_prototype_list;
        using component_changed_list = typename query_types::changed_prototype_list;
        using component_contain_list = internal::type_list_cat_t<internal::type_list_filter_t<component_prototype_list, entity_type>, typename query_types::with_prototype_list, component_added_list, component_changed_list>;
        using component_not_contain_list = typename query_types::without_prototype_list;
        using component_bundle_type = typename internal::list_to_tuple_t<component_datatype_list>;
        using component_bundle_container_type = std::vector<component_bundle_type>;
        using size_type = typename component_bundle_container_type::size_type;
        using iterator = typename component_bundle_container_type::iterator;

        basic_querier(component_bundle_container_type&& component_bundles) noexcept : _component_bundles(std::move(component_bundles)) {}

    public:
        size_type size() const noexcept { return _component_bundles.size(); }

        bool empty() const noexcept { return size() == 0; }

        iterator begin() noexcept { return _component_bundles.begin(); }

        iterator end() noexcept { return _component_bundles.end(); }

    private:
        component_bundle_container_type _component_bundles;
    };

    template<typename RegistryT, PureComponentType T>
    class basic_removed_entities final {
    public:
        using registry_type = RegistryT;
        using entities_type = std::vector<typename registry_type::entity_type>;
        using size_type = typename entities_type::size_type;
        using iterator = typename entities_type::iterator;
        using const_iterator = typename entities_type::const_iterator;

        basic_removed_entities(entities_type& entities) noexcept : _entities(entities) {}

    public:
        iterator begin() noexcept { return _entities.begin(); }
        iterator end() noexcept { return _entities.end(); }

        const_iterator begin() const noexcept { return _entities.begin(); }
        const_iterator end() const noexcept { return _entities.end(); }

        size_type size() const noexcept { return _entities.size(); }

        bool empty() const noexcept { return size() == 0; }

    private:
        entities_type& _entities;
    };

    template<typename T>
    inline constexpr bool is_querier_v = mytho::core::is_template_v<T, basic_querier>;

    template<typename T>
    inline constexpr bool is_removed_entities_v = mytho::core::is_template_v<T, basic_removed_entities>;
}