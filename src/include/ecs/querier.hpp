#pragma once
#include <vector>
#include "utils/concept.hpp"
#include "utils/type_list.hpp"

namespace mytho::ecs {
    template<mytho::utils::PureValueType... Ts>
    struct mut {};
}

namespace mytho::utils {
    namespace interal {
        template<typename T>
        struct is_mut_t : std::false_type {};

        template<typename... Ts>
        struct is_mut_t<mytho::ecs::mut<Ts...>> : std::true_type {};

        template<typename T>
        inline constexpr bool is_mut_v = is_mut_t<T>::value;

        template<typename T>
        struct rm_mut {
            using prototype_list = type_list<T>;
            using datatype_list = type_list<const T&>;
        };

        template<typename... Ts>
        struct rm_mut<mytho::ecs::mut<Ts...>> {
            using prototype_list = type_list<Ts...>;
            using datatype_list = type_list<Ts&...>;
        };
    }

    template<typename... Ts>
    using convert_to_datatype_list = typename type_list_cat<typename interal::rm_mut<Ts>::datatype_list...>::list;

    template<typename... Ts>
    using convert_to_prototype_list = typename type_list_cat<typename interal::rm_mut<Ts>::prototype_list...>::list;

    template<typename T>
    concept PureComponentType = PureValueType<T> && !interal::is_mut_v<T>;

    template<typename T>
    concept QueryValueType = PureValueType<T>;
}

namespace mytho::ecs {
    template<typename RegistryT, typename... Ts>
    class basic_querier final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;
        using component_datatype_list = mytho::utils::convert_to_datatype_list<Ts...>;
        using component_bundle_type = typename mytho::utils::list_to_tuple<component_datatype_list>::tuple;
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