#pragma once
#include <optional>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include <cstddef>

#include "utils/idgen.hpp"
#include "container/entity_storage.hpp"
#include "container/component_storage.hpp"
#include "container/resource_storage.hpp"
#include "ecs/schedule.hpp"

namespace mytho::ecs {
    namespace internal {
        // id generators
        struct component_genor final {};
        struct resource_genor final {};
        struct event_genor final {};
        struct schedule_genor final {};
    }

    enum class startup_schedules {
        PreStartup,
        Startup,
        PostStartup
    };

    enum class main_schedules {
        First,
        PreUpdate,
        Update,
        PostUpdate,
        Last
    };

    template<
        mytho::utils::EntityType EntityT,
        mytho::utils::UnsignedIntegralType ComponentIdT = uint16_t,
        mytho::utils::UnsignedIntegralType ResourceIdT = uint16_t,
        mytho::utils::UnsignedIntegralType ScheduleIdT = uint8_t,
        size_t PageSize = 256
    >
    class basic_registry final {
    private:
        enum class internal_schedules {
            Startup,
            Main
        };

    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using resource_id_type = ResourceIdT;
        using schedule_id_type = ScheduleIdT;
        using self_type = mytho::ecs::basic_registry<entity_type, component_id_type, resource_id_type, schedule_id_type, PageSize>;

        using component_id_generator = mytho::utils::basic_id_generator<internal::component_genor, component_id_type>;
        using resource_id_generator = mytho::utils::basic_id_generator<internal::resource_genor, component_id_type>;
        using schedule_id_generator = mytho::utils::basic_id_generator<internal::schedule_genor, component_id_type>;

        using entity_storage_type = mytho::container::basic_entity_storage<entity_type, component_id_generator, PageSize>;
        using component_storage_type = mytho::container::basic_component_storage<entity_type, component_id_generator, std::allocator, PageSize>;
        using resource_storage_type = mytho::container::basic_resource_storage<resource_id_generator, std::allocator>;
        using command_queue_type = mytho::ecs::internal::basic_command_queue<self_type>;
        using schedules_type = mytho::ecs::internal::basic_schedules<self_type>;

        using size_type = typename entity_storage_type::size_type;
        using system_type = typename schedules_type::system_type;
        using entity_set_type = typename entity_storage_type::base_type;

        // system argument types
        using commands_type = mytho::ecs::basic_commands<self_type>;

        template<typename... Ts>
        using querier_type = mytho::ecs::basic_querier<self_type, Ts...>;

        template<typename... Ts>
        using resources_type = mytho::ecs::basic_resources<Ts...>;

        template<typename... Ts>
        using resources_mut_type = mytho::ecs::basic_resources_mut<Ts...>;

        template<typename T>
        using events_type = basic_events<T>;

        basic_registry() {
            _schedules.template add_startup_schedule<startup_schedules::PreStartup>()
                      .template add_startup_schedule<startup_schedules::Startup>()
                      .template add_startup_schedule<startup_schedules::PostStartup>()
                      .template add_startup_schedule<internal_schedules::Startup>()
                      .template add_update_schedule<main_schedules::First>()
                      .template add_update_schedule<main_schedules::PreUpdate>()
                      .template add_update_schedule<main_schedules::Update>()
                      .template add_update_schedule<main_schedules::PostUpdate>()
                      .template add_update_schedule<main_schedules::Last>()
                      .template add_update_schedule<internal_schedules::Main>()
                      .template set_default_schedule<main_schedules::Update>();

            _schedules.template add_system<internal_schedules::Startup>(+[](commands_type cmds){
                cmds.apply();
            });

            _schedules.template add_system<internal_schedules::Main>(+[](commands_type cmds){
                cmds.registry().removed_entities_clear();
                cmds.apply();
            });
        }

    public: // entity operations
        template<mytho::utils::PureComponentType... Ts>
        entity_type spawn(Ts&&... ts) {
            auto e = _entities.emplace();

            if constexpr (sizeof...(Ts) > 0) {
                if (e.valid()) {
                    _entities.template add<Ts...>(e);
                    _components.add(e, _current_tick, std::forward<Ts>(ts)...);
                }
            }

            return e;
        }

        void despawn(const entity_type& e) {
            ASSURE(alive(e), "entity not alive");

            _components.remove(e);
            _entities.pop(e);
        }

        bool alive(const entity_type& e) const noexcept {
            return e.valid() && _entities.contain(e);
        }

        const auto& entities() const noexcept {
            return _entities;
        }

