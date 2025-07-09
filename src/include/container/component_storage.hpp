#pragma once
#include <vector>
#include <tuple>
#include <memory>
#include <utility>
#include <cstdint>

#include "container/component_set.hpp"
#include "utils/idgen.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, mytho::utils::UnsignedIntegralType ComponentIdT = size_t, size_t PageSize = 1024>
    class basic_component_storage final {
    public:
        using entity_type = EntityT;
        using component_id_type = ComponentIdT;
        using component_set_base_type = basic_entity_set<entity_type, PageSize>;
        using component_pool_type = std::vector<std::unique_ptr<component_set_base_type>>;
        using size_type = typename component_pool_type::size_type;
        using component_id_generator = mytho::utils::basic_id_generator<component_id_type>;

    public:
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void add(const entity_type& e, uint64_t tick, Ts&&... ts) noexcept {
            ASSURE(_not_contain<Ts...>(e), "the entity already has some components.");

            _add(e, tick, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureValueType... Ts>
        void remove(const entity_type& e) noexcept {
            if constexpr (sizeof...(Ts) > 0) {
                ASSURE(_contain<Ts...>(e), "the entity is missing some components.");
                _remove_components<Ts...>(e);
            } else {
                _remove_entity(e);
            }
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        std::tuple<const Ts&...> get(const entity_type& e) const noexcept {
            ASSURE(_contain<Ts...>(e), "the entity is missing some components.");

            return _get<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, uint64_t tick, Ts&&... ts) noexcept {
            ASSURE(_contain<Ts...>(e), "the entity is missing some components.");

            _replace(e, tick, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _contain<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool not_contain(const entity_type& e) const noexcept {
            return _not_contain<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool is_added(const entity_type& e, uint64_t tick) const noexcept {
            return _is_added<Ts...>(e, tick);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        bool is_changed(const entity_type& e, uint64_t tick) const noexcept {
            return _is_changed<Ts...>(e, tick);
        }

        void clear() noexcept {
            _pool.clear();
        }

        size_type size() const noexcept { return _pool.size(); }

    public:
        component_set_base_type* operator[](size_type index) noexcept {
            return _pool[index].get();
        }

    private:
        component_pool_type _pool;

    private:
        template<typename T, typename... Rs>
        void _add(const entity_type& e, uint64_t tick, T&& t, Rs&&... rs) noexcept {
            assure<T>().add(e, tick, t);

            if constexpr (sizeof...(Rs) > 0) {
                _add(e, tick, std::forward<Rs>(rs)...);
            }
        }

        template<typename T, typename... Rs>
        void _remove_components(const entity_type& e) noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            static_cast<component_set_type&>(*_pool[id]).remove(e);

            if constexpr (sizeof...(Rs) > 0) {
                _remove_components<Rs...>(e);
            }
        }

        void _remove_entity(const entity_type& e) noexcept {
            for (auto& s : _pool) {
                if (s->contain(e)) s->remove(e);
            }
        }

        template<typename T, typename... Rs>
        std::tuple<const T&, const Rs&...> _get(const entity_type& e) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return std::tuple_cat(std::tie(static_cast<component_set_type&>(*_pool[id]).get(e)), _get<Rs...>(e));
            } else {
                return std::tie(static_cast<component_set_type&>(*_pool[id]).get(e));
            }
        }

        template<typename T, typename... Rs>
        void _replace(const entity_type& e, uint64_t tick, T&& t, Rs&&... rs) noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            static_cast<component_set_type&>(*_pool[id]).replace(e, tick, t);

            if constexpr (sizeof...(Rs) > 0) {
                _replace(e, tick, std::forward<Rs>(rs)...);
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

        template<typename T, typename... Rs>
        bool _is_added(const entity_type& e, uint64_t tick) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return id < _pool.size() && static_cast<component_set_type&>(*_pool[id]).is_added(e, tick) && _is_added<Rs...>(e, tick);
            } else {
                return id < _pool.size() && static_cast<component_set_type&>(*_pool[id]).is_added(e, tick);
            }
        }

        template<typename T, typename... Rs>
        bool _is_changed(const entity_type& e, uint64_t tick) const noexcept {
            using component_set_type = basic_component_set<entity_type, T, std::allocator<T>, PageSize>;

            auto id = component_id_generator::template gen<T>();

            if constexpr (sizeof...(Rs) > 0) {
                return id < _pool.size() && static_cast<component_set_type&>(*_pool[id]).is_changed(e, tick) && _is_changed<Rs...>(e, tick);
            } else {
                return id < _pool.size() && static_cast<component_set_type&>(*_pool[id]).is_changed(e, tick);
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