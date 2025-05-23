#pragma once
#include <vector>
#include "utils/assert.hpp"
#include "ecs/commands.hpp"
#include "ecs/querier.hpp"

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
            auto operator()(RegistryT& reg) const noexcept {
                return reg.template query<Ts...>();
            }
        };

        template<typename RegistryT, typename QuerierT>
        auto construct_querier(RegistryT& reg) {
            return querier_constructor<RegistryT, QuerierT>{}(reg);
        }

        template<typename RegistryT, typename T>
        auto construct(RegistryT& reg) {
            if constexpr (mytho::utils::is_commands_v<T>) {
                return construct_commands(reg);
            } else if constexpr (mytho::utils::is_querier_v<T>) {
                return construct_querier<RegistryT, T>(reg);
            } else {
                ASSURE(false, "Unsupport type, please check the args of systems!");
            }
        }

        template<typename RegistryT>
        class basic_system_type {
        public:
            using registry_type = RegistryT;
            using function_type = void(*)(registry_type&);

        public:
            basic_system_type(function_type func) : _func(func) {}

            void operator()(registry_type& reg) const noexcept {
                _func(reg);
            }

        private:
            function_type _func{};
        };

        template<typename RegistryT>
        class basic_system_storage {
        public: 
            using registry_type = RegistryT;
            using system_type = basic_system_type<registry_type>;
            using systems_type = std::vector<system_type>;

        public:
            template<auto Func>
            void add() noexcept {
                _systems.emplace_back([](registry_type& reg){
                    using types = mytho::utils::system_traits_t<mytho::utils::function_traits_t<std::decay_t<decltype(Func)>>>;

                    system_invoke<Func>(reg, types{});
                });
            }

        public:
            auto begin() const noexcept { return _systems.begin(); }

            auto end() const noexcept { return _systems.end(); }

        private:
            systems_type _systems;

        private:
            template<auto Func, typename... Ts>
            static void system_invoke(registry_type& reg, mytho::utils::type_list<Ts...>) noexcept {
                std::invoke(Func, construct<RegistryT, Ts>(reg)...);
            }
        };
    }
}