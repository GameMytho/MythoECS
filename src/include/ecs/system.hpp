#pragma once
#include <type_traits>
#include <optional>
#include <functional>
#include <utility>
#include <cstdint>
#include <vector>
#include <memory>
#include <queue>
#include <unordered_set>
#include <unordered_map>

#include "utils/assert.hpp"
#include "utils/type_list.hpp"
#include "ecs/registrar.hpp"
#include "ecs/commands.hpp"
#include "ecs/querier.hpp"
#include "ecs/resources.hpp"
#include "ecs/event.hpp"

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

        template<typename Ret, typename... Args>
        struct system_traits<Ret(Args...)> {
            using type = type_list<Args...>;
        };

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

namespace mytho::ecs::internal {
    // argument constructors
    template<typename RegistryT, typename ArgumentT>
    struct argument_constructor;

    template<typename RegistryT, typename... Ts>
    struct argument_constructor<RegistryT, basic_querier<RegistryT, Ts...>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return reg.template query<Ts...>(tick);
        }
    };

    template<typename RegistryT, typename... Ts>
    struct argument_constructor<RegistryT, basic_resources<Ts...>> {
        auto operator()(RegistryT& reg) const noexcept {
            return reg.template resources<Ts...>();
        }
    };

    template<typename RegistryT, typename... Ts>
    struct argument_constructor<RegistryT, basic_resources_mut<Ts...>> {
        auto operator()(RegistryT& reg) const noexcept {
            return reg.template resources_mut<Ts...>();
        }
    };

    template<typename RegistryT, typename T>
    struct argument_constructor<RegistryT, basic_event_writer<RegistryT, T>> {
        auto operator()(RegistryT& reg) const noexcept {
            return basic_event_writer<RegistryT, T>(reg);
        }
    };

    template<typename RegistryT, typename T>
    struct argument_constructor<RegistryT, basic_event_mutator<T>> {
        auto operator()(RegistryT& reg) const noexcept {
            return basic_event_mutator<T>(reg.template event_mutate<T>());
        }
    };

    template<typename RegistryT, typename T>
    struct argument_constructor<RegistryT, basic_event_reader<T>> {
        auto operator()(RegistryT& reg) const noexcept {
            return basic_event_reader<T>(reg.template event_read<T>());
        }
    };

    template<typename RegistryT, typename T>
    struct argument_constructor<RegistryT, basic_removed_entities<RegistryT, T>> {
        auto operator()(RegistryT& reg) const noexcept {
            return basic_removed_entities<RegistryT, T>(reg.template removed_entities<T>());
        }
    };

    // argument construct functions
    template<typename RegistryT>
    auto construct_registrar(RegistryT& reg, uint64_t tick) {
        return basic_registrar(reg, tick);
    }

    template<typename RegistryT>
    auto construct_commands(RegistryT& reg) {
        return basic_commands(reg);
    }

    template<typename RegistryT, typename QuerierT>
    auto construct_querier(RegistryT& reg, uint64_t tick) {
        return argument_constructor<RegistryT, QuerierT>{}(reg, tick);
    }

    template<typename RegistryT, typename ResourcesT>
    auto construct_resources(RegistryT& reg) {
        return argument_constructor<RegistryT, ResourcesT>{}(reg);
    }

    template<typename RegistryT, typename ResourcesMutT>
    auto construct_resources_mut(RegistryT& reg) {
        return argument_constructor<RegistryT, ResourcesMutT>{}(reg);
    }

    template<typename RegistryT, typename EventWriterT>
    auto construct_event_writer(RegistryT& reg) {
        return argument_constructor<RegistryT, EventWriterT>{}(reg);
    }

    template<typename RegistryT, typename EventMutatorT>
    auto construct_event_mutator(RegistryT& reg) {
        return argument_constructor<RegistryT, EventMutatorT>{}(reg);
    }

    template<typename RegistryT, typename EventReaderT>
    auto construct_event_reader(RegistryT& reg) {
        return argument_constructor<RegistryT, EventReaderT>{}(reg);
    }

    template<typename RegistryT, typename RemovedEntitiesT>
    auto construct_removed_entities(RegistryT& reg) {
        return argument_constructor<RegistryT, RemovedEntitiesT>{}(reg);
    }

    template<typename RegistryT, typename T>
    auto construct(RegistryT& reg, uint64_t tick) {
        if constexpr (mytho::utils::is_registrar_v<T>) {
            return construct_registrar(reg, tick);
        } else if constexpr (mytho::utils::is_commands_v<T>) {
            return construct_commands(reg);
        } else if constexpr (mytho::utils::is_querier_v<T>) {
            return construct_querier<RegistryT, T>(reg, tick);
        } else if constexpr (mytho::utils::is_resources_v<T>) {
            return construct_resources<RegistryT, T>(reg);
        } else if constexpr (mytho::utils::is_resources_mut_v<T>) {
            return construct_resources_mut<RegistryT, T>(reg);
        } else if constexpr (mytho::utils::is_event_writer_v<T>) {
            return construct_event_writer<RegistryT, T>(reg);
        } else if constexpr (mytho::utils::is_event_mutator_v<T>) {
            return construct_event_mutator<RegistryT, T>(reg);
        } else if constexpr (mytho::utils::is_event_reader_v<T>) {
            return construct_event_reader<RegistryT, T>(reg);
        } else if constexpr (mytho::utils::is_removed_entities_v<T>) {
            return construct_removed_entities<RegistryT, T>(reg);
        } else {
            ASSURE(false, "Unsupport type, please check the args of systems!");
        }
    }

    struct function_pointer_hash {
        size_t operator()(void* p) const noexcept {
            size_t v = reinterpret_cast<size_t>(p);
            v ^= v >> 16;
            v *= 0x85ebca6b;
            v ^= v >> 13;
            v *= 0xc2b2ae35;
            v ^= v >> 16;
            return v;
        }
    };

    template<typename RegistryT, typename ReturnT>
    class basic_function {
    public:
        using registry_type = RegistryT;
        using return_type = ReturnT;
        using function_wrapper_type = return_type(*)(void*, registry_type&, uint64_t);

        basic_function() noexcept : _function_wrapper(nullptr), _function_pointer(nullptr) {}

        template<mytho::utils::FunctionType Func>
        basic_function(Func&& func) noexcept {
            new (&_function_pointer) std::decay_t<Func>(std::forward<Func>(func));
            _function_wrapper = function_wrapper_construct<Func>();
        }

    public:
        return_type operator()(registry_type& reg, uint64_t tick) noexcept {
            return _function_wrapper(_function_pointer, reg, tick);
        }

    public:
        void* pointer() const noexcept { return _function_pointer; }

    private:
        function_wrapper_type _function_wrapper;
        void* _function_pointer;

    private:
        template<typename Func>
        auto function_wrapper_construct() noexcept {
            return [](void* func_ptr, registry_type& reg, uint64_t tick) -> return_type {
                using types = mytho::utils::system_traits_t<mytho::utils::function_traits_t<std::decay_t<Func>>>;

                return function_invoke(*reinterpret_cast<std::decay_t<Func>>(func_ptr), reg, tick, types{});
            };
        }

        template<typename Func, typename... Ts>
        static return_type function_invoke(Func&& func, registry_type& reg, uint64_t tick, mytho::utils::type_list<Ts...>) noexcept {
            return std::invoke(std::forward<Func>(func), construct<registry_type, Ts>(reg, tick)...);
        }
    };

    template<typename RegistryT>
    class basic_function<RegistryT, void> {
    public:
        using registry_type = RegistryT;
        using function_wrapper_type = void(*)(void*, registry_type&, uint64_t);

        basic_function() noexcept : _function_wrapper(nullptr), _function_pointer(nullptr) {}

        template<mytho::utils::FunctionType Func>
        basic_function(Func&& func) noexcept {
            new (&_function_pointer) std::decay_t<Func>(std::forward<Func>(func));
            _function_wrapper = function_wrapper_construct<Func>();
        }

    public:
        void operator()(registry_type& reg, uint64_t tick) noexcept {
            _function_wrapper(_function_pointer, reg, tick);
        }

    public:
        void* pointer() const noexcept { return _function_pointer; }

    private:
        function_wrapper_type _function_wrapper;
        void* _function_pointer;

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
}

