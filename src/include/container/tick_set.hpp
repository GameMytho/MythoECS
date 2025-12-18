#pragma once
#include <vector>
#include <cstdint>

namespace mytho::container {
    class basic_tick_set final {
    public:
        using added_ticks_type = std::vector<uint64_t>;
        using changed_ticks_type = std::vector<uint64_t>;
        using size_type = typename added_ticks_type::size_type;

        basic_tick_set() noexcept = default;
        basic_tick_set(const basic_tick_set& ts) = delete;
        basic_tick_set(basic_tick_set&& ts) noexcept = default;

        basic_tick_set& operator=(const basic_tick_set& ts) = delete;
        basic_tick_set& operator=(basic_tick_set&& ts) noexcept = default;

        ~basic_tick_set() noexcept = default;

    public:
        // must ensure index valid
        uint64_t get_added_tick(size_type index) const noexcept {
            return _added_ticks[index];
        }

        // must ensure index valid
        uint64_t& get_added_tick(size_type index) noexcept {
            return _added_ticks[index];
        }

        // must ensure index valid
        void set_added_tick(size_type index, uint64_t tick) noexcept {
            _added_ticks[index] = tick;
        }

        // must ensure index valid
        uint64_t get_changed_tick(size_type index) const noexcept {
            return _changed_ticks[index];
        }

        // must ensure index valid
        uint64_t& get_changed_tick(size_type index) noexcept {
            return _changed_ticks[index];
        }

        // must ensure index valid
        void set_changed_tick(size_type index, uint64_t tick) noexcept {
            _changed_ticks[index] = tick;
        }

        // must ensure index valid
        void swap_ticks(size_type l, size_type r) noexcept {
            std::swap(_added_ticks[l], _added_ticks[r]);
            std::swap(_changed_ticks[l], _changed_ticks[r]);
        }

        void resize(size_type size, uint64_t value = 0) {
            _added_ticks.resize(size, value);
            _changed_ticks.resize(size, value);
        }

        void clear() noexcept {
            _added_ticks.clear();
            _changed_ticks.clear();
        }

    public:
        size_type size() const noexcept { return _added_ticks.size(); }

        bool empty() const noexcept { return _added_ticks.empty(); }

    private:
        added_ticks_type _added_ticks;
        changed_ticks_type _changed_ticks;
    };
}