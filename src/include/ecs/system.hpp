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
#include "container/sparse_set.hpp"
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
    struct constructor;

    template<typename RegistryT>
    struct constructor<RegistryT, basic_registrar<RegistryT>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_registrar(reg, tick);
        }
    };

    template<typename RegistryT>
    struct constructor<RegistryT, basic_commands<RegistryT>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_commands(reg);
        }
    };

    template<typename RegistryT, typename... Ts>
    struct constructor<RegistryT, basic_querier<RegistryT, Ts...>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return reg.template query<Ts...>(tick);
        }
    };

    template<typename RegistryT, typename... Ts>
    struct constructor<RegistryT, basic_resources<Ts...>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return reg.template resources<Ts...>();
        }
    };

    template<typename RegistryT, typename... Ts>
    struct constructor<RegistryT, basic_resources_mut<Ts...>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return reg.template resources_mut<Ts...>();
        }
    };

    template<typename RegistryT, typename T>
    struct constructor<RegistryT, basic_event_writer<RegistryT, T>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_event_writer<RegistryT, T>(reg);
        }
    };

    template<typename RegistryT, typename T>
    struct constructor<RegistryT, basic_event_mutator<T>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_event_mutator<T>(reg.template event_mutate<T>());
        }
    };

    template<typename RegistryT, typename T>
    struct constructor<RegistryT, basic_event_reader<T>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_event_reader<T>(reg.template event_read<T>());
        }
    };

    template<typename RegistryT, typename T>
    struct constructor<RegistryT, basic_removed_entities<RegistryT, T>> {
        auto operator()(RegistryT& reg, uint64_t tick) const noexcept {
            return basic_removed_entities<RegistryT, T>(reg.template removed_entities<T>());
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
            return std::invoke(std::forward<Func>(func), constructor<registry_type, Ts>{}(reg, tick)...);
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
            std::invoke(std::forward<Func>(func), constructor<registry_type, Ts>{}(reg, tick)...);
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
        using befores_type = std::vector<void*>;
        using afters_type = std::vector<void*>;

        basic_system() noexcept = default;

        template<mytho::utils::FunctionType Func>
        basic_system(Func&& func) noexcept : _function(std::forward<Func>(func)) {}

    public:
        template<mytho::utils::FunctionType Func>
        self_type& after(Func&& func) {
            _afters.push_back(reinterpret_cast<void*>(func));

            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& before(Func&& func) {
            _befores.push_back(reinterpret_cast<void*>(func));

            return *this;
        }

        template<mytho::utils::FunctionType Func>
        self_type& runif(Func&& func) {
            _runif = std::forward<Func>(func);

            return *this;
        }

    public:
        auto& function() noexcept {
            return _function;
        }

        auto& runif() noexcept {
            return _runif;
        }

        auto befores() && noexcept {
            return std::move(_befores);
        }

        auto afters() && noexcept {
            return std::move(_afters);
        }

    private:
        function_type _function;
        runif_type _runif;
        befores_type _befores;
        afters_type _afters;
    };

    template<typename RegistryT>
    class basic_system_stage final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_system_stage<registry_type>;
        using system_type = basic_system<registry_type>;

        using tick_type = uint64_t;
        using function_type = typename system_type::function_type;
        using runif_type = typename system_type::runif_type;
        using befores_type = typename system_type::befores_type;
        using afters_type = typename system_type::afters_type;

        using ticks_type = std::vector<tick_type>;
        using functions_type = std::vector<function_type>;
        using runifs_type = std::vector<runif_type>;
        using befores_pool_type = std::vector<befores_type>;
        using afters_pool_type = std::vector<afters_type>;

        using size_type = typename functions_type::size_type;
        using id_map_type = std::unordered_map<void*, size_type>;

        basic_system_stage() = default;
        basic_system_stage(basic_system_stage& ss) = delete;

        basic_system_stage(basic_system_stage&& ss) noexcept
            : _last_run_ticks(std::move(ss).last_run_ticks()), _functions(std::move(ss).functions()),
            _runifs(std::move(ss).runifs()), _befores_pool(std::move(ss).befores_pool()),
            _afters_pool(std::move(ss).afters_pool()), _id_map(std::move(ss).id_map()) {}

        basic_system_stage& operator=(basic_system_stage&& ss) noexcept {
            _last_run_ticks = std::move(ss).last_run_ticks();
            _functions = std::move(ss).functions();
            _runifs = std::move(ss).runifs();
            _befores_pool = std::move(ss).befores_pool();
            _afters_pool = std::move(ss).afters_pool();
            _id_map = std::move(ss).id_map();

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        void add(Func&& func) {
            if (_id_map.contains(reinterpret_cast<void*>(func))) {
                return;
            }

            _last_run_ticks.push_back(0);
            _functions.emplace_back(std::forward<Func>(func));
            _runifs.emplace_back();
            _befores_pool.emplace_back();
            _afters_pool.emplace_back();

            _id_map[reinterpret_cast<void*>(func)] = _functions.size() - 1;
        }

        void add(system_type& system) {
            if (_id_map.contains(reinterpret_cast<void*>(system.function().pointer()))) {
                return;
            }

            _last_run_ticks.push_back(0);
            _functions.push_back(system.function());
            _runifs.push_back(system.runif());
            _befores_pool.push_back(std::move(system).befores());
            _afters_pool.push_back(std::move(system).afters());

            _id_map[reinterpret_cast<void*>(system.function().pointer())] = _functions.size() - 1;
        }

    public:
        void run(registry_type& reg, uint64_t& tick) {
            mytho::container::basic_sparse_set<size_type, 64> ids;

            auto size = _runifs.size();
            for (auto i = 0; i < size; i++) {
                auto& runif = _runifs[i];
                if (!runif.pointer() || runif(reg, _last_run_ticks[i])) {
                    ids.add(i);
                }
            }

            auto graph = build(ids);
            size = graph.size();

            for (auto i = 0; i < size; ++i) {
                auto& vec = graph[i];
                auto in_size = vec.size();

                for (auto j = 0; j < in_size; ++j) {
                    auto id = vec[j];

                    auto& last_tick = _last_run_ticks[id];
                    _functions[id](reg, last_tick);
                    last_tick = tick;
                }
            }
        }

    public:
        auto last_run_ticks() && noexcept { return std::move(_last_run_ticks); }

        auto functions() && noexcept { return std::move(_functions); }

        auto runifs() && noexcept { return std::move(_runifs); }

        auto befores_pool() && noexcept { return std::move(_befores_pool); }

        auto afters_pool() && noexcept { return std::move(_afters_pool); }

        auto id_map() && noexcept { return std::move(_id_map); }

        auto size() const noexcept { return _functions.size(); }

        void clear() noexcept {
            _last_run_ticks.clear();
            _functions.clear();
            _runifs.clear();
            _befores_pool.clear();
            _afters_pool.clear();
            _id_map.clear();
        }

    private:
        ticks_type _last_run_ticks;
        functions_type _functions;
        runifs_type _runifs;
        befores_pool_type _befores_pool;
        afters_pool_type _afters_pool;

        id_map_type _id_map;

    private:
        auto build(auto& ids) {
            std::vector<std::vector<size_type>> edges(ids.size());
            std::vector<size_t> in_degrees(ids.size(), 0);

            auto size = ids.size();
            for (auto i = 0; i < size; i++) {
                auto id = ids[i];
                auto& befores = _befores_pool[id];
                auto bf_size = befores.size();

                for (auto j = 0; j < bf_size; ++j) {
                    auto p = befores[j];

                    if (!_id_map.contains(p)) {
                        continue;
                    }

                    auto pid = _id_map[p];
                    if (!ids.contain(pid)) {
                        continue;
                    }

                    auto v = ids.index(pid);
                    auto& edge = edges[i];
                    if (std::find(edge.begin(), edge.end(), v) == edge.end()) {
                        edge.push_back(v);
                        in_degrees[v]++;
                    }
                }

                auto& afters = _afters_pool[id];
                auto af_size = afters.size();

                for (auto j = 0; j < af_size; ++j) {
                    auto p = afters[j];

                    if (!_id_map.contains(p)) {
                        continue;
                    }

                    auto pid = _id_map[p];
                    if (!ids.contain(pid)) {
                        continue;
                    }

                    auto k = ids.index(pid);
                    auto& edge = edges[k];
                    if (std::find(edge.begin(), edge.end(), i) == edge.end()) {
                        edge.push_back(i);
                        in_degrees[i]++;
                    }
                }
            }

            return kahn(edges, in_degrees, ids);
        }

        auto kahn(auto& edges, auto& in_degrees, auto& ids) {
            std::vector<size_type> v;

            auto size = in_degrees.size();
            for (auto i = 0; i < size; ++i) {
                if (in_degrees[i] == 0) {
                    v.push_back(i);
                }
            }

            std::vector<std::vector<size_type>> result;
            size_t count = 0;

            while(!v.empty()) {
                count += v.size();
                auto& back = result.emplace_back(); // emplace_back will return ref after c++17

                back.swap(v);
                auto new_size = back.size();
                for (auto i = 0; i < new_size; ++i) {
                    auto& it = back[i];
                    auto id = it;
                    it = ids[id];

                    auto& edge = edges[id];
                    auto in_size = edge.size();
                    for (auto j = 0; j < in_size; ++j) {
                        auto idx = edge[j];
                        if (--in_degrees[idx] == 0) {
                            v.push_back(idx);
                        }
                    }
                }
            }

            ASSURE(count == in_degrees.size(), "Cycle detected in system dependencies.");
            return result;
        }
    };
}