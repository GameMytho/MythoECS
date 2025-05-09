#pragma once
#include "container/component_set.hpp"
#include "utils/idgen.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, mytho::utils::UnsignedIntegralType ComponentIdT = size_t, size_t PageSize = 1024>
    class basic_component_storage {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using component_set_base_type = basic_entity_set<entity_type, PageSize>;
        using component_pool_type = std::vector<std::unique_ptr<component_set_base_type>>;
        using size_type = typename component_pool_type::size_type;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;

    public:
        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e, Ts&&... ts) noexcept {
            ASSURE(_not_contain<Ts...>(e), "the entity already has some components.");

            _add(e, std::forward<Ts>(ts)...);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            ASSURE(_contain<Ts...>(e), "the entity is missing some components.");

            _remove<Ts...>(e);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        std::tuple<Ts...> get(const entity_type& e) const noexcept {
            ASSURE(_contain<Ts...>(e), "the entity is missing some components.");

            return _get<Ts...>(e);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            ASSURE(_contain<Ts...>(e), "the entity is missing some components.");

            _replace(e, std::forward<Ts>(ts)...);
        }

        template<typename... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _contain<Ts...>(e);
        }

        void clear() noexcept {
            _pool.clear();
        }

        size_type size() const noexcept { return _pool.size(); }

    private:
        component_pool_type _pool;

    private:
        template<typename T, typename... Rs>
        void _add(const entity_type& e, T&& t, Rs&&... rs) noexcept {
            assure<T>().add(e, t);

            if constexpr (sizeof...(Rs) > 0) {
                _add(e, std::forward<Rs>(rs)...);
            }
        }

        template<typename T, typename... Rs>
        void _remove(const entity_type& e) noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            static_cast<component_set_type&>(*_pool[id]).remove(e);

            if constexpr (sizeof...(Rs) > 0) {
                _remove<Rs...>(e);
            }
        }

        template<typename T, typename... Rs>
        auto _get(const entity_type& e) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return std::tuple_cat(std::make_tuple(static_cast<component_set_type&>(*_pool[id]).get(e)), _get<Rs...>(e));
            } else {
                return std::make_tuple(static_cast<component_set_type&>(*_pool[id]).get(e));
            }
        }

        template<typename T, typename... Rs>
        void _replace(const entity_type& e, T&& t, Rs&&... rs) noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            static_cast<component_set_type&>(*_pool[id]).replace(e, t);

            if constexpr (sizeof...(Rs) > 0) {
                _replace(e, std::forward<Rs>(rs)...);
            }
        }

        template<typename T, typename... Rs>
        bool _contain(const entity_type& e) const noexcept {
            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return id < _pool.size() && _pool[id]->contain(e) && _contain<Rs...>(e);
            } else {
                return id < _pool.size() && _pool[id]->contain(e);
            }
        }

        template<typename T, typename... Rs>
        bool _not_contain(const entity_type& e) const noexcept {
            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return (id >= _pool.size() || !_pool[id]->contain(e)) && _not_contain<Rs...>(e);
            } else {
                return id >= _pool.size() || !_pool[id]->contain(e);
            }
        }

    private:
        template<typename T>
        auto& assure() noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;
            auto id = component_id_generator::template gen<T>();

            if (id >= _pool.size()) {
                _pool.resize(id + 1);
            }

            if (_pool[id] == nullptr) {
                _pool[id] = std::make_unique<component_set_type>();
            }

            return static_cast<component_set_type&>(*_pool[id]);
        }
    };
}