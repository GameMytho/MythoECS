#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <limits>

#include "utils/assert.hpp"
#include "utils/concept.hpp"

namespace mytho::container {
    template<mytho::utils::UnsignedIntegralType T, size_t PageSize = 1024>
    class basic_sparse_set {
    public:
        using data_type = T;
        using density_type = std::vector<data_type>;
        using size_type = typename density_type::size_type;
        using page_data_type = typename density_type::size_type;
        using page_type = std::array<page_data_type, PageSize>;
        using sparsity_type = std::vector<page_type>;

        static constexpr data_type data_null = std::numeric_limits<data_type>::max();

    public:
        void add(data_type data) noexcept {
            ASSURE(!contain(data), "invalid integral value(value exist).");

            _density.push_back(data);
            expand(page(data))[offset(data)] = _density.size() - 1;
        }

        void remove(data_type data) noexcept {
            ASSURE(contain(data), "invalid integral value(value not exist).");

            if (data != _density.back()) {
                page_data_type pos = sparse_ref(data);
                _density[pos] = _density.back();
                sparse_ref(_density[pos]) = pos;
            }

            sparse_ref(data) = data_null;
            _density.pop_back();
        }

        void swap(data_type src, data_type dst) {
            ASSURE(contain(src) && contain(dst), "invalid integral value(value not exist).");

            if (src != dst) {
                std::swap(sparse_ref(src), sparse_ref(dst));
                std::swap(_density[sparse_ref(src)], _density[sparse_ref(dst)]);
            }
        }

        bool contain(data_type data) const noexcept {
            ASSURE(data != data_null, "invalid integral value(value reach max).");

            return page(data) < _sparsity.size() && _sparsity[page(data)][offset(data)] != data_null;
        }

        size_type index(data_type data) const noexcept {
            ASSURE(contain(data), "invalid integral value(value not exist).");

            return _sparsity[page(data)][offset(data)];
        }

        void clear() noexcept {
            _density.clear();
            _sparsity.clear();
        }

        size_type size() const noexcept { return _density.size(); }

        T data(size_type idx) const noexcept {
            ASSURE(idx < _density.size(), "index out of sparse set!");

            return _density[idx];
        }

    private:
        density_type _density;
        sparsity_type _sparsity;

    private:
        size_type page(data_type data) const noexcept { return data / PageSize; }

        size_type offset(data_type data) const noexcept { return data % PageSize; }

        page_type& expand(size_t idx) noexcept {
            if (idx >= _sparsity.size()) {
                size_t old_size = _sparsity.size();
                _sparsity.resize(idx + 1);
                for (size_t i = old_size; i < _sparsity.size(); i++) {
                    std::fill(_sparsity[i].begin(), _sparsity[i].end(), data_null);
                }
            }

            return _sparsity[idx];
        }

        page_data_type& sparse_ref(data_type data) noexcept { return _sparsity[page(data)][offset(data)]; }
    };
}