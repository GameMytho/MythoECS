#pragma once
#include <memory>

#include "container/entity_set.hpp"
#include "utils/idgen.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, mytho::utils::UnsignedIntegralType ComponentIdT = size_t, size_t PageSize = 1024>
    class basic_entity_storage final : protected basic_entity_set<EntityT, PageSize> {
    public:
        using entity_type = EntityT;
        using base_type = basic_entity_set<entity_type, PageSize>;
        using size_type = typename basic_entity_set<EntityT, PageSize>::size_type;
        using component_id_type = ComponentIdT;
        using component_id_set_type = basic_sparse_set<component_id_type, PageSize>;
        using component_id_set_ptr_type = std::unique_ptr<component_id_set_type>;
        using component_id_map_type = std::vector<component_id_set_ptr_type>;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;

    public:
        template<typename... Ts>
        entity_type emplace() noexcept {
            _length++;

            if (_length <= base_type::size()) {
                if constexpr (sizeof...(Ts) > 0) {
                    insert_components<Ts...>(_map[_length - 1]);
                }

                return base_type::entity(_length - 1);
            } else {
                _map.push_back(std::make_unique<component_id_set_type>());
                if constexpr (sizeof...(Ts) > 0) {
                    insert_components<Ts...>(_map.back());
                }

                return base_type::add(entity_type(base_type::size()));
            }
        }

        void pop(const entity_type& e) noexcept {
            if (base_type::index(e) != (_length - 1)) {
                _map[base_type::index(e)]->clear();
                std::swap(_map[base_type::index(e)], _map[_length - 1]);
                base_type::swap(e, base_type::entity(_length - 1));
            }

            base_type::version_next(e);

            _length--;
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e) noexcept {
            insert_components<Ts...>(_map[base_type::index(e)]);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            remove_components<Ts...>(_map[base_type::index(e)]);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        bool has(const entity_type& e) const noexcept {
            return contain(e) && _has<Ts...>(e);
        }

        bool contain(const entity_type& e) const noexcept {
            return base_type::contain(e) && base_type::index(e) < _length;
        }

        void clear() noexcept {
            base_type::clear();
            _length = 0;
        }

        size_type size() const noexcept { return _length; }

    private:
        template<typename T, typename... Rs>
        void insert_components(const component_id_set_ptr_type& s) {
            auto id = component_id_generator::template gen<T>();

            if (!s->contain(id)) {
                s->add(id);
            }

            if constexpr (sizeof...(Rs) > 0) {
                insert_components<Rs...>(s);
            }
        }

        template<typename T, typename... Rs>
        void remove_components(const component_id_set_ptr_type& s) noexcept {
            auto id = component_id_generator::template gen<T>();

            if (s->contain(id)) {
                s->remove(id);
            }

            if constexpr (sizeof...(Rs) > 0) {
                remove_components<Rs...>(s);
            }
        }

        template<typename T, typename... Rs>
        bool _has(const entity_type& e) const noexcept {
            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return _map[base_type::index(e)]->contain(id) && _has<Rs...>(e);
            } else {
                return _map[base_type::index(e)]->contain(id);
            }
        }

    private:
        component_id_map_type _map;
        size_type _length = 0;
    };
}