#pragma once
#include <algorithm>
#include <vector>
#include <limits>

namespace mytho::container {
    template<typename DataT, typename IndexT>
    class indexed_list {
    public:
        using data_type = DataT;
        using index_type = IndexT;
        using data_list_type = std::vector<data_type>;
        using size_type = typename data_list_type::size_type;
        using index_list_type = std::vector<index_type>;

        inline static index_type index_null = std::numeric_limits<index_type>::max();

    public:
        template<typename... Ts>
        void emplace_front(Ts&&... ts) noexcept {
            index_type index = _data_list.size();
            _data_list.emplace_back(std::forward<Ts>(ts)...);
            _prev_list.emplace_back(index_null);
            _next_list.emplace_back(_head);

            if (_head != index_null) {
                _prev_list[_head] = index;
            }

            _head = index;

            if (_tail == index_null) {
                _tail = index;
            }
        }

        void pop_front() noexcept {
            if (_head == index_null) {
                return;
            }

            if (_head != _tail) {
                if (_head != _next_list.size() - 1) {
                    if (_tail == _next_list.size() - 1) {
                        _tail = _head;
                    }

                    swap(_head, _data_list.size() - 1);

                    _head = _next_list.size() - 1;
                }
            } else {
                _tail = index_null;
            }

            _head = _next_list[_head];
            if (_head != index_null) {
                _prev_list[_head] = index_null;
            }

            _data_list.pop_back();
            _prev_list.pop_back();
            _next_list.pop_back();
        }

        template<typename... Ts>
        void emplace_back(Ts&&... ts) noexcept {
            index_type index = _data_list.size();
            _data_list.emplace_back(std::forward<Ts>(ts)...);
            _prev_list.emplace_back(_tail);
            _next_list.emplace_back(index_null);

            if (_tail != index_null) {
                _next_list[_tail] = index;
            }

            _tail = index;

            if (_head == index_null) {
                _head = index;
            }
        }

        void pop_back() noexcept {
            if (_tail == index_null) {
                return;
            }

            if (_head != _tail) {
                if (_tail != _next_list.size() - 1) {
                    if (_head == _next_list.size() - 1) {
                        _head = _tail;
                    }

                    swap(_tail, _data_list.size() - 1);

                    _tail = _data_list.size() - 1;
                }
            } else {
                _head = index_null;
            }

            _tail = _prev_list[_tail];
            if (_tail != index_null) {
                _next_list[_tail] = index_null;
            }

            _data_list.pop_back();
            _prev_list.pop_back();
            _next_list.pop_back();
        }

        template<typename... Ts>
        void emplace(index_type index, Ts&&... ts) noexcept {
            if (index == 0) {
                emplace_front(std::forward<Ts>(ts)...);
                return;
            }

            if (index >= _data_list.size()) {
                emplace_back(std::forward<Ts>(ts)...);
                return;
            }

            index_type actual_index = _head;
            for (index_type i = 0; i < index; i++) {
                actual_index = _next_list[actual_index];
            }

            index_type new_idx = _data_list.size();
            _data_list.emplace_back(std::forward<Ts>(ts)...);
            _prev_list.emplace_back(_prev_list[actual_index]);
            _next_list.emplace_back(actual_index);

            _next_list[_prev_list[actual_index]] = new_idx;
            _prev_list[actual_index] = new_idx;
        }

        void pop(index_type index) noexcept {
            if (index == 0) {
                pop_front();
                return;
            }

            if (index >= _data_list.size() - 1) {
                pop_back();
                return;
            }

            index_type actual_index = _head;
            for (index_type i = 0; i < index; i++) {
                actual_index = _next_list[actual_index];
            }

            if (_head == _data_list.size() - 1) {
                _head = actual_index;
            }

            if (_tail == _data_list.size() - 1) {
                _tail = actual_index;
            }

            index_type back_index = _data_list.size() - 1;
            swap(actual_index, back_index);

            if (_prev_list[back_index] != index_null) {
                _next_list[_prev_list[back_index]] = _next_list[back_index];
            }

            if (_next_list[back_index] != index_null) {
                _prev_list[_next_list[back_index]] = _prev_list[back_index];
            }

            _data_list.pop_back();
            _prev_list.pop_back();
            _next_list.pop_back();
        } 

        index_type find(const data_type& data) const noexcept {
            index_type ordered_index = index_null;
            index_type actual_idx = _head;
            while(actual_idx != index_null) {
                ordered_index++;

                if (_data_list[actual_idx] == data) {
                    break;
                }

                actual_idx = _next_list[actual_idx];
            }

            return ordered_index;
        }

        void sort() noexcept {
            if (empty()) {
                return;
            }

            _sort();
        }

    public:
        auto begin() noexcept { return _data_list.begin(); }

        auto end() noexcept { return _data_list.end(); }

        bool empty() const noexcept { return _data_list.empty(); }

        size_type size() const noexcept { return _data_list.size(); }

        auto& operator[](size_type index) noexcept { return _data_list[index]; }

    private:
        data_list_type _data_list;
        index_list_type _prev_list;
        index_list_type _next_list;
        index_type _head = index_null;
        index_type _tail = index_null;

    private:
        void _sort() noexcept {
            index_type ordered_index = _head;
            index_type actual_index = 0;

            while(ordered_index != index_null) {
                swap(actual_index, ordered_index);

                ordered_index = _next_list[actual_index];
                actual_index++;
            }

            _head = 0;
            _tail = actual_index - 1;
        }

        void swap(index_type idx1, index_type idx2) noexcept {
            if (idx1 == idx2) {
                return;
            }

            if (_next_list[idx2] == idx1) {
                _prev_list[idx1] = idx1;
                _next_list[idx2] = idx2;

                if (_prev_list[idx2] != index_null) {
                    _next_list[_prev_list[idx2]] = idx1;
                }

                if (_next_list[idx1] != index_null) {
                    _prev_list[_next_list[idx1]] = idx2;
                }
            } else if (_next_list[idx1] == idx2) {
                _prev_list[idx2] = idx2;
                _next_list[idx1] = idx1;

                if (_prev_list[idx1] != index_null) {
                    _next_list[_prev_list[idx1]] = idx2;
                }

                if (_next_list[idx2] != index_null) {
                    _prev_list[_next_list[idx2]] = idx1;
                }
            } else {
                if (_prev_list[idx1] != index_null) {
                    _next_list[_prev_list[idx1]] = idx2;
                }

                if (_next_list[idx1] != index_null) {
                    _prev_list[_next_list[idx1]] = idx2;
                }

                if (_prev_list[idx2] != index_null) {
                    _next_list[_prev_list[idx2]] = idx1;
                }

                if (_next_list[idx2] != index_null) {
                    _prev_list[_next_list[idx2]] = idx1;
                }
            }

            std::swap(_data_list[idx1], _data_list[idx2]);
            std::swap(_prev_list[idx1], _prev_list[idx2]);
            std::swap(_next_list[idx1], _next_list[idx2]);
        }
    };
}