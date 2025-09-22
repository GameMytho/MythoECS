#pragma once
#include <vector>
#include <cstdint>

namespace mytho::container {
    class basic_tick_set final {
    public:
        using added_ticks_type = std::vector<uint64_t>;
        using changed_ticks_type = std::vector<uint64_t>;
        using size_type = typename added_ticks_type::size_type;

    public:
        uint64_t get_added_tick(size_type index) const noexcept {
            return _added_ticks[index];
        }

        uint64_t& get_added_tick(size_type index) noexcept {
            return _added_ticks[index];
        }

        void set_added_tick(size_type index, uint64_t tick) noexcept {
            _added_ticks[index] = tick;
        }

        uint64_t get_changed_tick(size_type index) const noexcept {
            return _changed_ticks[index];
        }

        uint64_t& get_changed_tick(size_type index) noexcept {
            return _changed_ticks[index];
        }

        void set_changed_tick(size_type index, uint64_t tick) noexcept {
            _changed_ticks[index] = tick;
        }

        void resize(size_type size, uint64_t value = 0) noexcept {
            _added_ticks.resize(size, value);
            _changed_ticks.resize(size, value);
        }

        void clear() noexcept {
            _added_ticks.clear();
            _changed_ticks.clear();
        }

    public:
        size_type size() const noexcept {
            return _added_ticks.size();
        }

        bool empty() const noexcept { return _added_ticks.empty(); }

    private:
        added_ticks_type _added_ticks;
        changed_ticks_type _changed_ticks;
    };
}