#pragma once
#include <optional>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstddef>

#include "container/entity_storage.hpp"
#include "container/component_storage.hpp"
#include "ecs/system.hpp"

namespace mytho::ecs {
    template<
        mytho::utils::EntityType EntityT,
        mytho::utils::UnsignedIntegralType ComponentIdT = size_t,
        size_t PageSize = 1024
    >
    class basic_registry final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using self_type = mytho::ecs::basic_registry<entity_type, component_id_type, PageSize>;
        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_type, PageSize>;
        using size_type = typename entity_storage_type::size_type;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_type, PageSize>;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;
        using command_queue_type = mytho::ecs::internal::basic_command_queue<self_type>;
        using system_storage_type = internal::basic_system_storage<self_type>;
        using system_config_type = typename system_storage_type::system_config_type;

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
                _components.add(e, _current_tick, std::forward<Ts>(ts)...);
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
            _components.add(e, _current_tick, std::forward<Ts>(ts)...);
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
            _components.replace(e, _current_tick, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _entities.contain(e) && _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        querier_type<Ts...> query() noexcept {
            return query<Ts...>(_current_tick);
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        querier_type<Ts...> query(uint64_t tick) noexcept {
            using querier = querier_type<Ts...>;
            using component_bundle_container_type = typename querier::component_bundle_container_type;
            using component_prototype_list = typename querier::component_prototype_list;
            using component_contain_list = typename querier::component_contain_list;
            using component_not_contain_list = typename querier::component_not_contain_list;
            using component_added_list = typename querier::component_added_list;
            using component_changed_list = typename querier::component_changed_list;

            component_bundle_container_type component_bundles;

            auto id = get_cid_with_minimun_entities(component_contain_list{});

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(contain(it, component_contain_list{}) 
                        && not_contain(it, component_not_contain_list{}) 
                        && is_added(it, tick, component_added_list{})
                        && is_changed(it, tick, component_changed_list{})
                    ) {
                        component_bundles.emplace_back(_query(it, component_prototype_list{}));
                    }
                }
            }

            return { component_bundles, _current_tick };
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        size_type count(uint64_t tick) noexcept {
            using query_types = internal::query_types<Ts...>;
            using component_contain_list = internal::type_list_cat_t<
                                                internal::type_list_filter_t<typename query_types::require_prototype_list, entity_type>,
                                                typename query_types::with_prototype_list,
                                                typename query_types::added_prototype_list,
                                                typename query_types::changed_prototype_list
                                            >;
            using component_not_contain_list = typename query_types::without_prototype_list;
            using component_added_list = typename query_types::added_prototype_list;
            using component_changed_list = typename query_types::changed_prototype_list;

            size_type count = 0;
            auto id = get_cid_with_minimun_entities(component_contain_list{});

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(contain(it, component_contain_list{})
                        && not_contain(it, component_not_contain_list{})
                        && is_added(it, tick, component_added_list{})
                        && is_changed(it, tick, component_changed_list{})
                    ) {
                        ++count;
                    }
                }
            }

            return count;
        }

    public:
        template<mytho::utils::PureResourceType T, typename... Rs>
        self_type& init_resource(Rs&&... rs) noexcept {
            resource_cache<T>::init(std::forward<Rs>(rs)...);

            return *this;
        }

        template<mytho::utils::PureResourceType T>
        self_type& remove_resource() noexcept {
            resource_cache<T>::destroy();

            return *this;
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources() const noexcept {
            return std::tuple_cat(std::tie(_resource<Ts>())...);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources_mut() const noexcept {
            return std::tuple_cat(std::tie(_resource_mut<Ts>())...);
        }

        template<typename T>
        bool resource_exist() const noexcept {
            return internal::basic_resource_cache<T>::instance() != std::nullopt;
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_exist() const noexcept {
            return (resource_exist<Ts>() && ...);
        }

    public:
        template<mytho::utils::FunctionType Func>
        static system_config_type system(Func&& func) noexcept {
            return system_config_type(std::forward<Func>(func));
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_startup_system(Func&& func) noexcept {
            _startup_systems.add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_startup_system(const system_config_type& config) noexcept {
            _startup_systems.add(config);
            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_update_system(Func&& func) noexcept {
            _update_systems.add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_update_system(const system_config_type& config) noexcept {
            _update_systems.add(config);
            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_shutdown_system(Func&& func) noexcept {
            _shutdown_systems.add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_shutdown_system(const system_config_type& config) noexcept {
            _shutdown_systems.add(config);
            return *this;
        }

    public:
        void ready() noexcept {
            _startup_systems.ready();
            _update_systems.ready();
            _shutdown_systems.ready();
        }

        void startup() noexcept {
            for (auto& sys : _startup_systems) {
                if (sys.should_run(*this)) {
                    sys(*this, ++_current_tick);
                }
            }

            _current_tick++;
            apply_commands();
        }

        void update() noexcept {
            for (auto& sys : _update_systems) {
                if (sys.should_run(*this)) {
                    sys(*this, ++_current_tick);
                }
            }

            _current_tick++;
            apply_commands();
        }

        void shutdown() noexcept {
            for (auto& sys : _shutdown_systems) {
                if (sys.should_run(*this)) {
                    sys(*this, ++_current_tick);
                }
            }

            _current_tick++;
            apply_commands();
        }

    public:
        command_queue_type& command_queue() noexcept {
            return _command_queue;
        }

        void apply_commands() noexcept {
            _command_queue.apply(*this);
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;
        command_queue_type _command_queue;

        uint64_t _current_tick = 0;
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

        template<mytho::utils::PureComponentType... Ts>
        bool is_added(const entity_type& e, uint64_t tick, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _components.template is_added<Ts...>(e, tick);
            } else {
                return true;
            }
        }

        template<mytho::utils::PureComponentType... Ts>
        bool is_changed(const entity_type& e, uint64_t tick, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _components.template is_changed<Ts...>(e, tick);
            } else {
                return true;
            }
        }

        template<typename T, typename... Rs>
        auto _query(const entity_type& e, internal::type_list<T, Rs...>) noexcept {
            using prototype = std::decay_t<T>;
            using component_set_type = mytho::container::basic_component_set<entity_type, prototype, std::allocator<prototype>, PageSize>;

            if constexpr (std::is_same_v<prototype, entity_type>) {
                if constexpr (sizeof...(Rs) > 0) {
                    return std::tuple_cat(
                        std::tuple(mytho::utils::internal::data_wrapper<entity_type>(e)),
                        _query(e, internal::type_list<Rs...>{})
                    );
                } else {
                    return std::tuple(mytho::utils::internal::data_wrapper<entity_type>(e));
                }
            } else {
                auto id = component_id_generator::template gen<prototype>();

                if constexpr (sizeof...(Rs) > 0) {
                    return std::tuple_cat(
                        std::tuple(
                            mytho::utils::internal::data_wrapper<T>(
                                &(static_cast<component_set_type&>(*_components[id]).get(e)),
                                static_cast<component_set_type&>(*_components[id]).changed_tick(e),
                                _current_tick
                            )
                        ),
                        _query(e, internal::type_list<Rs...>{})
                    );
                }
                else {
                    return std::tuple(
                        mytho::utils::internal::data_wrapper<T>(
                            &(static_cast<component_set_type&>(*_components[id]).get(e)),
                            static_cast<component_set_type&>(*_components[id]).changed_tick(e),
                            _current_tick
                        )
                    );
                }
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