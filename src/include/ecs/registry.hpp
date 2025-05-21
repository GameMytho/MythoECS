#pragma once

#include <optional>

#include "container/entity_storage.hpp"
#include "container/component_storage.hpp"
#include "ecs/querier.hpp"

namespace mytho::ecs {
    template<mytho::utils::EntityType EntityT, mytho::utils::UnsignedIntegralType ComponentIdT = size_t, size_t PageSize = 1024>
    class basic_registry final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using self_type = mytho::ecs::basic_registry<entity_type, component_id_type, PageSize>;
        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_type, PageSize>;
        using size_type = typename entity_storage_type::size_type;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_type, PageSize>;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;

        template<typename... Ts>
        using querier_type = mytho::ecs::basic_querier<self_type, Ts...>;

    public:
        template<mytho::utils::PureComponentType... Ts>
        entity_type spawn(Ts&&... ts) noexcept {
            auto e = _entities.emplace();

            if constexpr (sizeof...(Ts) > 0) {
                _entities.template add<Ts...>(e);
                _components.add(e, std::forward<Ts>(ts)...);
            }

            return e;
        }

        void despawn(const entity_type& e) noexcept {
            _components.remove(e);
            _entities.pop(e);
        }

    public:
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) noexcept {
            _entities.template add<Ts...>(e);
            _components.add(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            _entities.template remove<Ts...>(e);
            _components.template remove<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        std::tuple<const Ts&...> get(const entity_type& e) const noexcept {
            return _components.template get<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            _components.replace(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _entities.contain(e) && _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        querier_type<Ts...> query() noexcept {
            using component_bundle_container_type = typename querier_type<Ts...>::component_bundle_container_type;

            component_bundle_container_type component_bundles;

            auto id = get_cid_with_minimun_entities(mytho::utils::convert_to_prototype_list<Ts...>{});

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(contain<mytho::utils::convert_to_prototype_list<Ts...>>(it)) {
                        component_bundles.emplace_back(_query(it, mytho::utils::convert_to_datatype_list<Ts...>{}));
                    }
                }
            }

            return component_bundles;
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;

    private:
        template<typename T, typename... Rs>
        std::optional<component_id_type> get_cid_with_minimun_entities(mytho::utils::type_list<T, Rs...>) {
            auto id = component_id_generator::template gen<T>();
            if (id >= _components.size() || !_components[id]) {
                return std::nullopt;
            }

            if constexpr (sizeof...(Rs) > 0) {
                auto id_rs = get_cid_with_minimun_entities(mytho::utils::type_list<Rs...>{});
                if (!id_rs) {
                    return std::nullopt;
                } else {
                    return _components[id]->size() < _components[id_rs.value()]->size() ? id : id_rs;
                }
            }

            return id;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e, mytho::utils::type_list<Ts...>) const noexcept {
            return _entities.contain(e) && _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<typename T, typename... Rs>
        auto _query(const entity_type& e, mytho::utils::type_list<T, Rs...>) noexcept {
            using prototype = std::decay_t<T>;
            using component_set_type = mytho::container::basic_component_set<entity_type, prototype, std::allocator<prototype>, PageSize>;

            auto id = component_id_generator::template gen<prototype>();

            if constexpr (sizeof...(Rs) > 0) {
                return std::tuple_cat(std::tie(static_cast<component_set_type&>(*_components[id]).get(e)), _query(e, mytho::utils::type_list<Rs...>{}));
            } else {
                return std::tie(static_cast<component_set_type&>(*_components[id]).get(e));
            }
        }
    };
}