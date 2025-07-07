#pragma once
#include <vector>
#include <optional>
#include "utils/func_list.hpp"
#include "ecs/commands.hpp"
#include "ecs/querier.hpp"
#include "ecs/resources.hpp"
#include "container/indexed_list.hpp"

namespace mytho::ecs {
    template<auto... Funcs>
    struct before {};

    template<auto... Funcs>
    struct after {};
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_before_v = internal::is_location_v<T, mytho::ecs::before>;

    template<typename T>
    inline constexpr bool is_after_v = internal::is_location_v<T, mytho::ecs::after>;

    template<typename T>
    concept LocationType = is_before_v<T> || is_after_v<T>;

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
        template<auto... Ts>
        using func_list = mytho::utils::func_list<Ts...>;

        template<typename... Ls>
        using func_list_cat_t = typename mytho::utils::func_list_cat_t<Ls...>;

        template<typename L, template<auto...> typename E>
        using func_list_extract_template_t = typename mytho::utils::func_list_extract_template_t<L, E>;

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
        struct basic_system_type {
            using registry_type = RegistryT;
            using function_type = void(*)(registry_type&, uint64_t);

            basic_system_type(function_type func) : _func(func) {}

            void enable() noexcept {
                _enabled = true;
            }

            void disable() noexcept {
                _enabled = false;
            }

            bool enabled() const noexcept {
                return _enabled;
            }

            void operator()(registry_type& reg, uint64_t tick) noexcept {
                _func(reg, _last_run_tick);
                _last_run_tick = tick;
            }

            bool _enabled = true;
            uint64_t _last_run_tick = 0;
            function_type _func{};
        };

        template<typename L, template<auto...> typename T>
        struct location_traits;

        template<template<auto...> typename T, mytho::utils::LocationType... Locs>
        struct location_traits<type_list<Locs...>, T> {
            using type = func_list_cat_t<mytho::utils::internal::rm_location_t<Locs, T>...>;
        };

        template<typename L, template<auto...> typename T>
        using location_traits_t = typename location_traits<L, T>::type;

        template<mytho::utils::LocationType... Locs>
        struct location_types {
            using location_list = type_list<Locs...>;
            using before_list = location_traits_t<func_list_extract_template_t<location_list, before>, before>;
            using after_list = location_traits_t<func_list_extract_template_t<location_list, after>, after>;
        };

        template<typename RegistryT, mytho::utils::UnsignedIntegralType SystemIndexT>
        class basic_system_storage {
        public: 
            using registry_type = RegistryT;
            using system_index_type = SystemIndexT;
            using system_type = basic_system_type<registry_type>;
            using systems_type = mytho::container::indexed_list<system_type, system_index_type>;

            inline static system_index_type system_index_null = systems_type::index_null;

        public:
            template<auto Func, mytho::utils::LocationType... Locs>
            void add() noexcept {
                if constexpr (sizeof...(Locs) > 0) {
                    _add<Func, location_types<Locs...>>();
                } else {
                    _systems.emplace_back(function_construct<Func>());
                }
            }

            template<auto Func>
            void remove() noexcept {
                auto func = function_construct<Func>();
                while(1) {
                    system_index_type idx = _systems.find_if([func](const system_type& sys){
                        return sys._func == func;
                    });

                    if (idx == system_index_null) {
                        break;
                    }

                    _systems.pop(idx);
                }
            }

            template<auto Func>
            void enable() noexcept {
                auto func = function_construct<Func>();
                for (auto& it : _systems) {
                    if (it._func == func) {
                        it.enable();
                    }
                }
            }

            template<auto Func>
            void disable() noexcept {
                auto func = function_construct<Func>();
                for (auto& it : _systems) {
                    if (it._func == func) {
                        it.disable();
                    }
                }
            }

            void sort() noexcept {
                _systems.sort();
            }

        public:
            auto begin() noexcept { return _systems.begin(); }

            auto end() noexcept { return _systems.end(); }

        private:
            systems_type _systems;

        private:
            template<auto Func, typename LocT>
            void _add() noexcept {
                using before_list = typename LocT::before_list;
                using after_list = typename LocT::after_list;

                auto bf_idx = get_list_index(before_list{}, [](system_index_type l, system_index_type r){ return l < r ? l : r; });
                if (!bf_idx) {
                    return;
                }

                auto af_idx = get_list_index(after_list{}, [](system_index_type l, system_index_type r){ return l > r ? l : r; });
                if (!af_idx) {
                    return;
                }

                add_with_locations<Func>(af_idx.value() == system_index_null ? 0 : af_idx.value() + 1, bf_idx.value());
            }

            template<typename Compare, auto... Funcs>
            std::optional<system_index_type> get_list_index(func_list<Funcs...> l, Compare c) const noexcept {
                if constexpr (sizeof...(Funcs) == 0) {
                    return system_index_null;
                } else {
                    return _get_list_index<Compare, Funcs...>(c);
                }
            }

            template<auto Func>
            void add_with_locations(system_index_type front, system_index_type back) noexcept {
                if (front > back) {
                    return;
                }

                _systems.emplace(back == system_index_null ? front : back, function_construct<Func>());
            }

        private:
            template<typename Compare, auto Func, auto... Funcs>
            std::optional<system_index_type> _get_list_index(Compare c) const noexcept {
                auto idx = get_system_index<Func>();

                if constexpr (sizeof...(Funcs) > 0) {
                    auto i = _get_list_index<Compare, Funcs...>(c);
                    if (i) {
                        if (idx == system_index_null) {
                            return i;
                        } else {
                            return c(i.value(), idx);
                        }
                    }
                }

                if (idx == system_index_null) {
                    return std::nullopt;
                } else {
                    return idx;
                }
            }

            template<auto Func>
            system_index_type get_system_index() const noexcept {
                auto func = function_construct<Func>();
                return _systems.find_if([func](const system_type& sys){
                    return sys._func == func;
                });
            }

            template<auto Func>
            auto function_construct() const noexcept {
                return [](registry_type& reg, uint64_t tick){
                    using types = mytho::utils::system_traits_t<mytho::utils::function_traits_t<std::decay_t<decltype(Func)>>>;

                    function_invoke<Func>(reg, tick, types{});
                };
            }

            template<auto Func, typename... Ts>
            static void function_invoke(registry_type& reg, uint64_t tick, mytho::utils::type_list<Ts...>) noexcept {
                std::invoke(Func, construct<RegistryT, Ts>(reg, tick)...);
            }
        };
    }
}