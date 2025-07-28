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
    using registry = mytho::ecs::basic_registry<entity, uint8_t, 1024>;
    using commands = mytho::ecs::basic_commands<registry>;

    template<typename... Ts>
    using querier = mytho::ecs::basic_querier<registry, Ts...>;

    template<mytho::utils::FunctionType Func>
    auto system(Func&& func) {
        return registry::system(std::forward<Func>(func));
    }
}