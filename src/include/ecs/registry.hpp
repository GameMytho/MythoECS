#pragma once

#include "container/entity_storage.hpp"
#include "container/component_storage.hpp"

namespace mytho::ecs {
    template<mytho::utils::EntityType EntityT, mytho::utils::UnsignedIntegralType ComponentIdT = size_t, size_t PageSize = 1024>
    class basic_registry final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_type, PageSize>;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_type, PageSize>;

    public:
        template<mytho::utils::PureValueType... Ts>
        entity_type spawn(Ts&&... ts) noexcept {
            auto e = _entities.emplace();

            if constexpr (sizeof...(Ts) > 0) {
                _components.add(e, std::forward<Ts>(ts)...);
            }

            return e;
        }

        void despawn(const entity_type& e) noexcept {
            _components.remove(e);
            _entities.pop(e);
        }

    public:
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) noexcept {
            _entities.template add<Ts...>(e);
            _components.add(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            _entities.template remove<Ts...>(e);
            _components.template remove<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        auto get(const entity_type& e) const noexcept {
            return _components.template get<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            _components.replace(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _entities.contain(e) && _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;
    };
}