#pragma once

#include <vector>

#include "ecs/entity.hpp"
#include "container/sparse_set.hpp"

namespace mytho::container {

    template<mytho::utils::EntityType EntityT, size_t PageSize = 1024>
    class entity_set : public basic_sparse_set<typename EntityT::id_type, PageSize> {
    public:
        using entity_type = EntityT;
        using entity_version_type = typename entity_type::version_type;
        using entity_version_container_type = std::vector<entity_version_type>;
        using size_type = typename entity_version_container_type::size_type;
        using base_type = basic_sparse_set<typename entity_type::id_type, PageSize>;

    public:
        void add(entity_type e) noexcept {
            if (contain(e)) {
                return;
            }

            base_type::add(e.id());
            _versions.push_back(e.version());
        }

        void remove(entity_type e) noexcept {
            if (!contain(e)) {
                return;
            }

            if (base_type::index(e.id()) != _versions.size() - 1) {
                _versions[base_type::index(e.id())] = _versions.back();
            }
            _versions.pop_back();

            base_type::remove(e.id());
        }

        bool contain(entity_type e) const noexcept {
            return base_type::contain(e.id()) && base_type::index(e.id()) < _versions.size() && _versions[base_type::index(e.id())] == e.version();
        }

        void clear() noexcept {
            base_type::clear();
            _versions.clear();
        }

        size_type size() const noexcept { return _versions.size(); }

    private:
        entity_version_container_type _versions;
    };
}