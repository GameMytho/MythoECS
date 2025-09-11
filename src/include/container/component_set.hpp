#pragma once
#include <memory>
#include <cstdint>
#include <utility>
#include <vector>

#include "container/entity_set.hpp"
#include "container/tick_set.hpp"

namespace mytho::container {
    template<
        mytho::utils::EntityType EntityT,
        typename ComponentT,
        typename AllocatorT,
        size_t PageSize = 1024
    >
    class basic_component_set : public basic_entity_set<EntityT, PageSize> {
    public:
        using entity_type = EntityT;
        using base_type = basic_entity_set<entity_type, PageSize>;
        using size_type = typename base_type::size_type;
        using component_type = ComponentT;
        using allocator_type = AllocatorT;
        using alloc_traits = std::allocator_traits<allocator_type>;
        using component_data_ptr_type = typename alloc_traits::pointer;
        using component_data_ptr_set_type = std::vector<component_data_ptr_type, typename alloc_traits::template rebind_alloc<component_data_ptr_type>>;
        using component_tick_set_type = basic_tick_set;

    public:
        ~basic_component_set() {
            clear();
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e, uint64_t tick, Ts&&... ts) noexcept {
            ASSURE(!base_type::contain(e), "entity already exist.");

            base_type::add(e);

            auto idx = base_type::index(e);
            auto data_size = _cdata.size();
            if (idx >= data_size) {
                _cdata.resize(idx + 1);
                _ticks.resize(idx + 1);

                for (int i = data_size; i < _cdata.size(); i++) {
                    allocator_type allocator{_cdata.get_allocator()};
                    _cdata[i] = alloc_traits::allocate(allocator, 1);
                }
            }

            // use placement new, construct_at(c++20) need explicit constructor
            // alloc_traits::construct(allocator, _cdata[idx], std::forward<Ts>(ts)...);
            new (_cdata[idx]) component_type{ std::forward<Ts>(ts)... };

            _ticks.set_added_tick(idx, tick);
            _ticks.set_changed_tick(idx, 0);
        }

        void remove(const entity_type& e) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            auto idx = base_type::index(e);

            // allocator_type allocator{_cdata.get_allocator()};
            // alloc_traits::destroy(allocator, _cdata[idx]);
            _cdata[idx]->~component_type();

            if (idx != base_type::size() - 1) {
                std::swap(_cdata[idx], _cdata[base_type::size() - 1]);
                _ticks.set_added_tick(idx, _ticks.get_added_tick(base_type::size() - 1));
                _ticks.set_changed_tick(idx, _ticks.get_changed_tick(base_type::size() - 1));
            }

            base_type::remove(e);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, uint64_t tick, Ts&&... ts) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            auto idx = base_type::index(e);
            _cdata[idx]->~component_type();
            new (_cdata[idx]) component_type{ std::forward<Ts>(ts)... };

            _ticks.set_changed_tick(idx, tick);
        }

        void clear() noexcept {
            for (unsigned int i = 0; i < base_type::size(); i++) {
                _cdata[i]->~component_type();
            }

            for (unsigned int i = 0; i < _cdata.size(); i++) {
                allocator_type allocator{_cdata.get_allocator()};
                alloc_traits::deallocate(allocator, _cdata[i], 1);
            }

            _cdata.clear();
            _ticks.clear();
            base_type::clear();
        }

        component_type& get(const entity_type& e) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            return *_cdata[base_type::index(e)];
        }

        const component_type& get(const entity_type& e) const noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            return *_cdata[base_type::index(e)];
        }

        uint64_t& changed_tick(const entity_type& e) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            return _ticks.get_changed_tick(base_type::index(e));
        }

        bool is_added(const entity_type& e, uint64_t tick) const noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            /*
             * if the component added system is same as the caller of this,
             * component added tick will be equal to the last run tick of the system,
             * so we use `>=` not `>`.
             */
            return _ticks.get_added_tick(base_type::index(e)) >= tick;
        }

        bool is_changed(const entity_type& e, uint64_t tick) const noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            /*
             * if the component changed system is same as the caller of this,
             * component changed tick will be equal to the last run tick of the system,
             * so we use `>=` not `>`, and then `is_changed` means component added or changed.
             */
            return _ticks.get_changed_tick(base_type::index(e)) >= tick;
        }

        bool contain(const entity_type& e) const noexcept {
            return base_type::contain(e);
        }

        size_type size() const noexcept { return base_type::size(); }

    private:
        component_data_ptr_set_type _cdata;
        component_tick_set_type _ticks;
    };
}