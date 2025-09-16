#pragma once

#include <vector>

#include "utils/idgen.hpp"

#include "container/event_set.hpp"

namespace mytho::container {
    template<
        mytho::utils::UnsignedIntegralType EventIdT = size_t,
        template<typename> typename AllocatorT = std::allocator
    >
    class basic_event_storage final {
    public:
        using event_id_type = EventIdT;
        using self_type = basic_event_storage<event_id_type, AllocatorT>;
        using event_pool_type = std::vector<std::vector<void*>>;
        using size_type = typename event_pool_type::size_type;
        using event_destroy_funcs_type = std::vector<void(*)(void*)>;
        using event_id_generator = mytho::utils::basic_id_generator<mytho::utils::GeneratorType::EVENT_GENOR, event_id_type>;

        ~basic_event_storage() {
            clear();
        }

        template<mytho::utils::PureValueType T, typename... Rs>
        void write(Rs&&... rs) noexcept {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = event_id_generator::template gen<T>();
            if (id >= _pool.size()) {
                _pool.resize(id + 1);
                _destroy_funcs.resize(id + 1, nullptr);
            }

            AllocatorT<T> allocator;
            _pool[id].push_back(static_cast<void*>(alloc_traits::allocate(allocator, 1)));
            new (static_cast<T*>(_pool[id].back())) T{ std::forward<Rs>(rs)... };

            if (_destroy_funcs[id]) return;

            _destroy_funcs[id] = [] (void* ptr) {
                AllocatorT<T> allocator;

                static_cast<T*>(ptr)->~T();
                alloc_traits::deallocate(allocator, static_cast<T*>(ptr), 1);
            };
        }

        template<mytho::utils::PureValueType T>
        mytho::container::basic_event_set<T> mutate() noexcept {
            auto id = event_id_generator::template gen<T>();

            if (id >= _pool.size()) return {};

            return mytho::container::basic_event_set<T>{ _pool[id] };
        }

        template<mytho::utils::PureValueType T>
        const mytho::container::basic_event_set<T> read() const noexcept {
            auto id = event_id_generator::template gen<T>();

            if (id >= _pool.size()) return {};

            return mytho::container::basic_event_set<T>{ _pool[id] };
        }

        void swap(self_type& other) noexcept {
            _pool.swap(other.pool());
            _destroy_funcs.swap(other.destroy_funcs());
        }

        void clear() {
            for (size_type i = 0; i < _pool.size(); i++) {
                for (auto e : _pool[i]) {
                    _destroy_funcs[i](e);
                }
            }

            _pool.clear();
            _destroy_funcs.clear();
        }

    public:
        event_pool_type& pool() noexcept { return _pool; }

        event_destroy_funcs_type& destroy_funcs() noexcept { return _destroy_funcs; }

    private:
        event_pool_type _pool;
        event_destroy_funcs_type _destroy_funcs;
    };
}