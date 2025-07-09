#pragma once
#include <vector>
#include <utility>
#include <cstddef>
#include <iterator>

#include "ecs/entity.hpp"
#include "container/sparse_set.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, size_t PageSize = 1024>
    class basic_entity_set : public basic_sparse_set<typename EntityT::id_type, PageSize> {
    public:
        using entity_type = EntityT;
        using entity_version_type = typename entity_type::version_type;
        using entity_version_container_type = std::vector<entity_version_type>;
        using size_type = typename entity_version_container_type::size_type;
        using base_type = basic_sparse_set<typename entity_type::id_type, PageSize>;

    public:
        virtual ~basic_entity_set() = default;

        entity_type add(const entity_type& e) noexcept {
            ASSURE(!contain(e), "invalid entity value(entity exist).");

            base_type::add(e.id());
            _versions.push_back(e.version());

            return e;
        }

        void remove(const entity_type& e) noexcept {
            ASSURE(contain(e), "invalid entity value(entity not exist).");

            if (base_type::index(e.id()) != _versions.size() - 1) {
                _versions[base_type::index(e.id())] = _versions.back();
            }
            _versions.pop_back();

            base_type::remove(e.id());
        }

        void swap(const entity_type& src, const entity_type& dst) noexcept {
            ASSURE(contain(src) && contain(dst), "invalid entity value(entities not exist).");

            if (src.id() != dst.id()) {
                base_type::swap(src.id(), dst.id());
                std::swap(_versions[base_type::index(src.id())], _versions[base_type::index(dst.id())]);
            }
        }

        bool contain(const entity_type& e) const noexcept {
            return base_type::contain(e.id()) && base_type::index(e.id()) < _versions.size() && _versions[base_type::index(e.id())] == e.version();
        }

        size_type index(const entity_type& e) const noexcept {
            ASSURE(contain(e), "invalid entity value(entity not exist).");

            return base_type::index(e.id());
        }

        void clear() noexcept {
            base_type::clear();
            _versions.clear();
        }

        size_type size() const noexcept { return _versions.size(); }

        entity_type entity(size_type idx) const noexcept {
            ASSURE(idx < _versions.size(), "entity index out of entity set!");

            return entity_type(base_type::data(idx), _versions[idx]);
        }

    public:
        class iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = entity_type;
            using pointer = value_type*;
            using reference = value_type&;

            iterator(typename base_type::iterator id_it, typename entity_version_container_type::iterator version_it)
                : _id_it(id_it), _version_it(version_it) {}

            entity_type operator*() const {
                return { *_id_it, *_version_it };
            }

            iterator operator++() {
                ++_id_it;
                ++_version_it;
                return *this;
            }

            bool operator!=(const iterator& other) const {
                return _id_it != other._id_it || _version_it != other._version_it;
            }

        private:
            typename base_type::iterator _id_it;
            typename entity_version_container_type::iterator _version_it;
        };

    public:
        iterator begin() noexcept { return { base_type::begin(), _versions.begin()}; }

        iterator end() noexcept { return { base_type::end(), _versions.end()}; }

    protected:
        void version_next(const entity_type& e) noexcept {
            ASSURE(contain(e), "invalid entity value(entity not exist).");

            _versions[base_type::index(e.id())]++;
        }

    private:
        entity_version_container_type _versions;
    };
}