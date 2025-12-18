#pragma once
#include <optional>
#include <utility>
#include <type_traits>
#include <cstddef>

#include "utils/concept.hpp"
#include "utils/type_list.hpp"
#include "ecs/entity.hpp"

namespace mytho::utils {
    template<typename T>
    concept PureResourceType = PureValueType<T>;
}

namespace mytho::ecs {
    // namespace internal {
    //     template<mytho::utils::PureResourceType T>
    //     class basic_resource_cache final {
    //     public:
    //         using data_type = std::optional<T>;
    //         using data_reference_type = data_type&;

    //         basic_resource_cache(data_reference_type res) = delete;
    //         data_reference_type operator=(data_reference_type res) = delete;

    //     public:
    //         template<typename... Ts>
    //         static void init(Ts&&... ts) noexcept {
    //             if (_res) {
    //                 return;
    //             }

    //             _res = std::make_optional(T{std::forward<Ts>(ts)...});
    //         }

    //         static data_reference_type instance() noexcept {
    //             return _res;
    //         }

    //         static void destroy() noexcept {
    //             _res.reset();
    //         }

    //     private:
    //         basic_resource_cache() = default;
    //         ~basic_resource_cache() = default;

    //         inline static data_type _res = std::nullopt;
    //     };
    // }

    template<mytho::utils::PureResourceType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_resources final {
    public:
        using size_type = size_t;
        using resource_list_type = mytho::utils::type_list<const mytho::utils::internal::data_wrapper<Ts>...>;
        using resource_bundle_type = mytho::utils::list_to_tuple_t<resource_list_type>;

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

    template<mytho::utils::PureResourceType... Ts>
    requires (sizeof...(Ts) > 0)
    class basic_resources_mut final {
    public:
        using size_type = size_t;
        using resource_list_type = mytho::utils::type_list<mytho::utils::internal::data_wrapper<Ts>...>;
        using resource_bundle_type = mytho::utils::list_to_tuple_t<resource_list_type>;

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
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_resources_v = internal::is_template_v<T, mytho::ecs::basic_resources>;

    template<typename T>
    inline constexpr bool is_resources_mut_v = internal::is_template_v<T, mytho::ecs::basic_resources_mut>;
}

// `Structured Binding`'s imple for `basic_resources` and `basic_resource_mut`
namespace std {
    template <typename... Ts>
    struct tuple_size<mytho::ecs::basic_resources<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

    template <size_t I, typename... Ts>
    struct tuple_element<I, mytho::ecs::basic_resources<Ts...>> {
        using type = typename std::tuple_element<I, std::tuple<const mytho::utils::internal::data_wrapper<Ts>...>>::type;
    };

    template <typename... Ts>
    struct tuple_size<mytho::ecs::basic_resources_mut<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> {};

    template <size_t I, typename... Ts>
    struct tuple_element<I, mytho::ecs::basic_resources_mut<Ts...>> {
        using type = typename std::tuple_element<I, std::tuple<mytho::utils::internal::data_wrapper<Ts>...>>::type;
    };
}

namespace mytho::ecs {
    template <size_t I, typename... Ts>
    decltype(auto) get(mytho::ecs::basic_resources<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const mytho::ecs::basic_resources<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(mytho::ecs::basic_resources<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const mytho::ecs::basic_resources<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(mytho::ecs::basic_resources_mut<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const mytho::ecs::basic_resources_mut<Ts...>& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(mytho::ecs::basic_resources_mut<Ts...>&& res) {
        return std::get<I>(res.data());
    }

    template <size_t I, typename... Ts>
    decltype(auto) get(const mytho::ecs::basic_resources_mut<Ts...>&& res) {
        return std::get<I>(res.data());
    }
}