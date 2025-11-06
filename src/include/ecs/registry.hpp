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
#include "container/resource_storage.hpp"
#include "ecs/schedule.hpp"

namespace mytho::ecs {
    enum class startup_stage {
        Startup
    };

    enum class core_stage {
        First,
        PreUpdate,
        Update,
        PostUpdate,
        Last
    };

    template<
        mytho::utils::EntityType EntityT,
        mytho::utils::UnsignedIntegralType ComponentIdT = size_t,
        mytho::utils::UnsignedIntegralType ResourceIdT = size_t,
        mytho::utils::UnsignedIntegralType EventIdT = size_t,
        size_t PageSize = 1024
    >
    class basic_registry final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using resource_id_type = ResourceIdT;
        using event_id_type = EventIdT;
        using self_type = mytho::ecs::basic_registry<entity_type, component_id_type, resource_id_type, event_id_type, PageSize>;
        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_type, PageSize>;
        using size_type = typename entity_storage_type::size_type;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_type, PageSize>;
        using component_id_generator = mytho::utils::basic_id_generator<mytho::utils::GeneratorType::COMPONENT_GENOR, component_id_type>;
        using resource_storage_type = mytho::container::basic_resource_storage<resource_id_type, std::allocator>;
        using events_type = mytho::ecs::basic_events<event_id_type, std::allocator>;
        using command_queue_type = mytho::ecs::internal::basic_command_queue<self_type>;
        using schedule_type = mytho::ecs::internal::basic_schedule<self_type, uint16_t>;
        using system_type = typename schedule_type::system_type;

        template<typename... Ts>
        using querier_type = mytho::ecs::basic_querier<self_type, Ts...>;

        template<typename T>
        using resource_cache = mytho::ecs::internal::basic_resource_cache<T>;

        template<typename... Ts>
        using resources_type = mytho::ecs::basic_resources<Ts...>;

        template<typename... Ts>
        using resources_mut_type = mytho::ecs::basic_resources_mut<Ts...>;

        basic_registry() {
            _startup_schedule.template add_stage<startup_stage::Startup>();

            _update_schedule.template add_stage<core_stage::First>()
                            .template add_stage<core_stage::PreUpdate>()
                            .template add_stage<core_stage::Update>()
                            .template add_stage<core_stage::PostUpdate>()
                            .template add_stage<core_stage::Last>();
        }

    public: // entity operations
        template<mytho::utils::PureComponentType... Ts>
        entity_type spawn(Ts&&... ts) {
            auto e = _entities.emplace();

            if constexpr (sizeof...(Ts) > 0) {
                _entities.template add<Ts...>(e);
                _components.add(e, _current_tick, std::forward<Ts>(ts)...);
            }

            return e;
        }

        void despawn(const entity_type& e) {
            _components.remove(e);
            _entities.pop(e);
        }

        bool alive(const entity_type& e) const noexcept {
            return _entities.contain(e);
        }

        const auto& entities() const noexcept {
            return _entities;
        }

