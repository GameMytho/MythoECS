#pragma once
#include <type_traits>

#include "utils/concept.hpp"

namespace mytho::ecs {
    template<mytho::utils::UnsignedIntegralType EntityIdentityT, mytho::utils::UnsignedIntegralType EntityVersionT>
    class basic_entity final {
    public:
        using id_type = EntityIdentityT;
        using version_type = EntityVersionT;
        using self_type = basic_entity<id_type, version_type>;

    public:
        basic_entity(id_type id, version_type ver = 0) : _id(id), _ver(ver) { }

        id_type id() const noexcept { return _id; }
        
        version_type version() const noexcept { return _ver; }

    private:
        id_type _id;
        version_type _ver;
    };
}

namespace mytho::utils {
    namespace internal {
        template<typename T>
        struct is_entity_t : std::false_type {};

        template<typename T, typename U>
        struct is_entity_t<mytho::ecs::basic_entity<T, U>> : std::true_type {};
    }

    template<typename T>
    inline constexpr bool is_entity_v = internal::is_entity_t<T>::value;

    template<typename T>
    concept EntityType = is_entity_v<T>;
}