#pragma once
#include <memory>

#include "container/entity_set.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, typename ComponentT, typename AllocatorT, size_t PageSize = 1024>
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

    public:
        ~basic_component_set() {
            clear();
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e, Ts&&... ts) noexcept {
            ASSURE(!base_type::contain(e), "entity already exist.");

            base_type::add(e);
            auto idx = base_type::index(e);
            new (assure(idx)) component_type{std::forward<Ts>(ts)...};
        }

        void remove(const entity_type& e) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            auto idx = base_type::index(e);
            _cdata[idx]->~component_type();
            std::swap(_cdata[idx], _cdata[base_type::size() - 1]);
            base_type::remove(e);
        }

        component_type get(const entity_type& e) const noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            auto idx = base_type::index(e);
            return *_cdata[idx];
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            ASSURE(base_type::contain(e), "entity not exist.");

            auto idx = base_type::index(e);
            _cdata[idx]->~component_type();
            new (_cdata[idx]) component_type{ std::forward<Ts>(ts)... };
        }

        bool contain(const entity_type& e) noexcept {
            return base_type::contain(e);
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
            base_type::clear();
        }

        size_type size() const noexcept { return base_type::size(); }

    private:
        component_data_ptr_set_type _cdata;

    private:
        component_data_ptr_type assure(size_t index) noexcept {
            auto data_size = _cdata.size();
            if (index >= data_size) {
                _cdata.resize(index + 1);
            }

            for (int i = data_size; i < _cdata.size(); i++) {
                allocator_type allocator{_cdata.get_allocator()};
                _cdata[i] = alloc_traits::allocate(allocator, 1);
            }

            return _cdata[index];
        }
    };
}