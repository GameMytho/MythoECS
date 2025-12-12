#pragma once

#include <vector>

#include "utils/idgen.hpp"

#include "container/event_set.hpp"

namespace mytho::container {
    template<
        typename EventIdGenerator,
        template<typename> typename AllocatorT = std::allocator
    >
    class basic_event_storage final {
    public:
        using event_id_generator = EventIdGenerator;
        using event_id_type = typename event_id_generator::value_type;
        using self_type = basic_event_storage<event_id_generator, AllocatorT>;
        using event_pool_type = std::vector<std::vector<void*>>;
        using size_type = typename event_pool_type::size_type;
        using event_destroy_funcs_type = std::vector<void(*)(void*)>;

        basic_event_storage() noexcept = default;
        basic_event_storage(const basic_event_storage& es) = delete;
        basic_event_storage(basic_event_storage&& es) noexcept = default;

        basic_event_storage& operator=(const basic_event_storage& es) = delete;
        basic_event_storage& operator=(basic_event_storage&& es) noexcept = default;

        ~basic_event_storage() { clear(); }

    public:
        template<mytho::utils::PureValueType T>
        void init() {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = event_id_generator::template gen<T>();
            if (id >= _pool.size()) {
                _pool.resize(id + 1);
                _destroy_funcs.resize(id + 1, nullptr);
            }

            auto& f = _destroy_funcs[id];
            if (f) return;

            f = [] (void* ptr) {
                AllocatorT<T> allocator;

                static_cast<T*>(ptr)->~T();
                alloc_traits::deallocate(allocator, static_cast<T*>(ptr), 1);
            };
        }

        template<mytho::utils::PureValueType T>
        void deinit() {
            auto id = event_id_generator::template gen<T>();

            if (id >= _pool.size()) return;

            auto& f = _destroy_funcs[id];

            if (!f) return;

            for (auto e : _pool[id]) {
                f(e);
            }

            f = nullptr;
        }

    public:
        template<mytho::utils::PureValueType T, typename... Rs>
        void write(Rs&&... rs) {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = event_id_generator::template gen<T>();
            auto& p = _pool[id];

            AllocatorT<T> allocator;
            p.push_back(static_cast<void*>(alloc_traits::allocate(allocator, 1)));
            new (static_cast<T*>(p.back())) T{ std::forward<Rs>(rs)... };
        }

        template<mytho::utils::PureValueType T>
        mytho::container::basic_event_set<T> mutate() {
            auto id = event_id_generator::template gen<T>();

            return mytho::container::basic_event_set<T>{ _pool[id] };
        }

        template<mytho::utils::PureValueType T>
        const mytho::container::basic_event_set<T> read() const {
            auto id = event_id_generator::template gen<T>();

            return mytho::container::basic_event_set<T>{ _pool[id] };
        }

        void swap(self_type& other) noexcept {
            _pool.swap(other.pool());
            _destroy_funcs.swap(other.destroy_funcs());
        }

        template<mytho::utils::PureValueType T>
        bool exist() const {
            auto id = event_id_generator::template gen<T>();

            return id < _pool.size() && _destroy_funcs[id];
        }

        void clear() {
            auto size = _destroy_funcs.size();
            for (size_type i = 0; i < size; i++) {
                auto& f = _destroy_funcs[i];
                auto& p = _pool[i];
                if (f) {
                    for (auto e : p) {
                        f(e);
                    }

                    p.clear();
                }
            }
        }

    public:
        event_pool_type& pool() noexcept { return _pool; }

        event_destroy_funcs_type& destroy_funcs() noexcept { return _destroy_funcs; }

        size_type size() const noexcept { return _pool.size(); }

        bool empty() const noexcept { return _pool.empty(); }

    private:
        event_pool_type _pool;
        event_destroy_funcs_type _destroy_funcs;
    };
}