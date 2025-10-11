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

        friend bool operator==(const self_type& l, const self_type& r) noexcept { return l._id == r._id && l._ver == r._ver; }

        friend bool operator!=(const self_type& l, const self_type& r) noexcept { return !(l == r); }

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

namespace mytho::utils::internal {
    template<typename T>
    class data_wrapper {
    public:
        using data_type = T;

        data_wrapper(T* data, uint64_t& data_tick, uint64_t tick) : _data(data), _data_tick(data_tick), _tick(tick) {}

        const T* operator->() const noexcept { return _data; }
        const T& operator*() const noexcept { return *_data; }

        T* operator->() noexcept { _data_tick = _tick; return _data; }
        T& operator*() noexcept { _data_tick = _tick; return *_data; }

    private:
        T* _data = nullptr;
        uint64_t& _data_tick;
        uint64_t _tick = 0;
    };

    template<typename T, typename U>
    class data_wrapper<mytho::ecs::basic_entity<T, U>> {
    public:
        using data_type = mytho::ecs::basic_entity<T, U>;

        data_wrapper(data_type data) : _data(data) {}

        const data_type* operator->() const noexcept { return &_data; }
        const data_type& operator*() const noexcept { return _data; }

    private:
        data_type _data;
    };
}