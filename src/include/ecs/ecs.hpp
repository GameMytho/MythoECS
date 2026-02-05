#pragma once
#include "ecs/registry.hpp"

namespace mecs {
    template<typename... Ts>
    using Mut = mytho::ecs::mut<Ts...>;

    template<typename... Ts>
    using With = mytho::ecs::with<Ts...>;

    template<typename... Ts>
    using Without = mytho::ecs::without<Ts...>;

    template<typename... Ts>
    using Added = mytho::ecs::added<Ts...>;

    template<typename... Ts>
    using Changed = mytho::ecs::changed<Ts...>;

    template<typename T>
    using DataWrapper = mytho::utils::internal::data_wrapper<T>;

    template<typename... Ts>
    using Res = mytho::ecs::basic_resources<Ts...>;

    template<typename... Ts>
    using ResMut = mytho::ecs::basic_resources_mut<Ts...>;

    using Entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    using Registry = mytho::ecs::basic_registry<Entity, uint16_t, uint16_t, uint8_t, 256>;
    using Commands = mytho::ecs::basic_commands<Registry>;

    template<typename... Ts>
    using Querier = mytho::ecs::basic_querier<Registry, Ts...>;

    template<typename T>
    using EventWriter = mytho::ecs::basic_event_writer<T>;

    template<typename T>
    using EventMutator = mytho::ecs::basic_event_mutator<T>;

    template<typename T>
    using EventReader = mytho::ecs::basic_event_reader<T>;

    template<typename T>
    using RemovedEntities = mytho::ecs::basic_removed_entities<Registry, T>;

    using StartupSchedules = mytho::ecs::startup_schedules;

    using MainSchedules = mytho::ecs::main_schedules;

    template<mytho::utils::FunctionType Func>
    auto system(Func&& func) {
        return Registry::system(std::forward<Func>(func));
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool ComponentsAdded(Commands cmds) {
        return cmds.template components_added<T, Rs...>();
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool ComponentsChanged(Commands cmds) {
        return cmds.template components_changed<T, Rs...>();
    }

    template<mytho::utils::PureComponentType T, mytho::utils::PureComponentType... Rs>
    bool ComponentsRemoved(Commands cmds) {
        return cmds.template components_removed<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool ResourcesExist(Commands cmds) {
        return cmds.template resources_exist<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool ResourcesAdded(Commands cmds) {
        return cmds.template resources_added<T, Rs...>();
    }

    template<mytho::utils::PureResourceType T, mytho::utils::PureResourceType... Rs>
    bool ResourcesChanged(Commands cmds) {
        return cmds.template resources_changed<T, Rs...>();
    }
}