#pragma once
#include <type_traits>
#include <optional>
#include <functional>
#include <utility>
#include <cstdint>
#include <vector>
#include <memory>

#include "utils/func_list.hpp"
#include "ecs/commands.hpp"
#include "ecs/querier.hpp"
#include "ecs/resources.hpp"

namespace mytho::utils {
    namespace internal {
        template<typename T>
        struct function_traits;

        template<typename Ret, typename... Args>
        struct function_traits<Ret(Args...)> {
            using type = Ret(Args...);
        };

        template<typename Ret, typename... Args>
        struct function_traits<Ret(*)(Args...)> {
            using type = Ret(Args...);
        };
    }

    template<typename T>
    using function_traits_t = typename internal::function_traits<T>::type;

    namespace internal {
        template<typename T>
        struct system_traits;

        template<typename... Args>
        struct system_traits<void(Args...)> {
            using type = type_list<Args...>;
        };
    }

    template<typename T>
    using system_traits_t = typename internal::system_traits<T>::type;

    template<typename F>
    concept FunctionType = requires(F f) {
        {+f} -> std::same_as<decltype(+f)>;
    };
}

namespace mytho::ecs {
    namespace internal {
        template<typename RegistryT>
        auto construct_commands(RegistryT& reg) {
            return basic_commands(reg);
        }

        template<typename RegistryT, typename QuerierT>
        struct querier_constructor;

        template<typename RegistryT, typename... Ts>
        struct querier_constructor<RegistryT, basic_querier<RegistryT, Ts...>> {
            auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
                return reg.template query<Ts...>(tick);
            }
        };

        template<typename RegistryT, typename QuerierT>
        auto construct_querier(RegistryT& reg, uint64_t tick) {
            return querier_constructor<RegistryT, QuerierT>{}(reg, tick);
        }

        template<typename RegistryT, typename ResourcesT>
        struct resources_constructor;

        template<typename RegistryT, typename... Ts>
        struct resources_constructor<RegistryT, basic_resources<Ts...>> {
            auto operator()(RegistryT& reg) const noexcept {
                return reg.template resources<Ts...>();
            }
        };

        template<typename RegistryT, typename ResourcesT>
        auto construct_resources(RegistryT& reg) {
            return resources_constructor<RegistryT, ResourcesT>{}(reg);
        }

        template<typename RegistryT, typename ResourcesMutT>
        struct resources_mut_constructor;

        template<typename RegistryT, typename... Ts>
        struct resources_mut_constructor<RegistryT, basic_resources_mut<Ts...>> {
            auto operator()(RegistryT& reg) const noexcept {
                return reg.template resources_mut<Ts...>();
            }
        };

        template<typename RegistryT, typename ResourcesMutT>
        auto construct_resources_mut(RegistryT& reg) {
            return resources_mut_constructor<RegistryT, ResourcesMutT>{}(reg);
        }

        template<typename RegistryT, typename T>
        auto construct(RegistryT& reg, uint64_t tick) {
            if constexpr (mytho::utils::is_commands_v<T>) {
                return construct_commands(reg);
            } else if constexpr (mytho::utils::is_querier_v<T>) {
                return construct_querier<RegistryT, T>(reg, tick);
            } else if constexpr (mytho::utils::is_resources_v<T>) {
                return construct_resources<RegistryT, T>(reg);
            } else if constexpr (mytho::utils::is_resources_mut_v<T>) {
                return construct_resources_mut<RegistryT, T>(reg);
            } else {
                ASSURE(false, "Unsupport type, please check the args of systems!");
            }
        }

        template<typename RegistryT>
        class basic_function_wrapper {
        public:
            using registry_type = RegistryT;
            using function_type = void(*)(void*, registry_type&, uint64_t);

            template<mytho::utils::FunctionType Func>
            basic_function_wrapper(Func&& func) noexcept {
                new (&_func_ptr) std::decay_t<Func>(std::forward<Func>(func));
                _func_wrapper = function_wrapper_construct<Func>();
            }

        public:
            void operator()(registry_type& reg, uint64_t tick) noexcept {
                _func_wrapper(_func_ptr, reg, tick);
            }

        private:
            function_type _func_wrapper = nullptr;
            void* _func_ptr = nullptr;

        private:
            template<typename Func>
            auto function_wrapper_construct() noexcept {
                return [](void* func_ptr, registry_type& reg, uint64_t tick) {
                    using types = mytho::utils::system_traits_t<mytho::utils::function_traits_t<std::decay_t<Func>>>;

                    function_invoke(*reinterpret_cast<std::decay_t<Func>>(func_ptr), reg, tick, types{});
                };
            }

            template<typename Func, typename... Ts>
            static void function_invoke(Func&& func, registry_type& reg, uint64_t tick, mytho::utils::type_list<Ts...>) noexcept {
                std::invoke(std::forward<Func>(func), construct<registry_type, Ts>(reg, tick)...);
            }
        };

        template<typename RegistryT>
        class basic_system {
        public:
            using registry_type = RegistryT;
            using function_wrapper_type = basic_function_wrapper<registry_type>;

            template<mytho::utils::FunctionType Func>
            basic_system(Func&& func) noexcept : _wrapper(std::forward<Func>(func)) {}

        public:
            void operator()(registry_type& reg, uint64_t tick) noexcept {
                _wrapper(reg, _last_run_tick);
                _last_run_tick = tick;
            }

        private:
            uint64_t _last_run_tick = 0;
            function_wrapper_type _wrapper;
        };

        template<typename RegistryT>
        class basic_system_storage {
        public:
            using registry_type = RegistryT;
            using system_type = basic_system<registry_type>;
            using systems_type = std::vector<system_type>;

        public:
            template<mytho::utils::FunctionType Func>
            void add(Func&& func) noexcept {
                _systems.emplace_back(std::forward<Func>(func));
            }

        public:
            auto begin() noexcept { return _systems.begin(); }

            auto end() noexcept { return _systems.end(); }

            auto size() const noexcept { return _systems.size(); }

        private:
            systems_type _systems;
        };
    }
}