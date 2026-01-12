#pragma once
#include <memory>
#include <utility>
#include <vector>

#include "container/entity_set.hpp"
#include "utils/idgen.hpp"

namespace mytho::container {
    template<
        mytho::utils::EntityType EntityT,
        typename ComponentIdGenerator,
        size_t PageSize = 256
    >
    class basic_entity_storage final : protected basic_entity_set<EntityT, PageSize> {
    public:
        using entity_type = EntityT;
        using component_id_generator = ComponentIdGenerator;
        using base_type = basic_entity_set<entity_type, PageSize>;
        using size_type = typename basic_entity_set<EntityT, PageSize>::size_type;
        using component_id_type = typename component_id_generator::value_type;
        using component_id_set_type = basic_sparse_set<component_id_type, PageSize>;
        using component_id_set_ptr_type = std::unique_ptr<component_id_set_type>;
        using component_id_map_type = std::vector<component_id_set_ptr_type>;

        basic_entity_storage() noexcept = default;
        basic_entity_storage(const basic_entity_storage& es) = delete;
        basic_entity_storage(basic_entity_storage&& es) noexcept = default;

        basic_entity_storage& operator=(const basic_entity_storage& es) = delete;
        basic_entity_storage& operator=(basic_entity_storage&& es) noexcept = default;

        ~basic_entity_storage() noexcept = default;

    public:
        template<mytho::utils::PureValueType... Ts>
        entity_type emplace() {
            ++_length;

            auto size = base_type::size();

            if (_length <= size) {
                if constexpr (sizeof...(Ts) > 0) {
                    (insert_components<Ts>(_map[_length - 1]), ...);
                }

                return base_type::operator[](_length - 1);
            } else {
                _map.push_back(std::make_unique<component_id_set_type>());
                if constexpr (sizeof...(Ts) > 0) {
                    (insert_components<Ts>(_map.back()), ...);
                }

                entity_type e(size);

                if (e.valid()) {
                    base_type::add(e);
                }

                return e;
            }
        }

        // must ensure entity exist
        void pop(const entity_type& e) noexcept {
            auto idx = base_type::index(e);
            base_type::version_next(e);

            if (idx != (_length - 1)) {
                base_type::swap(e, base_type::operator[](_length - 1));
                _map[idx]->clear();
                std::swap(_map[idx], _map[_length - 1]);
            }

            --_length;
        }

        // must ensure entity exist
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e) {
            auto idx = base_type::index(e);
            (insert_components<Ts>(_map[idx]), ...);
        }

        // must ensure entity exist
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            auto idx = base_type::index(e);
            (remove_components<Ts>(_map[idx]), ...);
        }

        // must ensure entity exist
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool has(const entity_type& e) const noexcept {
            auto idx = base_type::index(e);
            return (_has<Ts>(idx) && ...);
        }

        // must ensure entity exist
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool not_has(const entity_type& e) const noexcept {
            auto idx = base_type::index(e);
            return (!_has<Ts>(idx) && ...);
        }

        bool contain(const entity_type& e) const noexcept {
            return base_type::contain(e) && base_type::index(e) < _length;
        }

        void clear() noexcept {
            base_type::clear();
            _length = 0;
        }

    public:
        auto begin() noexcept { return base_type::begin(); }
        auto begin() const noexcept { return base_type::begin(); }

        auto end() noexcept { return base_type::end(); }
        auto end() const noexcept { return base_type::end(); }

        size_type size() const noexcept { return _length; }

        bool empty() const noexcept { return _length == 0; }

        const entity_type operator[](size_type idx) const { return base_type::operator[](idx); }

    private:
        template<typename T>
        void insert_components(const component_id_set_ptr_type& s) {
            auto id = component_id_generator::template gen<T>();
            if (!s->contain(id)) { s->add(id); }
        }

        template<typename T>
        void remove_components(const component_id_set_ptr_type& s) noexcept {
            auto id = component_id_generator::template gen<T>();
            if (s->contain(id)) { s->remove(id); }
        }

        template<typename T>
        bool _has(auto idx) const noexcept {
            auto id = component_id_generator::template gen<T>();
            return _map[idx]->contain(id);
        }

    private:
        component_id_map_type _map;
        size_type _length = 0;
    };
}