namespace mytho::ecs::internal {
    template<typename RegistryT>
    class basic_system {
    public:
        using registry_type = RegistryT;
        using function_type = basic_function<registry_type, void>;
        using runif_type = basic_function<registry_type, bool>;

        template<mytho::utils::FunctionType Func>
        basic_system(Func&& func) noexcept : _function(std::forward<Func>(func)) {}

        basic_system(const function_type& func, const runif_type& runif) noexcept : _function(func), _runif(runif) {}

    public:
        bool should_run(registry_type& reg) noexcept {
            return !_runif.pointer() || _runif(reg, _last_run_tick);
        }

        void operator()(registry_type& reg, uint64_t tick) noexcept {
            _function(reg, _last_run_tick);
            _last_run_tick = tick;
        }

    private:
        uint64_t _last_run_tick = 0;
        function_type _function;
        runif_type _runif;
    };

    template<typename RegistryT>
    class basic_system_config {
    public:
        using registry_type = RegistryT;
        using self_type = basic_system_config<registry_type>;
        using function_type = basic_function<registry_type, void>;
        using runif_type = basic_function<registry_type, bool>;
        using afters_type = std::unordered_set<void*, function_pointer_hash>;
        using befores_type = std::unordered_set<void*, function_pointer_hash>;

        basic_system_config() noexcept : _function() {}

