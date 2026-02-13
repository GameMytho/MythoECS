#pragma once

#include "utils/concept.hpp"
#include "ecs/resources.hpp"

#include <optional>

namespace mytho::utils {
    // require the state type is scoped enumeration and its underlying type must is uint8_t.
    template<typename T>
    concept PureStateType = PureEnumClassType<T> && std::is_same_v<std::underlying_type_t<T>, uint8_t>;
}

namespace mytho::ecs {
    template<mytho::utils::PureStateType StateT>
    class basic_state final {
    public:
        using value_type = StateT;

        basic_state(value_type state) : _value(state) {}

    public:
        const value_type get() const noexcept { return _value; }

    //protected:
        void set(value_type value) noexcept { _value = value; }

    private:
        value_type _value;
    };

    template<mytho::utils::PureStateType StateT>
    class basic_next_state final {
    public:
        using state_type = StateT;
        using value_type = std::optional<state_type>;

        basic_next_state() : _value(std::nullopt) {}

    public:
        void set(state_type state) noexcept { _value = state; }

    //protected:
        const value_type get() const noexcept { return _value; }

        void reset() noexcept { _value = std::nullopt; }

    private:
        value_type _value;
    };

    template<auto StateE>
    class on_enter final {};

    template<auto StateE>
    class on_exit final {};
}