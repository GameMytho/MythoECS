#pragma once

#include <optional>

#include "container/entity_storage.hpp"
#include "container/component_storage.hpp"
#include "ecs/system.hpp"
#include "ecs/resources.hpp"

namespace mytho::ecs {
    template<
        mytho::utils::EntityType EntityT,
        mytho::utils::UnsignedIntegralType ComponentIdT = size_t,
        mytho::utils::UnsignedIntegralType SystemIndexT = size_t,
        size_t PageSize = 1024
    >
    class basic_registry final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using system_index_type = SystemIndexT;
        using self_type = mytho::ecs::basic_registry<entity_type, component_id_type, system_index_type, PageSize>;
        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_type, PageSize>;
        using size_type = typename entity_storage_type::size_type;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_type, PageSize>;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;
        using system_storage_type = internal::basic_system_storage<self_type, system_index_type>;

        template<typename... Ts>
        using querier_type = mytho::ecs::basic_querier<self_type, Ts...>;

        template<typename T>
        using resource_cache = mytho::ecs::internal::basic_resource_cache<T>;

        template<typename... Ts>
        using resources_type = mytho::ecs::basic_resources<Ts...>;

        template<typename... Ts>
        using resources_mut_type = mytho::ecs::basic_resources_mut<Ts...>;

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
            using querier = querier_type<Ts...>;
            using component_bundle_container_type = typename querier::component_bundle_container_type;
            using component_datatype_list = typename querier::component_datatype_list;
            using component_contain_list = typename querier::component_contain_list;
            using component_not_contain_list = typename querier::component_not_contain_list;

            component_bundle_container_type component_bundles;

            auto id = get_cid_with_minimun_entities(component_contain_list{});

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(contain(it, component_contain_list{}) && not_contain(it, component_not_contain_list{})) {
                        component_bundles.emplace_back(_query(it, component_datatype_list{}));
                    }
                }
            }

            return component_bundles;
        }

    public:
        template<typename T, typename... Rs>
        self_type& init_resource(Rs&&... rs) noexcept {
            resource_cache<T>::init(std::forward<Rs>(rs)...);

            return *this;
        }

        template<typename T>
        self_type& remove_resource() noexcept {
            resource_cache<T>::destroy();

            return *this;
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources() const noexcept {
            return std::tuple_cat(std::tie(_resource<Ts>())...);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources_mut() const noexcept {
            return std::tuple_cat(std::tie(_resource_mut<Ts>())...);
        }

    public:
        template<auto Func, mytho::utils::LocationType... Locs>
        self_type& add_startup_system() noexcept {
            _startup_systems.template add<Func, Locs...>();

            return *this;
        }

        template<auto Func>
        self_type& remove_startup_system() noexcept {
            _startup_systems.template remove<Func>();

            return *this;
        }

        template<auto Func>
        self_type& enable_startup_system() noexcept {
            _startup_systems.template enable<Func>();

            return *this;
        }

        template<auto Func>
        self_type& disable_startup_system() noexcept {
            _startup_systems.template disable<Func>();

            return *this;
        }

        template<auto Func, mytho::utils::LocationType... Locs>
        self_type& add_update_system() noexcept {
            _update_systems.template add<Func, Locs...>();

            return *this;
        }

        template<auto Func>
        self_type& remove_update_system() noexcept {
            _update_systems.template remove<Func>();

            return *this;
        }

        template<auto Func>
        self_type& enable_update_system() noexcept {
            _update_systems.template enable<Func>();

            return *this;
        }

        template<auto Func>
        self_type& disable_update_system() noexcept {
            _update_systems.template disable<Func>();

            return *this;
        }

        template<auto Func, mytho::utils::LocationType... Locs>
        self_type& add_shutdown_system() noexcept {
            _shutdown_systems.template add<Func, Locs...>();

            return *this;
        }

        template<auto Func>
        self_type& remove_shutdown_system() noexcept {
            _shutdown_systems.template remove<Func>();

            return *this;
        }

        template<auto Func>
        self_type& enable_shutdown_system() noexcept {
            _shutdown_systems.template enable<Func>();

            return *this;
        }

        template<auto Func>
        self_type& disable_shutdown_system() noexcept {
            _shutdown_systems.template disable<Func>();

            return *this;
        }

    public:
        void ready() noexcept {
            _startup_systems.sort();
            _update_systems.sort();
            _shutdown_systems.sort();
        }

        void startup() noexcept {
            for (auto& sys : _startup_systems) {
                if (sys.enabled()) {
                    sys(*this);
                }
            }
        }

        void update() noexcept {
            for (auto& sys : _update_systems) {
                if (sys.enabled()) {
                    sys(*this);
                }
            }
        }

        void shutdown() noexcept {
            for (auto& sys : _shutdown_systems) {
                if (sys.enabled()) {
                    sys(*this);
                }
            }
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;
        system_storage_type _startup_systems;
        system_storage_type _update_systems;
        system_storage_type _shutdown_systems;

    private:
        template<typename T, typename... Rs>
        std::optional<component_id_type> get_cid_with_minimun_entities(internal::type_list<T, Rs...>) {
            auto id = component_id_generator::template gen<T>();
            if (id >= _components.size() || !_components[id]) {
                return std::nullopt;
            }

            if constexpr (sizeof...(Rs) > 0) {
                auto id_rs = get_cid_with_minimun_entities(internal::type_list<Rs...>{});
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
        bool contain(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            return _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        bool not_contain(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _entities.template not_has<Ts...>(e) && _components.template not_contain<Ts...>(e);
            } else {
                return true;
            }
        }

        template<typename T, typename... Rs>
        auto _query(const entity_type& e, internal::type_list<T, Rs...>) noexcept {
            using prototype = std::decay_t<T>;
            using component_set_type = mytho::container::basic_component_set<entity_type, prototype, std::allocator<prototype>, PageSize>;

            auto id = component_id_generator::template gen<prototype>();

            if constexpr (sizeof...(Rs) > 0) {
                return std::tuple_cat(std::tie(static_cast<component_set_type&>(*_components[id]).get(e)), _query(e, internal::type_list<Rs...>{}));
            } else {
                return std::tie(static_cast<component_set_type&>(*_components[id]).get(e));
            }
        }

        template<typename T>
        const T& _resource() const noexcept {
            ASSURE(internal::basic_resource_cache<T>::instance(), "Resource not exist (may be destroyed or uninitialized).");

            return internal::basic_resource_cache<T>::instance().value();
        }

        template<typename T>
        T& _resource_mut() const noexcept {
            ASSURE(internal::basic_resource_cache<T>::instance(), "Resource not exist (may be destroyed or uninitialized).");

            return internal::basic_resource_cache<T>::instance().value();
        }
    };
}