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

    using Entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    using Registry = mytho::ecs::basic_registry<Entity, uint16_t, uint16_t, uint8_t, 256>;
    using Commands = typename Registry::commands_type;

    template<typename... Ts>
    using Querier = typename Registry::querier_type<Ts...>;

    template<typename... Ts>
    using Res = typename Registry::resources_type<Ts...>;

    template<typename... Ts>
    using ResMut = typename Registry::resources_mut_type<Ts...>;

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

    template<typename T>
    using State = mytho::ecs::basic_state<T>;

    template<typename T>
    using NextState = mytho::ecs::basic_next_state<T>;

    template<auto E>
    using OnEnter = mytho::ecs::on_enter<E>;

    template<auto E>
    using OnExit = mytho::ecs::on_exit<E>;

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