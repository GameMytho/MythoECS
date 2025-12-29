#pragma once
#include "ecs/registry.hpp"

namespace {
    template<typename... Ts>
    using mut = mytho::ecs::mut<Ts...>;

    template<typename... Ts>
    using with = mytho::ecs::with<Ts...>;

    template<typename... Ts>
    using without = mytho::ecs::without<Ts...>;

    template<typename... Ts>
    using added = mytho::ecs::added<Ts...>;

    template<typename... Ts>
    using changed = mytho::ecs::changed<Ts...>;

    template<typename T>
    using data_wrapper = mytho::utils::internal::data_wrapper<T>;

    template<typename... Ts>
    using res = mytho::ecs::basic_resources<Ts...>;

    template<typename... Ts>
    using res_mut = mytho::ecs::basic_resources_mut<Ts...>;

    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    using registry = mytho::ecs::basic_registry<entity, uint16_t, uint16_t, uint8_t, 256>;
    using commands = mytho::ecs::basic_commands<registry>;

    template<typename... Ts>
    using querier = mytho::ecs::basic_querier<registry, Ts...>;

    template<typename T>
    using event_writer = mytho::ecs::basic_event_writer<T>;

    template<typename T>
    using event_mutator = mytho::ecs::basic_event_mutator<T>;

    template<typename T>
    using event_reader = mytho::ecs::basic_event_reader<T>;

    template<typename T>
    using removed_entities = mytho::ecs::basic_removed_entities<registry, T>;

    template<mytho::utils::FunctionType Func>
    auto system(Func&& func) {
        return registry::system(std::forward<Func>(func));
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool components_added(commands cmd) {
        return cmd.template components_added<T, Rs...>();
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool components_changed(commands cmd) {
        return cmd.template components_changed<T, Rs...>();
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool components_removed(commands cmd) {
        return cmd.template components_removed<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool resources_exist(commands cmd) {
        return cmd.template resources_exist<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool resources_added(commands cmd) {
        return cmd.template resources_added<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool resources_changed(commands cmd) {
        return cmd.template resources_changed<T, Rs...>();
    }
}