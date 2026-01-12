#pragma once
#include <vector>
#include <utility>
#include <cstddef>
#include <iterator>

#include "ecs/entity.hpp"
#include "container/sparse_set.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, size_t PageSize = 256>
    class basic_entity_set : public basic_sparse_set<typename EntityT::id_type, PageSize> {
    public:
        using entity_type = EntityT;
        using entity_version_type = typename entity_type::version_type;
        using entity_version_container_type = std::vector<entity_version_type>;
        using size_type = typename entity_version_container_type::size_type;
        using base_type = basic_sparse_set<typename entity_type::id_type, PageSize>;

        basic_entity_set() noexcept = default;
        basic_entity_set(const basic_entity_set& es) = delete;
        basic_entity_set(basic_entity_set&& es) noexcept = default;

        basic_entity_set& operator=(const basic_entity_set& es) = delete;
        basic_entity_set& operator=(basic_entity_set&& es) noexcept = default;

        virtual ~basic_entity_set() noexcept = default;

    public:
        // must ensure entity not exist
        auto add(const entity_type& e) {
            _versions.push_back(e.version());
            return base_type::add(e.id());
        }

        // must ensure entity exist
        void remove(const entity_type& e) noexcept {
            auto eid = e.id();
            auto idx = base_type::index(eid);

            base_type::remove(eid);

            if (idx != _versions.size() - 1) {
                _versions[idx] = _versions.back();
            }
            _versions.pop_back();
        }

        // must ensure entity exist
        std::pair<size_type, size_type> swap(const entity_type& src, const entity_type& dst) noexcept {
            auto sid = src.id();
            auto did = dst.id();

            auto ids = base_type::swap(sid, did);
            std::swap(_versions[ids.first], _versions[ids.second]);

            return ids;
        }

        // must ensure entity exist
        size_type index(const entity_type& e) const noexcept {
            return base_type::index(e.id());
        }

        bool contain(const entity_type& e) const noexcept {
            if (!base_type::contain(e.id())) {
                return false;
            }

            auto idx = base_type::index(e.id());

            return idx < _versions.size() && _versions[idx] == e.version();
        }

        void clear() noexcept {
            base_type::clear();
            _versions.clear();
        }

    public:
        size_type size() const noexcept { return _versions.size(); }

        bool empty() const noexcept { return _versions.empty(); }

        const entity_type operator[](size_type idx) const { return entity_type(base_type::operator[](idx), _versions[idx]); }

    protected:
        void version_next(const entity_type& e) noexcept {
            ++_versions[base_type::index(e.id())];
        }

    private:
        entity_version_container_type _versions;
    };
}