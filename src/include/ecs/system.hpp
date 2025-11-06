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
    class basic_function final {
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
        return_type operator()(registry_type& reg, uint64_t tick) {
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
        static return_type function_invoke(Func&& func, registry_type& reg, uint64_t tick, mytho::utils::type_list<Ts...>) {
            return std::invoke(std::forward<Func>(func), construct<registry_type, Ts>(reg, tick)...);
        }
    };

    template<typename RegistryT>
    class basic_function<RegistryT, void> final {
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
        void operator()(registry_type& reg, uint64_t tick) const {
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
        static void function_invoke(Func&& func, registry_type& reg, uint64_t tick, mytho::utils::type_list<Ts...>) {
            std::invoke(std::forward<Func>(func), construct<registry_type, Ts>(reg, tick)...);
        }
    };
}

namespace mytho::ecs::internal {
    template<typename RegistryT>
    class basic_system final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_system<registry_type>;
        using function_type = basic_function<registry_type, void>;
        using runif_type = basic_function<registry_type, bool>;

        basic_system() noexcept = default;

        template<mytho::utils::FunctionType Func>
        basic_system(Func&& func) noexcept : _function(std::forward<Func>(func)) {}

    public:
        template<mytho::utils::FunctionType Func>
        self_type& after(Func&& func) {
            _meta._afters.push_back(reinterpret_cast<void*>(func));

            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& before(Func&& func) {
            _meta._befores.push_back(reinterpret_cast<void*>(func));

            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& runif(Func&& func) {
            _meta._runif = std::forward<Func>(func);

            return *this;
        }

    public:
        bool should_run(registry_type& reg) {
            return !_meta._runif.pointer() || _meta._runif(reg, _meta._last_run_tick);
        }

        void operator()(registry_type& reg, uint64_t tick) {
            _function(reg, _meta._last_run_tick);
            _meta._last_run_tick = tick;
        }

    public:
        const auto& function() const noexcept {
            return _function;
        }

        const auto& meta() const noexcept {
            return _meta;
        }

    private:
        struct system_meta {
            using befores_type = std::vector<void*>;
            using afters_type = std::vector<void*>;

            uint64_t _last_run_tick = 0;
            runif_type _runif;
            befores_type _befores;
            afters_type _afters;
        };

        function_type _function;
        system_meta _meta;
    };

    template<typename RegistryT, typename SystemIdT = uint16_t>
    class basic_system_stage final {
    public:
        using registry_type = RegistryT;
        using system_id_type = SystemIdT;
        using self_type = basic_system_stage<registry_type, system_id_type>;
        using system_type = basic_system<registry_type>;
        using systems_type = std::unordered_map<void*, system_type, function_pointer_hash>;
        using frame_systems_type = std::vector<std::vector<system_type*>>;
        using size_type = typename systems_type::size_type;

        basic_system_stage() = default;
        basic_system_stage(basic_system_stage& ss) = delete;

        basic_system_stage(basic_system_stage&& ss) noexcept
            : _systems(std::move(ss.systems())) {}

        basic_system_stage& operator=(basic_system_stage&& ss) noexcept {
            _systems = std::move(ss.systems());

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        void add(Func&& func) {
            _systems.emplace(reinterpret_cast<void*>(func), system_type{ std::forward<Func>(func) });
        }

        void add(const system_type& system) {
            _systems.emplace(system.function().pointer(), system);
        }

    public:
        void run(registry_type& reg, uint64_t& tick) {
            std::vector<void*> valid_systems;

            for (auto& [ptr, system] : _systems) {
                if (system.should_run(reg)) {
                    valid_systems.push_back(ptr);
                }
            }

            std::vector<system_type*> result;
            result.reserve(valid_systems.size());

            build(valid_systems, result);

            for (auto sp : result) {
                (*sp)(reg, tick);
            }
        }

    public:
        auto begin() noexcept { return _systems.begin(); }

        auto end() noexcept { return _systems.end(); }

        auto size() const noexcept { return _systems.size(); }

        auto& systems() noexcept { return _systems; }

    private:
        systems_type _systems;

    private:
        void build(auto& valid_ptrs, auto& result) {
            std::unordered_map<void*, std::unordered_set<void*>, function_pointer_hash> edges(valid_ptrs.size());
            std::unordered_map<void*, size_t> in_degrees;

            for (auto p : valid_ptrs) {
                edges.emplace(p, std::unordered_set<void*>());
                in_degrees.emplace(p, 0);
            }

            for (auto p : valid_ptrs) {
                auto& system = _systems[p];

                for (auto before : system.meta()._befores) {
                    if (!edges.contains(before)) {
                        continue;
                    }

                    if (edges[p].insert(before).second) {
                        in_degrees[before]++;
                    }
                }

                for (auto after : system.meta()._afters) {
                    if (!edges.contains(after)) {
                        continue;
                    }

                    if (edges[after].insert(p).second) {
                        in_degrees[p]++;
                    }
                }
            }

            kahn(edges, in_degrees, result);
        }

        void kahn(auto& edges, auto& in_degrees, auto& result) {
            std::queue<void*> q;
            for (auto [p, degree] : in_degrees) {
                if (degree == 0) {
                    q.push(p);
                }
            }

            while(!q.empty()) {
                auto p = q.front();
                q.pop();
                result.push_back(&_systems[p]);

                if (edges.contains(p)) {
                    for (auto tp : edges[p]) {
                        if (--in_degrees[tp] == 0) {
                            q.push(tp);
                        }
                    }
                }
            }

            ASSURE(result.size() == in_degrees.size(), "Cycle detected in system dependencies.");
        }
    };
}