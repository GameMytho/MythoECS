#pragma once

#include "core/concept.hpp"
#include "ecs/resources.hpp"

#include <optional>

namespace mytho::ecs {
    // require the state type is scoped enumeration and its underlying type must is uint8_t.
    template<typename T>
    concept PureStateType = mytho::core::PureEnumClassType<T> && std::is_same_v<std::underlying_type_t<T>, uint8_t>;

    // friend class pre-definition
    template<PureStateType StateT> class basic_state_helper;

    template<PureStateType StateT>
    class basic_state final {
    public:
        using value_type = StateT;

        basic_state(value_type state) : _value(state) {}

    public:
        const value_type get() const noexcept { return _value; }

    protected:
        void set(value_type value) noexcept { _value = value; }

    private:
        value_type _value;

        friend class basic_state_helper<value_type>;
    };

    template<PureStateType StateT>
    class basic_next_state final {
    public:
        using state_type = StateT;
        using value_type = std::optional<state_type>;

        basic_next_state() : _value(std::nullopt) {}

    public:
        void set(state_type state) noexcept { _value = state; }

    protected:
        const value_type get() const noexcept { return _value; }

        void reset() noexcept { _value = std::nullopt; }

    private:
        value_type _value;

        friend class basic_state_helper<state_type>;
    };

    template<PureStateType StateT>
    class basic_state_helper final {
    public:
        using state_value_type = StateT;

    public:
        inline static void set_state(basic_state<state_value_type>& state, state_value_type value) {
            state.set(value);
        }

        inline static auto get_next_state(const basic_next_state<state_value_type>& next_state) noexcept {
            return next_state.get();
        }

        inline static void reset_next_state(basic_next_state<state_value_type>& next_state) noexcept {
            next_state.reset();
        }
    };

    template<auto StateE>
    class on_enter final {};

    template<auto StateE>
    class on_exit final {};
}