    public: // component operations
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) {
            if (contain<Ts...>(e)) {
                return;
            }

            _entities.template add<Ts...>(e);
            _components.add(e, _current_tick, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) {
            if (!contain<Ts...>(e)) {
                return;
            }

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
        void replace(const entity_type& e, Ts&&... ts) {
            if (!contain<Ts...>(e)) {
                return;
            }

            _components.replace(e, _current_tick, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _entities.contain(e) && _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_added(uint64_t tick) noexcept {
            auto id = _get_cid_with_minimun_entities<Ts...>();

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(_components.template is_added<Ts...>(it, tick)) {
                        return true;
                    }
                }
            }

            return false;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_changed(uint64_t tick) noexcept {
            auto id = _get_cid_with_minimun_entities<Ts...>();

            if (id) {
                for (auto it : *_components[id.value()]) {
                    if(_components.template is_changed<Ts...>(it, tick)) {
                        return true;
                    }
                }
            }

            return false;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_removed() noexcept {
            return (!_components.template removed_entities<Ts>().empty() && ...);
        }

        template<mytho::utils::PureComponentType T>
        auto& removed_entities() noexcept {
            return _components.template removed_entities<T>();
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        querier_type<Ts...> query() {
            return query<Ts...>(_current_tick);
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        querier_type<Ts...> query(uint64_t tick) {
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
                    if(component_list_contained(it, component_contain_list{})
                        && component_list_not_contained(it, component_not_contain_list{}) 
                        && component_list_added(it, tick, component_added_list{})
                        && component_list_changed(it, tick, component_changed_list{})
                    ) {
                        component_bundles.emplace_back(_query(it, component_prototype_list{}));
                    }
                }
            }

            return { std::move(component_bundles) };
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
                    if(component_list_contained(it, component_contain_list{})
                        && component_list_not_contained(it, component_not_contain_list{})
                        && component_list_added(it, tick, component_added_list{})
                        && component_list_changed(it, tick, component_changed_list{})
                    ) {
                        ++count;
                    }
                }
            }

            return count;
        }

    public: // resource operations
        template<mytho::utils::PureResourceType T, typename... Rs>
        self_type& init_resource(Rs&&... rs) {
            _resources.template init<T>(_current_tick, std::forward<Rs>(rs)...);

            return *this;
        }

        template<mytho::utils::PureResourceType T>
        self_type& remove_resource() {
            _resources.template deinit<T>();

            return *this;
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources() noexcept {
            return std::tuple<const mytho::utils::internal::data_wrapper<Ts>...>{
                mytho::utils::internal::data_wrapper<Ts>{
                    &_resources.template get<Ts>(),
                    _resources.template get_changed_tick_ref<Ts>(),
                    _current_tick
                }...
            };
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources_mut() noexcept {
            return std::tuple<mytho::utils::internal::data_wrapper<Ts>...>{
                mytho::utils::internal::data_wrapper<Ts>{
                    &_resources.template get<Ts>(),
                    _resources.template get_changed_tick_ref<Ts>(),
                    _current_tick
                }...
            };
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_exist() const noexcept {
            return (_resources.template contain<Ts>() && ...);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_added(uint64_t tick) const noexcept {
            return (_resources.template is_added<Ts>(tick) && ...);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_changed(uint64_t tick) const noexcept {
            return (_resources.template is_changed<Ts>(tick) && ...);
        }

    public: // event operations
        template<mytho::utils::PureEventType T, typename... Rs>
        void event_write(Rs&&... rs) {
            _events.template write<T>(std::forward<Rs>(rs)...);
        }

        template<mytho::utils::PureEventType T>
        auto event_mutate() noexcept {
            return _events.template mutate<T>();
        }

        template<mytho::utils::PureEventType T>
        auto event_read() const noexcept {
            return _events.template read<T>();
        }

    public: // schedule operations
        template<auto StageE>
        self_type& add_startup_stage() {
            _startup_schedule.template add_stage<StageE>();

            return *this;
        }

        template<auto StageE, auto BeforeStageE>
        self_type& add_startup_stage_before() {
            _startup_schedule.template add_stage_before<StageE, BeforeStageE>();

            return *this;
        }

        template<auto StageE, auto AfterStageE>
        self_type& add_startup_stage_after() {
            _startup_schedule.template add_stage_after<StageE, AfterStageE>();

            return *this;
        }

        template<auto StageE, auto InsertStageE>
        self_type& insert_startup_stage() {
            _startup_schedule.template insert_stage<StageE, InsertStageE>();

            return *this;
        }

        template<auto StageE>
        self_type& add_update_stage() {
            _update_schedule.template add_stage<StageE>();

            return *this;
        }

        template<auto StageE, auto BeforeStageE>
        self_type& add_update_stage_before() {
            _update_schedule.template add_stage_before<StageE, BeforeStageE>();

            return *this;
        }

        template<auto StageE, auto AfterStageE>
        self_type& add_update_stage_after() {
            _update_schedule.template add_stage_after<StageE, AfterStageE>();

            return *this;
        }

        template<auto StageE, auto InsertStageE>
        self_type& insert_update_stage() {
            _update_schedule.template insert_stage<StageE, InsertStageE>();

            return *this;
        }

    public: // system operations
        template<mytho::utils::FunctionType Func>
        static system_type system(Func&& func) noexcept {
            return system_type(std::forward<Func>(func));
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_startup_system(Func&& func) {
            _startup_schedule.template add_system<startup_stage::Startup>(std::forward<Func>(func));

            return *this;
        }

        template<auto StageE, mytho::utils::FunctionType Func>
        self_type& add_startup_system(Func&& func) {
            _startup_schedule.template add_system<StageE>(std::forward<Func>(func));

            return *this;
        }

        self_type& add_startup_system(system_type& system) {
            _startup_schedule.template add_system<startup_stage::Startup>(system);

            return *this;
        }

        template<auto StageE>
        self_type& add_startup_system(system_type& system) {
            _startup_schedule.template add_system<StageE>(system);

            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_update_system(Func&& func) {
            _update_schedule.template add_system<core_stage::Update>(std::forward<Func>(func));

            return *this;
        }

        template<auto StageE, mytho::utils::FunctionType Func>
        self_type& add_update_system(Func&& func) {
            _update_schedule.template add_system<StageE>(std::forward<Func>(func));
            return *this;
        }

        self_type& add_update_system(system_type& system) {
            _update_schedule.template add_system<core_stage::Update>(system);

            return *this;
        }

        template<auto StageE>
        self_type& add_update_system(system_type& system) {
            _update_schedule.template add_system<StageE>(system);

            return *this;
        }

    public: // core operations
        void startup() {
            _startup_schedule.run(*this, _current_tick);

            _current_tick++;
            apply_commands();
        }

        void update() {
            _update_schedule.run(*this, _current_tick);

            _current_tick++;
            apply_commands();

            _components.removed_entities_clear();
            _events.swap();
        }

    public: // command operations
        command_queue_type& command_queue() noexcept {
            return _command_queue;
        }

        void apply_commands() {
            _command_queue.apply(*this);
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;
        resource_storage_type _resources;
        events_type _events;

        command_queue_type _command_queue;

        // tick start from 1, and 0 is reserved for init
        uint64_t _current_tick = 1;
        schedule_type _startup_schedule;
        schedule_type _update_schedule;

    private:
        template<typename T, typename... Rs>
        auto get_cid_with_minimun_entities(internal::type_list<T, Rs...>) noexcept {
            return _get_cid_with_minimun_entities<T, Rs...>();
        }

        template<typename T, typename... Rs>
        auto _get_cid_with_minimun_entities() noexcept {
            auto best = component_id_generator::template gen<T>();

            bool ok = (best < _components.size() && _components[best]) && (
                true && ... && [&]{
                    auto cur = component_id_generator::template gen<Rs>();
                    if (cur >= _components.size() || !_components[cur]) return false;
                    if (_components[cur]->size() < _components[best]->size()) best = cur;
                    return true;
                }()
            );

            return ok ? std::optional<component_id_type>(best) : std::nullopt;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool component_list_contained(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            return _entities.template has<Ts...>(e) && _components.template contain<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        bool component_list_not_contained(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _entities.template not_has<Ts...>(e) && _components.template not_contain<Ts...>(e);
            } else {
                return true;
            }
        }

        template<mytho::utils::PureComponentType... Ts>
        bool component_list_added(const entity_type& e, uint64_t tick, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _components.template is_added<Ts...>(e, tick);
            } else {
                return true;
            }
        }

        template<mytho::utils::PureComponentType... Ts>
        bool component_list_changed(const entity_type& e, uint64_t tick, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                return _components.template is_changed<Ts...>(e, tick);
            } else {
                return true;
            }
        }

        template<typename... Ts>
        auto _query(const entity_type& e, internal::type_list<Ts...>) noexcept {
            return std::tuple_cat(_query<Ts>(e)...);
        }

        template<typename T>
        auto _query(const entity_type& e) noexcept {
            using prototype = std::decay_t<T>;
            using component_set_type = mytho::container::basic_component_set<entity_type, prototype, std::allocator<prototype>, PageSize>;

            if constexpr (std::is_same_v<prototype, entity_type>) {
                return std::tuple(mytho::utils::internal::data_wrapper<entity_type>(e));
            } else {
                auto id = component_id_generator::template gen<prototype>();
                return std::tuple(
                    mytho::utils::internal::data_wrapper<T>(
                        &(static_cast<component_set_type&>(*_components[id]).get(e)),
                        static_cast<component_set_type&>(*_components[id]).changed_tick(e),
                        _current_tick
                    )
                );
            }
        }
    };
}