        template<mytho::utils::FunctionType Func>
        basic_system_config(Func&& func) noexcept : _function(std::forward<Func>(func)) {}

    public:
        template<mytho::utils::FunctionType Func>
        self_type& after(Func&& func) noexcept {
            _afters.emplace(reinterpret_cast<void*>(std::forward<Func>(func)));
            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& before(Func&& func) noexcept {
            _befores.emplace(reinterpret_cast<void*>(std::forward<Func>(func)));
            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& runif(Func&& func) noexcept {
            _runif = std::forward<Func>(func);
            return *this;
        }

    public:
        const function_type& function() const noexcept { return _function; }

        const runif_type& runif() const noexcept { return _runif; }

        const afters_type& afters() const noexcept { return _afters; }
        afters_type& afters() noexcept { return _afters; }

        const befores_type& befores() const noexcept { return _befores; }
        befores_type& befores() noexcept { return _befores; }

    private:
        function_type _function;
        runif_type _runif;
        afters_type _afters;
        befores_type _befores;
    };

    template<typename RegistryT>
    class basic_system_storage {
    public:
        using registry_type = RegistryT;
        using system_type = basic_system<registry_type>;
        using system_config_type = basic_system_config<registry_type>;
        using systems_type = std::vector<system_type>;
        using configs_type = std::unordered_map<void*, system_config_type, function_pointer_hash>;

        basic_system_storage() = default;
        basic_system_storage(basic_system_storage& ss) = delete;

        basic_system_storage(basic_system_storage&& ss) noexcept
            : _systems(std::move(ss.systems())), _configs(std::move(ss.configs())) {}

        basic_system_storage& operator=(basic_system_storage&& ss) noexcept {
            _systems = std::move(ss.systems());
            _configs = std::move(ss.configs());

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        void add(Func&& func) noexcept {
            _configs.emplace(reinterpret_cast<void*>(std::forward<Func>(func)), system_config_type(std::forward<Func>(func)));
        }

        void add(const system_config_type& config) noexcept {
            _configs.emplace(config.function().pointer(), config);
        }

    public:
        void ready() noexcept {
            ASSURE(valid_dependencies(), "There are some system dependencies not found in the system dependencies map!");

            for (auto& [ptr, config] : _configs) {
                for (auto& before : config.befores()) {
                    _configs[before].afters().emplace(ptr);
                }
                config.befores().clear();
            }

            kahn();
        }

        void run(registry_type& reg, uint64_t& tick) noexcept {
            for (auto& system : _systems) {
                if (system.should_run(reg)) {
                    system(reg, ++tick);
                }
            }
        }

    public:
        auto begin() noexcept { return _systems.begin(); }

        auto end() noexcept { return _systems.end(); }

        auto size() const noexcept { return _systems.size(); }

        auto& systems() noexcept { return _systems; }

        auto& configs() noexcept { return _configs; }

    private:
        systems_type _systems;
        configs_type _configs;

    private:
        bool valid_dependencies() const noexcept {
            for (auto& [ptr, config] : _configs) {
                for (auto& after : config.afters()) {
                    if (!_configs.contains(after)) {
                        return false;
                    }
                }

                for (auto& before : config.befores()) {
                    if (!_configs.contains(before)) {
                        return false;
                    }
                }
            }

            return true;
        }

        void kahn() noexcept {
            const std::size_t sys_count = _configs.size();
            _systems.reserve(sys_count);

            std::unordered_map<void*, int> in_degree;
            in_degree.reserve(sys_count);
            std::unordered_map<void*, std::vector<void*>> adj;
            adj.reserve(sys_count);

            for (auto& [ptr, config] : _configs) {
                in_degree.emplace(ptr, 0);
                for (auto after : config.afters()) {
                    if (!_configs.contains(after)) continue;
                    adj[after].push_back(ptr);
                    ++in_degree[ptr];
                }
            }

            std::queue<void*> zero_in_degree;
            for (auto& [ptr, ind] : in_degree) {
                if (ind == 0) {
                    zero_in_degree.push(ptr);
                }
            }

            std::size_t count = 0;
            while(!zero_in_degree.empty()) {
                void* ptr = zero_in_degree.front();
                zero_in_degree.pop();

                ++count;

                if (auto it = _configs.find(ptr); it != _configs.end()) {
                    _systems.emplace_back(it->second.function(), it->second.runif());
                    _configs.erase(it);
                }

                if (auto item = adj.find(ptr); item != adj.end()) {
                    for (auto it : item->second) {
                        if (auto i = in_degree.find(it); i != in_degree.end() && --(i->second) == 0) {
                            zero_in_degree.push(it);
                        }
                    }
                }
            }

            ASSURE(count == sys_count, "Cycle detected in system dependencies!");
        }
    };
}