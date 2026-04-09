#pragma once
#include <optional>
#include <utility>
#include <type_traits>
#include <cstddef>

#include "core/concept.hpp"
#include "core/type_list.hpp"
#include "ecs/entity.hpp"

namespace mytho::ecs {
    template<typename T>
    concept PureResourceType = mytho::core::PureValueType<T>;

    template<PureResourceType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_resources final {
    public:
        using size_type = size_t;
        using resource_list_type = mytho::core::type_list<const internal::data_wrapper<Ts>...>;
        using resource_bundle_type = mytho::core::list_to_tuple_t<resource_list_type>;

        basic_resources(resource_bundle_type&& resources) noexcept : _resources(std::move(resources)) {}

    public:
        const resource_bundle_type& data() const noexcept {
            return _resources;
        }

        resource_bundle_type& data() noexcept {
            return _resources;
        }

    private:
        resource_bundle_type _resources;
    };

    template<PureResourceType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_resources_mut final {
    public:
        using size_type = size_t;
        using resource_list_type = mytho::core::type_list<internal::data_wrapper<Ts>...>;
        using resource_bundle_type = mytho::core::list_to_tuple_t<resource_list_type>;

        basic_resources_mut(resource_bundle_type&& resources) noexcept : _resources(std::move(resources)) {}

    public:
        const resource_bundle_type& data() const noexcept {
            return _resources;
        }

        resource_bundle_type& data() noexcept {
            return _resources;
        }

    private:
        resource_bundle_type _resources;
    };

    template<typename T>
    inline constexpr bool is_resources_v = mytho::core::is_template_v<T, basic_resources>;

    template<typename T>
    inline constexpr bool is_resources_mut_v = mytho::core::is_template_v<T, basic_resources_mut>;
}

// `Structured Binding`'s imple for `basic_resources` and `basic_resource_mut`
namespace std {
    template <typename... Ts>
    struct tuple_size<mytho::ecs::basic_resources<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

    template <size_t I, typename... Ts>
    struct tuple_element<I, mytho::ecs::basic_resources<Ts...>> {
        using type = typename std::tuple_element<I, std::tuple<const mytho::ecs::internal::data_wrapper<Ts>...>>::type;
    };

    template <typename... Ts>
    struct tuple_size<mytho::ecs::basic_resources_mut<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

    template <size_t I, typename... Ts>
    struct tuple_element<I, mytho::ecs::basic_resources_mut<Ts...>> {
        using type = typename std::tuple_element<I, std::tuple<mytho::ecs::internal::data_wrapper<Ts>...>>::type;
    };
}

namespace mytho::ecs {
    template <size_t I, typename... Ts>
    decltype(auto) get(basic_resources<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const basic_resources<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(basic_resources<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const basic_resources<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(basic_resources_mut<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const basic_resources_mut<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(basic_resources_mut<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const basic_resources_mut<Ts...>&& res) {
        return std::get<I>(res.data());
    }
}