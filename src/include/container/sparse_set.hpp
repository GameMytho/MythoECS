#pragma once
#include <array>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <limits>
#include <utility>
#include <cstddef>

#include "utils/assert.hpp"
#include "utils/concept.hpp"

namespace mytho::container {
    template<mytho::utils::UnsignedIntegralType T, size_t PageSize = 256>
    class basic_sparse_set {
    public:
        using data_type = T;
        using density_type = std::vector<data_type>;
        using size_type = typename density_type::size_type;
        using page_data_type = typename density_type::size_type;
        struct alignas(64) page_type : public std::array<page_data_type, PageSize> {};
        using sparsity_type = std::vector<page_type>;
        using iterator = typename density_type::iterator;
        using const_iterator = typename density_type::const_iterator;

        static constexpr data_type data_null = std::numeric_limits<data_type>::max();

        basic_sparse_set() noexcept = default;
        basic_sparse_set(const basic_sparse_set& ss) = delete;
        basic_sparse_set(basic_sparse_set&& ss) noexcept = default;

        basic_sparse_set& operator=(const basic_sparse_set& ss) = delete;
        basic_sparse_set& operator=(basic_sparse_set&& ss) noexcept = default;

        ~basic_sparse_set() noexcept = default;

    public:
        // must ensure data value not exist
        size_type add(data_type data) {
            auto size = _density.size();

            expand(page(data))[offset(data)] = size;
            _density.push_back(data);

            return size;
        }

        // must ensure data value exist
        void remove(data_type data) noexcept {
            data_type last_data = _density.back();
            page_data_type& data_ref = sparse_ref(data);
            page_data_type pos = data_ref;

            data_ref = data_null;
            if (data != last_data) {
                sparse_ref(last_data) = pos;
                _density[pos] = last_data;
            }
            _density.pop_back();
        }

        // must ensure data value exist
        std::pair<size_type, size_type> swap(data_type src, data_type dst) noexcept {
            page_data_type& sref = sparse_ref(src);
            page_data_type& dref = sparse_ref(dst);

            std::swap(sref, dref);
            std::swap(_density[sref], _density[dref]);

            return {sref, dref};
        }

        // must ensure data value exist
        size_type index(data_type data) const noexcept {
            auto& sparse = _sparsity[page(data)];
            return sparse[offset(data)];
        }

        bool contain(data_type data) const noexcept {
            ASSURE(data != data_null, "invalid integral value(value reach max).");

            auto idx = page(data);
            if (idx >= _sparsity.size()) return false;

            auto& sparse = _sparsity[idx];

            return sparse[offset(data)] != data_null;
        }

        void clear() noexcept {
            _density.clear();
            _sparsity.clear();
        }

        data_type data(size_type idx) const noexcept {
            return _density[idx];
        }

    public:
        iterator begin() noexcept { return _density.begin(); }
        const_iterator begin() const noexcept { return _density.begin(); }

        iterator end() noexcept { return _density.end(); }
        const_iterator end() const noexcept { return _density.end(); }

        size_type size() const noexcept { return _density.size(); }

        bool empty() const noexcept { return _density.empty(); }

        const auto& operator[](auto idx) const { return _density[idx]; }

    private:
        density_type _density;
        sparsity_type _sparsity;

    private:
        size_type page(data_type data) const noexcept { return data / PageSize; }

        size_type offset(data_type data) const noexcept { return data % PageSize; }

        page_type& expand(size_t idx) {
            auto old_size = _sparsity.size();
            if (idx >= old_size) {
                _sparsity.resize(idx + 1);
                auto new_size = _sparsity.size();
                for (size_t i = old_size; i < new_size; ++i) {
                    auto& sparse = _sparsity[i];
                    std::fill(sparse.begin(), sparse.end(), data_null);
                }
            }

            return _sparsity[idx];
        }

        page_data_type& sparse_ref(data_type data) noexcept {
            auto& sparse = _sparsity[page(data)];
            return sparse[offset(data)];
        }
    };
}