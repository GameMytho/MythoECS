#pragma once

#include "utils/idgen.hpp"
#include "utils/concept.hpp"
#include "utils/assert.hpp"
#include "container/tick_set.hpp"

namespace mytho::container {
    template<
        typename ResourceIdGenerator,
        template<typename> typename AllocatorT = std::allocator
    >
    class basic_resource_storage final {
    public:
        using resource_id_generator = ResourceIdGenerator;
        using resource_id_type = typename resource_id_generator::value_type;
        using resource_pool_type = std::vector<void*>;
        using size_type = typename resource_pool_type::size_type;
        using resource_destroy_functions_type = std::vector<void(*)(void*)>;
        using resource_ticks_type = basic_tick_set;

        basic_resource_storage() noexcept = default;
        basic_resource_storage(const basic_resource_storage& rs) = delete;
        basic_resource_storage(basic_resource_storage&& rs) noexcept = default;

        basic_resource_storage& operator=(const basic_resource_storage& rs) = delete;
        basic_resource_storage& operator=(basic_resource_storage&& rs) noexcept = default;

        ~basic_resource_storage() { clear(); }

    public:
        template<mytho::utils::PureValueType T, typename... Rs>
        void init(uint64_t tick, Rs&&... rs) {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = resource_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                _pool.resize(id + 1, nullptr);
                _ticks.resize(id + 1);
                _destroy_funcs.resize(id + 1, nullptr);
            }

            auto& p = _pool[id];
            if (p) return;

            AllocatorT<T> allocator;
            p = static_cast<void*>(alloc_traits::allocate(allocator, 1));

            // use placement new, construct_at(c++20) need explicit constructor
            // alloc_traits::construct(allocator, static_cast<T*>(_pool[id]), std::forward<Rs>(rs)...);
            new (static_cast<T*>(p)) T{ std::forward<Rs>(rs)... };

            // resource init means resource added and changed, is_added case is considered in is_changed case
            _ticks.set_added_tick(id, tick);
            _ticks.set_changed_tick(id, tick);

            _destroy_funcs[id] = [](void* ptr){
                AllocatorT<T> allocator;

                static_cast<T*>(ptr)->~T();

                //alloc_traits::destroy(allocator, static_cast<T*>(_pool[id]));
                alloc_traits::deallocate(allocator, static_cast<T*>(ptr), 1);
            };
        }

        template<mytho::utils::PureValueType T>
        void deinit() {
            auto id = resource_id_generator::template gen<T>();
            if (id >= _pool.size()) return;

            auto& p = _pool[id];
            if (!p) return;

            _destroy_funcs[id](p);
            p = nullptr;

            // we do not need to reset added/changed tick
        }

    public:
        template<mytho::utils::PureValueType T>
        const T& get() const {
            auto id = resource_id_generator::template gen<T>();

            return *static_cast<T*>(_pool[id]);
        }

        template<mytho::utils::PureValueType T>
        T& get() {
            auto id = resource_id_generator::template gen<T>();

            return *static_cast<T*>(_pool[id]);
        }

        template<mytho::utils::PureValueType T>
        uint64_t& get_changed_tick_ref() {
            auto id = resource_id_generator::template gen<T>();

            return _ticks.get_changed_tick(id);
        }

        template<mytho::utils::PureValueType T>
        bool is_added(uint64_t tick) const {
            auto id = resource_id_generator::template gen<T>();

            return id < _pool.size() && _pool[id] && _ticks.get_added_tick(id) >= tick;
        }

        template<mytho::utils::PureValueType T>
        bool is_changed(uint64_t tick) const {
            auto id = resource_id_generator::template gen<T>();

            return id < _pool.size() && _pool[id] && _ticks.get_changed_tick(id) >= tick;
        }

        template<mytho::utils::PureValueType T>
        bool exist() const {
            auto id = resource_id_generator::template gen<T>();

            return id < _pool.size() && _pool[id];
        }

        void clear() {
            auto size = _pool.size();
            for (size_type i = 0; i < size; ++i) {
                auto p = _pool[i];
                if (p == nullptr) continue;

                _destroy_funcs[i](p);
            }

            _pool.clear();
            _ticks.clear();
            _destroy_funcs.clear();
        }

    public:
        size_type size() const noexcept { return _pool.size(); }

        bool empty() const noexcept { return _pool.empty(); }

    private:
        resource_pool_type _pool;
        resource_ticks_type _ticks;
        resource_destroy_functions_type _destroy_funcs;
    };
}