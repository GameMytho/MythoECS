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
    private:
        template<typename IdIteratorT, typename VersionIteratorT>
        class entity_iterator final {
        public:
            using id_iterator_type = IdIteratorT;
            using version_iterator_type = VersionIteratorT;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = EntityT;
            using self_type = entity_iterator<id_iterator_type, version_iterator_type>;

            entity_iterator() = default;
            entity_iterator(id_iterator_type id_it, version_iterator_type version_it) : _id_it(id_it), _version_it(version_it) {}

        public:
            value_type operator*() const noexcept { return value_type(*_id_it, *_version_it); }

            self_type& operator++() noexcept { ++_id_it; ++_version_it; return *this; }
            self_type operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }
            self_type& operator--() noexcept { --_id_it; --_version_it; return *this; }
            self_type operator--(int) noexcept { auto tmp = *this; --(*this); return tmp; }

            self_type& operator+=(difference_type n) noexcept { _id_it += n; _version_it += n; return *this; }
            self_type& operator-=(difference_type n) noexcept { _id_it -= n; _version_it -= n; return *this; }

            friend self_type operator+(self_type it, difference_type n) noexcept { it += n; return it; }
            friend self_type operator+(difference_type n, self_type it) noexcept { it += n; return it; }
            friend self_type operator-(self_type it, difference_type n) noexcept { it -= n; return it; }
            friend difference_type operator-(const self_type& l, const self_type& r) noexcept { return l._id_it - r._id_it; }

            value_type operator[](difference_type n) const noexcept { return value_type(*(_id_it + n), *(_version_it + n)); }

            friend bool operator==(const self_type& l, const self_type& r) noexcept { return (l._id_it == r._id_it) && (l._version_it == r._version_it); }
            friend bool operator!=(const self_type& l, const self_type& r) noexcept { return !(l == r); }
            friend bool operator<(const self_type& l, const self_type& r) noexcept { return l._id_it < r._id_it; }
            friend bool operator>(const self_type& l, const self_type& r) noexcept { return r < l; }
            friend bool operator<=(const self_type& l, const self_type& r) noexcept { return !(r < l); }
            friend bool operator>=(const self_type& l, const self_type& r) noexcept { return !(l < r); }

        private:
            id_iterator_type _id_it;
            version_iterator_type _version_it;
        };

    public:
        using entity_type = EntityT;
        using entity_version_type = typename entity_type::version_type;
        using entity_version_container_type = std::vector<entity_version_type>;
        using size_type = typename entity_version_container_type::size_type;
        using base_type = basic_sparse_set<typename entity_type::id_type, PageSize>;
        using iterator = entity_iterator<typename base_type::iterator, typename entity_version_container_type::iterator>;
        using const_iterator = entity_iterator<typename base_type::const_iterator, typename entity_version_container_type::const_iterator>;

        virtual ~basic_entity_set() = default;

    public:
        entity_type add(const entity_type& e) {
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

        entity_type entity(size_type idx) const noexcept {
            ASSURE(idx < _versions.size(), "entity index out of entity set!");

            return entity_type(base_type::data(idx), _versions[idx]);
        }

    public:
        iterator begin() noexcept { return { base_type::begin(), _versions.begin()}; }
        const_iterator begin() const noexcept { return { base_type::begin(), _versions.begin()}; }

        iterator end() noexcept { return { base_type::end(), _versions.end()}; }
        const_iterator end() const noexcept { return { base_type::end(), _versions.end()}; }

        size_type size() const noexcept { return _versions.size(); }

        bool empty() const noexcept { return _versions.empty(); }

    protected:
        void version_next(const entity_type& e) noexcept {
            ASSURE(contain(e), "invalid entity value(entity not exist).");

            _versions[base_type::index(e.id())]++;
        }

    private:
        entity_version_container_type _versions;
    };
}