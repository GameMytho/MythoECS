#pragma once
#include <bit>
#include <limits>

#include "utils/concept.hpp"

namespace mytho::ecs {
    template<mytho::utils::UnsignedIntegralType EntityIdentityT, mytho::utils::UnsignedIntegralType EntityVersionT>
    class entity {
    public:
        using id_type = EntityIdentityT;
        using version_type = EntityVersionT;
        using self_type = entity<id_type, version_type>;

    public:
        entity(id_type id, version_type ver = 0) : _id(id), _ver(ver) { }

        id_type id() const noexcept { return _id; }
        
        version_type version() const noexcept { return _ver; }

    private:
        id_type _id;
        version_type _ver;

    private:
        self_type version_next () noexcept { _ver++; return *this; }

    };
}

namespace mytho::utils {
    namespace interal {
        template<typename T>
        struct is_entity_v : std::false_type {};

        template<typename T, typename U>
        struct is_entity_v<mytho::ecs::entity<T, U>> : std::true_type {};
    }

    template<typename T>
    concept EntityType = interal::is_entity_v<T>::value;
}