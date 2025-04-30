#pragma once
#include "container/entity_set.hpp"

namespace mytho::container {
    template<mytho::utils::EntityType EntityT, size_t PageSize = 1024>
    class basic_entity_storage : protected basic_entity_set<EntityT, PageSize> {
    public:
        using entity_type = EntityT;
        using base_type = basic_entity_set<EntityT, PageSize>;
        using size_type = typename basic_entity_set<EntityT, PageSize>::size_type;

    public:
        entity_type emplace() noexcept {
            _length++;

            if (_length <= base_type::size()) {
                return base_type::entity(_length - 1);
            } else {
                return base_type::add(entity_type(base_type::size()));
            }
        }

        void remove(entity_type e) noexcept {
            if (!base_type::contain(e)) {
                return;
            }

            base_type::swap(e, base_type::entity(_length - 1));
            base_type::version_next(e);

            _length--;
        }

        bool contain(entity_type e) const noexcept {
            return base_type::contain(e) && base_type::index(e) < _length;
        }

        void clear() noexcept {
            base_type::clear();
            _length = 0;
        }

        size_type size() const noexcept { return _length; }

    private:
        size_type _length = 0;
    };
}