    public: // component operations
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) {
            if (!alive(e)) {
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
            ASSURE(contain<Ts...>(e), "entity does not have some components");

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
            // _entities and _components are synchronized, just check one
            return alive(e) && _entities.template has<Ts...>(e) /* && _components.template contain<Ts...>(e) */;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_added(uint64_t tick) noexcept {
            auto id = _get_cid_with_minimun_entities<Ts...>();

            if (id) {
                const auto& entts = *_components[id.value()];
                for (auto i = 0; i < entts.size(); ++i) {
                    if(_components.template is_added<Ts...>(entts[i], tick)) {
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
                const auto& entts = *_components[id.value()];
                for (auto i = 0; i < entts.size(); ++i) {
                    if(_components.template is_changed<Ts...>(entts[i], tick)) {
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
            using entts_type = std::conditional_t<component_contain_list::size == 0, entity_storage_type, entity_set_type>;

            component_bundle_container_type component_bundles;

            entts_type* entts_ptr = nullptr;
            if constexpr (component_contain_list::size == 0) {
                entts_ptr = &_entities;
            } else {
                auto id = get_cid_with_minimun_entities(component_contain_list{});
                if (!id) {
                    return { std::move(component_bundles) };
                }

                entts_ptr = _components[id.value()];
            }

            const auto& entts = *entts_ptr;
            auto size = entts.size();
            component_bundles.reserve(size);
            for (auto i = 0; i < size; ++i) {
                const auto e = entts[i];

                if constexpr (component_contain_list::size != 0) {
                    if(!component_list_contained(e, component_contain_list{})) {
                        continue;
                    }
                }

                if(component_list_not_contained(e, component_not_contain_list{})
                    && component_list_added(e, tick, component_added_list{})
                    && component_list_changed(e, tick, component_changed_list{})
                ) {
                    component_bundles.emplace_back(_query(e, component_prototype_list{}));
                }
            }

            return { std::move(component_bundles) };
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        size_type count() noexcept {
            return count<Ts...>(_current_tick);
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        size_type count(uint64_t tick) noexcept {
            using querier = querier_type<Ts...>;
            using component_contain_list = typename querier::component_contain_list;
            using component_not_contain_list = typename querier::component_not_contain_list;
            using component_added_list = typename querier::component_added_list;
            using component_changed_list = typename querier::component_changed_list;
            using entts_type = std::conditional_t<component_contain_list::size == 0, entity_storage_type, entity_set_type>;

            size_type count = 0;

            entts_type* entts_ptr = nullptr;
            if constexpr (component_contain_list::size == 0) {
                entts_ptr = &_entities;
            } else {
                auto id = get_cid_with_minimun_entities(component_contain_list{});
                if (!id) {
                    return count;
                }

                entts_ptr = _components[id.value()];
            }

            const auto& entts = *entts_ptr;
            auto size = entts.size();
            for (auto i = 0; i < size; ++i) {
                const auto e = entts[i];

                if constexpr (component_contain_list::size != 0) {
                    if(!component_list_contained(e, component_contain_list{})) {
                        continue;
                    }
                }

                if(component_list_not_contained(e, component_not_contain_list{})
                    && component_list_added(e, tick, component_added_list{})
                    && component_list_changed(e, tick, component_changed_list{})
                ) {
                    ++count;
                }
            }

            return count;
        }

    public: // resource operations
        template<mytho::utils::PureResourceType T, typename... Rs>
        self_type& init_resource(Rs&&... rs) {
            ASSURE(!_resources.template exist<T>() , "resource already exists");

            _resources.template init<T>(_current_tick, std::forward<Rs>(rs)...);

            return *this;
        }

        template<mytho::utils::PureResourceType T>
        self_type& remove_resource() noexcept {
            ASSURE(_resources.template exist<T>() , "resource not exists");

            _resources.template deinit<T>();

            return *this;
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources() noexcept {
            ASSURE(_resources.template exist<Ts>() && ..., "some resources not exist");

            return std::tuple<const mytho::utils::internal::data_wrapper<Ts>...>(
                mytho::utils::internal::data_wrapper<Ts>(
                    &_resources.template get<Ts>(),
                    _resources.template get_changed_tick_ref<Ts>(),
                    _current_tick
                )...
            );
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        auto resources_mut() noexcept {
            ASSURE(_resources.template exist<Ts>() && ..., "some resources not exist");

            return std::tuple<mytho::utils::internal::data_wrapper<Ts>...>(
                mytho::utils::internal::data_wrapper<Ts>(
                    &_resources.template get<Ts>(),
                    _resources.template get_changed_tick_ref<Ts>(),
                    _current_tick
                )...
            );
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_added(uint64_t tick) const noexcept {
            ASSURE(_resources.template exist<Ts>() && ..., "some resources not exist");

            return (_resources.template is_added<Ts>(tick) && ...);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_changed(uint64_t tick) const noexcept {
            ASSURE(_resources.template exist<Ts>() && ..., "some resources not exist");

            return (_resources.template is_changed<Ts>(tick) && ...);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_exist() const noexcept {
            return (_resources.template exist<Ts>() && ...);
        }

    public: // event operations
        template<mytho::utils::PureEventType T>
        self_type& init_event() {
            ASSURE(!_resources.template exist<events_type<T>>(), "event type already exists");

            _resources.template init<events_type<T>>(_current_tick);

            _schedules.template add_system<internal_schedules::Main>(+[](resources_mut_type<events_type<T>> rm){
                auto& [events] = rm;

                events->swap();
            });

            return *this;
        }

        template<mytho::utils::PureEventType T>
        auto& event_write() noexcept {
            ASSURE(_resources.template exist<events_type<T>>(), "event type not exists");

            return _resources.template get<events_type<T>>().write();
        }

        template<mytho::utils::PureEventType T>
        auto& event_mutate() noexcept {
            ASSURE(_resources.template exist<events_type<T>>(), "event type not exists");

            return _resources.template get<events_type<T>>().mutate();
        }

        template<mytho::utils::PureEventType T>
        const auto& event_read() const noexcept {
            ASSURE(_resources.template exist<events_type<T>>(), "event type not exists");

            return _resources.template get<events_type<T>>().read();
        }

    public: // schedule operations
        template<auto ScheduleE>
        self_type& add_startup_schedule() {
            _schedules.template add_startup_schedule<ScheduleE>();

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_update_schedule() {
            _schedules.template add_update_schedule<ScheduleE>();

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_schedule() {
            _schedules.template add_schedule<ScheduleE>();

            return *this;
        }

        template<auto ScheduleE, auto BeforeScheduleE>
        self_type& add_schedule_before() {
            _schedules.template add_schedule_before<ScheduleE, BeforeScheduleE>();

            return *this;
        }

        template<auto ScheduleE, auto AfterScheduleE>
        self_type& add_schedule_after() {
            _schedules.template add_schedule_after<ScheduleE, AfterScheduleE>();

            return *this;
        }

        template<auto ScheduleE, auto InsertScheduleE>
        self_type& insert_schedule() {
            _schedules.template insert_schedule<ScheduleE, InsertScheduleE>();

            return *this;
        }

        template<auto ScheduleE>
        self_type& set_default_schedule() noexcept {
            _schedules.template set_default_schedule<ScheduleE>();

            return *this;
        }

    public: // system operations
        template<mytho::utils::FunctionType Func>
        static system_type system(Func&& func) noexcept {
            return system_type(std::forward<Func>(func));
        }

        template<mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            _schedules.add_system(std::forward<Func>(func));

            return *this;
        }

        template<auto ScheduleE, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            _schedules.template add_system<ScheduleE>(std::forward<Func>(func));

            return *this;
        }

        self_type& add_system(system_type& system) {
            _schedules.add_system(system);

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_system(system_type& system) {
            _schedules.template add_system<ScheduleE>(system);

            return *this;
        }

    public: // core operations
        void run() {
            _schedules.run(*this, _current_tick);
        }

        void exit() {
            _schedules.exit();
        }

        template<auto ScheduleE>
        void run_schedule() {
            _schedules.run_schedule<ScheduleE>(*this, _current_tick);
        }

    public: // command operations
        command_queue_type& command_queue() noexcept {
            return _command_queue;
        }

    public: // removed entities operations
        self_type& removed_entities_clear() noexcept {
            _components.removed_entities_clear();

            return *this;
        }

    private:
        entity_storage_type _entities;
        component_storage_type _components;
        resource_storage_type _resources;

        command_queue_type _command_queue;

        // tick start from 1, and 0 is reserved for init
        uint64_t _current_tick = 1;
        schedules_type _schedules;

    private:
        template<typename T, typename... Rs>
        auto get_cid_with_minimun_entities(internal::type_list<T, Rs...>) noexcept {
            return _get_cid_with_minimun_entities<T, Rs...>();
        }

        template<typename T, typename... Rs>
        std::optional<component_id_type> _get_cid_with_minimun_entities() noexcept {
            constexpr size_t N = 1 + sizeof...(Rs);

            component_id_type ids[N] = {component_id_generator::template gen<T>(), component_id_generator::template gen<Rs>()...};

            auto best = ids[0];
            auto size = _components.size();
            if (best >= size) return std::nullopt;

            auto* p = _components[best];
            if (!p) return std::nullopt;

            auto p_size = p->size();

            bool ok = [&]<size_t... I>(std::index_sequence<I...>){
                return (
                    [&]{
                        constexpr auto idx = I + 1;
                        auto id = ids[idx];

                        if (id >= size) return false;

                        auto* tp = _components[id];
                        if (!tp) return false;

                        auto tp_size = tp->size();

                        // branchless
                        size_t mask = -(size_t)(tp_size < p_size);
                        p_size = p_size ^ ((p_size ^ tp_size) & mask);
                        best = best ^ ((best ^ id) & mask);
                        return true;
                    }() && ...
                );
            }(std::make_index_sequence<sizeof...(Rs)>{});

            return ok ? std::optional{best} : std::nullopt;
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool component_list_contained(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            // _entities and _components are synchronized, just check one
            return _entities.template has<Ts...>(e) /* && _components.template contain<Ts...>(e) */;
        }

        template<mytho::utils::PureComponentType... Ts>
        bool component_list_not_contained(const entity_type& e, internal::type_list<Ts...>) const noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                // _entities and _components are synchronized, just check one
                return _entities.template not_has<Ts...>(e) /* && _components.template not_contain<Ts...>(e) */;
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