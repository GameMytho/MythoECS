#pragma once

#include "utils/idgen.hpp"
#include "container/tick_set.hpp"

namespace mytho::container {
    template<
        mytho::utils::UnsignedIntegralType ResourceIdT = size_t,
        template<typename> typename AllocatorT = std::allocator
    >
    class basic_resource_storage final {
    public:
        using resource_id_type = ResourceIdT;
        using resource_pool_type = std::vector<void*>;
        using size_type = typename resource_pool_type::size_type;
        using resource_destroy_functions_type = std::vector<void(*)(void*)>;
        using resource_ticks_type = basic_tick_set;
        using resource_id_generator = mytho::utils::basic_id_generator<mytho::utils::GeneratorType::RESOURCE_GENOR, resource_id_type>;

        ~basic_resource_storage() { clear(); }

    public:
        template<mytho::utils::PureValueType T, typename... Rs>
        void init(uint64_t tick, Rs&&... rs) noexcept {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = resource_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                _pool.resize(id + 1, nullptr);
                _ticks.resize(id + 1);
                _destroy_funcs.resize(id + 1, nullptr);
            }

            if (_pool[id]) return;

            AllocatorT<T> allocator;
            _pool[id] = static_cast<void*>(alloc_traits::allocate(allocator, 1));

            // use placement new, construct_at(c++20) need explicit constructor
            // alloc_traits::construct(allocator, static_cast<T*>(_pool[id]), std::forward<Rs>(rs)...);
            new (static_cast<T*>(_pool[id])) T{ std::forward<Rs>(rs)... };

            _ticks.set_added_tick(id, tick);

            _destroy_funcs[id] = [](void* ptr){
                AllocatorT<T> allocator;

                static_cast<T*>(ptr)->~T();

                //alloc_traits::destroy(allocator, static_cast<T*>(_pool[id]));
                alloc_traits::deallocate(allocator, static_cast<T*>(ptr), 1);
            };
        }

        template<mytho::utils::PureValueType T>
        void deinit() noexcept {
            using alloc_traits = std::allocator_traits<AllocatorT<T>>;

            auto id = resource_id_generator::template gen<T>();
            if (id >= _pool.size() || !_pool[id]) return;

            _destroy_funcs[id](_pool[id]);
            _pool[id] = nullptr;

            // reset changed tick to avoid the case: `init -> deinit -> init`
            // only need to reset changed tick, because added tick will be set when resource init
            _ticks.set_changed_tick(id, 0);
        }

        template<mytho::utils::PureValueType T>
        const T& get() const noexcept {
            ASSURE(contain<T>(), "resource not exist.");

            return *static_cast<T*>(_pool[resource_id_generator::template gen<T>()]);
        }

        template<mytho::utils::PureValueType T>
        T& get() noexcept {
            ASSURE(contain<T>(), "resource not exist.");

            return *static_cast<T*>(_pool[resource_id_generator::template gen<T>()]);
        }

        template<mytho::utils::PureValueType T>
        uint64_t& get_changed_tick_ref() noexcept {
            ASSURE(contain<T>(), "resource not exist.");

            return _ticks.get_changed_tick(resource_id_generator::template gen<T>());
        }

        template<mytho::utils::PureValueType T>
        bool contain() const noexcept {
            return _pool[resource_id_generator::template gen<T>()] != nullptr;
        }

        template<mytho::utils::PureValueType T>
        bool is_added(uint64_t tick) const noexcept {
            auto id = resource_id_generator::template gen<T>();

            return _pool[id] != nullptr && _ticks.get_added_tick(id) >= tick;
        }

        template<mytho::utils::PureValueType T>
        bool is_changed(uint64_t tick) const noexcept {
            auto id = resource_id_generator::template gen<T>();

            return _pool[id] != nullptr && _ticks.get_changed_tick(id) >= tick;
        }

        void clear() noexcept {
            for (size_type i = 0; i < _pool.size(); i++) {
                if (_pool[i] == nullptr) continue;

                _destroy_funcs[i](_pool[i]);
            }

            _pool.clear();
            _ticks.clear();
            _destroy_funcs.clear();
        }

    private:
        resource_pool_type _pool;
        resource_ticks_type _ticks;
        resource_destroy_functions_type _destroy_funcs;
    };
}