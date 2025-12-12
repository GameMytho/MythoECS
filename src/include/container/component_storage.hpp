#pragma once
#include <vector>
#include <tuple>
#include <memory>
#include <utility>
#include <cstdint>

#include "container/component_set.hpp"
#include "utils/idgen.hpp"

namespace mytho::container {
    template<
        mytho::utils::EntityType EntityT,
        typename ComponentIdGenerator,
        size_t PageSize = 1024
    >
    class basic_component_storage final {
    public:
        using entity_type = EntityT;
        using component_id_generator = ComponentIdGenerator;
        using component_id_type = typename component_id_generator::value_type;
        using component_set_base_type = basic_entity_set<entity_type, PageSize>;
        using component_pool_type = std::vector<std::unique_ptr<component_set_base_type>>;
        using size_type = typename component_pool_type::size_type;
        using entity_remove_functions_type = std::vector<void(*)(void*, const entity_type&)>;
        using removed_entities_type = std::vector<std::vector<entity_type>>;

        basic_component_storage() noexcept = default;
        basic_component_storage(const basic_component_storage& cs) = delete;
        basic_component_storage(basic_component_storage&& cs) noexcept = default;

        basic_component_storage& operator=(const basic_component_storage& cs) = delete;
        basic_component_storage& operator=(basic_component_storage&& cs) noexcept = default;

        ~basic_component_storage() = default;

    public:
        // must ensure the entity does not have any specific components
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e, uint64_t tick, Ts&&... ts) {
            (assure<Ts>().add(e, tick, std::forward<Ts>(ts)), ...);
        }

        // must ensure the entity has all specific components
        template<mytho::utils::PureValueType... Ts>
        void remove(const entity_type& e) {
            if constexpr (sizeof...(Ts) > 0) {
                (_remove_components<Ts>(e), ...);
            } else {
                _remove_entity(e);
            }
        }

        // must ensure the entity has all specific components
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        std::tuple<const Ts&...> get(const entity_type& e) const noexcept {
            return std::tuple<const Ts&...>(
                static_cast<basic_component_set<entity_type, Ts, std::allocator<Ts>, PageSize>&>(
                    *_pool[component_id_generator::template gen<Ts>()]
                ).get(e)...
            );
        }

        // must ensure the entity has all specific components
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, uint64_t tick, Ts&&... ts) {
            (_replace(e, tick, std::forward<Ts>(ts)), ...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool is_added(const entity_type& e, uint64_t tick) const noexcept {
            return (_is_added<Ts>(e, tick) && ...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool is_changed(const entity_type& e, uint64_t tick) const noexcept {
            return (_is_changed<Ts>(e, tick) && ...);
        }

        template<mytho::utils::PureValueType T>
        auto& removed_entities() noexcept {
            auto id = component_id_generator::template gen<T>();

            if (id >= _entities.size()) {
                _entities.resize(id + 1, std::vector<entity_type>());
            }

            return _entities[id];
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return (_contain<Ts>(e) && ...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool not_contain(const entity_type& e) const noexcept {
            return (!_contain<Ts>(e) && ...);
        }

        void clear() {
            _pool.clear();
            _remove_funcs.clear();
            _entities.clear();
        }

        void removed_entities_clear() noexcept {
            // we do not need to clear whole container, just clear the sub vector to avoid repetitive memory allocation
            for (auto& entity : _entities) {
                entity.clear();
            }
        }

    public:
        size_type size() const noexcept { return _pool.size(); }

        bool empty() const noexcept { return _pool.empty(); }

        component_set_base_type* operator[](size_type index) noexcept { return _pool[index].get(); }

    private:
        component_pool_type _pool;
        entity_remove_functions_type _remove_funcs;
        removed_entities_type _entities;

    private:
        template<typename T>
        void _remove_components(const entity_type& e) {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();
            static_cast<component_set_type&>(*_pool[id]).remove(e);

            // record the removed entity
            if (id >= _entities.size()) {
                _entities.resize(id + 1, std::vector<entity_type>());
            }
            _entities[id].push_back(e);
        }

        void _remove_entity(const entity_type& e) {
            for (size_type i = 0; i < _pool.size(); ++i) {
                auto& p = _pool[i];
                if (p && p->contain(e)) {
                    _remove_funcs[i](p.get(), e);

                    // record the removed entity
                    if (i >= _entities.size()) {
                        _entities.resize(i + 1, std::vector<entity_type>());
                    }
                    _entities[i].push_back(e);
                }
            }
        }

        template<typename T>
        void _replace(const entity_type& e, uint64_t tick, T&& t) {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();
            static_cast<component_set_type&>(*_pool[id]).replace(e, tick, t);
        }

        template<typename T>
        bool _is_added(const entity_type& e, uint64_t tick) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                return false;
            }

            auto& p = static_cast<component_set_type&>(*_pool[id]);

            return p.contain(e) && p.is_added(e, tick);
        }

        template<typename T>
        bool _is_changed(const entity_type& e, uint64_t tick) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                return false;
            }

            auto& p = static_cast<component_set_type&>(*_pool[id]);

            return p.contain(e) && p.is_changed(e, tick);
        }

        template<typename T>
        bool _contain(const entity_type& e) const noexcept {
            auto id = component_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                return false;
            }

            auto& p = _pool[id];

            return p &&  p->contain(e);
        }

    private:
        template<typename T>
        auto& assure() {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;
            auto id = component_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                _pool.resize(id + 1);
                _remove_funcs.resize(id + 1, nullptr);

                _pool[id] = std::make_unique<component_set_type>();
                _remove_funcs[id] = [](void* ptr, const entity_type& e){
                    static_cast<component_set_type*>(ptr)->remove(e);
                };
            }

            return static_cast<component_set_type&>(*_pool[id]);
        }
    };
}