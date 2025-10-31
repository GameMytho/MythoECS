#pragma once

#include <vector>
#include <iterator>

#include "utils/concept.hpp"

namespace mytho::container {
    template<mytho::utils::PureValueType EventT>
    class basic_event_set final {
    private:
        template<typename IteratorT>
        class event_iterator final {
        public:
            using base_iterator_type = IteratorT;
            using iterator_category = std::random_access_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = EventT;
            using self_type = event_iterator<base_iterator_type>;

            event_iterator() = default;
            explicit event_iterator(base_iterator_type it) : _it(it) {}

        public:
            value_type& operator*() const noexcept { return *static_cast<value_type*>(*_it); }
            value_type* operator->() const noexcept { return static_cast<value_type*>(*_it); }

            self_type& operator++() noexcept { ++_it; return *this; }
            self_type operator++(int) noexcept { auto tmp = *this; ++(*this); return tmp; }
            self_type& operator--() noexcept { --_it; return *this; }
            self_type operator--(int) noexcept { auto tmp = *this; --(*this); return tmp; }

            self_type& operator+=(difference_type n) noexcept { _it += n; return *this; }
            self_type& operator-=(difference_type n) noexcept { _it -= n; return *this; }

            friend self_type operator+(self_type it, difference_type n) noexcept { it += n; return it; }
            friend self_type operator+(difference_type n, self_type it) noexcept { it += n; return it; }
            friend self_type operator-(self_type it, difference_type n) noexcept { it -= n; return it; }
            friend difference_type operator-(const self_type& l, const self_type& r) noexcept { return l._it - r._it; }

            value_type& operator[](difference_type n) const noexcept { return *static_cast<value_type*>(*(_it + n)); }

            friend bool operator==(const self_type& l, const self_type& r) noexcept { return l._it == r._it; }
            friend bool operator!=(const self_type& l, const self_type& r) noexcept { return !(l == r); }
            friend bool operator<(const self_type& l, const self_type& r) noexcept { return l._it < r._it; }
            friend bool operator>(const self_type& l, const self_type& r) noexcept { return r < l; }
            friend bool operator<=(const self_type& l, const self_type& r) noexcept { return !(r < l); }
            friend bool operator>=(const self_type& l, const self_type& r) noexcept { return !(l < r); }

        private:
            base_iterator_type _it;
        };

    public:
        using event_type = EventT;
        using events_data_type = std::vector<void*>;
        using iterator = event_iterator<events_data_type::iterator>;
        using const_iterator = event_iterator<events_data_type::const_iterator>;
        using size_type = typename events_data_type::size_type;

        basic_event_set() noexcept : _data(nullptr) {}
        basic_event_set(const basic_event_set& es) noexcept = default;
        basic_event_set(basic_event_set&& es) noexcept = default;

        explicit basic_event_set(events_data_type& data) noexcept : _data(&data) {}
        explicit basic_event_set(const events_data_type& data) noexcept : _data(&const_cast<events_data_type&>(data)) {}

        basic_event_set& operator=(const basic_event_set& es) noexcept = default;
        basic_event_set& operator=(basic_event_set&& es) noexcept = default;

        ~basic_event_set() noexcept = default;

    public:
        iterator begin() noexcept { if(!_data) return iterator{}; return iterator(_data->begin()); }
        iterator end() noexcept { if(!_data) return iterator{}; return iterator(_data->end()); }

        const_iterator begin() const noexcept { if(!_data) return const_iterator{}; return const_iterator(_data->begin()); }
        const_iterator end() const noexcept { if(!_data) return const_iterator{}; return const_iterator(_data->end()); }

        size_type size() const noexcept { if(!_data) return 0; return _data->size(); }

        bool empty() const noexcept { if(!_data) return true; return _data->empty(); }

    private:
        events_data_type* _data;
    };
}