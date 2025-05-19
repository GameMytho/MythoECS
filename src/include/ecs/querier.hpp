#pragma once
#include <tuple>
#include <vector>

namespace mytho::ecs {
    template<typename T>
    struct mut {};
}

namespace mytho::utils {
    namespace interal {
        template<typename T>
        struct is_mut_t : std::false_type {};

        template<typename T>
        struct is_mut_t<mytho::ecs::mut<T>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_mut_v = is_mut_t<T>::value;

        template<typename T>
        struct rm_mut {
            using prototype = T;
            using data_type = const prototype&;
        };

        template<typename T>
        struct rm_mut<mytho::ecs::mut<T>> {
            using prototype = T;
            using data_type = prototype&;
        };
    }

    template<typename T>
    using convert_to_datatype = std::conditional_t<interal::is_mut_v<T>, typename interal::rm_mut<T>::data_type, const T&>;

    template<typename T>
    using convert_to_prototype = std::conditional_t<interal::is_mut_v<T>, typename interal::rm_mut<T>::prototype, T>;
}

namespace mytho::ecs {
    template<typename RegistryT, typename... Ts>
    class basic_querier final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;
        using component_bundle_type = std::tuple<mytho::utils::convert_to_datatype<Ts>...>;
        using component_bundle_container_type = std::vector<component_bundle_type>;
        using iterator = typename component_bundle_container_type::iterator;

    public:
        basic_querier(const component_bundle_container_type& component_bundles) : _component_bundles(component_bundles) {}

    public:
        iterator begin() noexcept { return _component_bundles.begin(); }

        iterator end() noexcept { return _component_bundles.end(); }

    private:
        component_bundle_container_type _component_bundles;
    };
}