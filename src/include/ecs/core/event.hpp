#pragma once

#include "core/concept.hpp"
#include "core/type_list.hpp"

namespace mytho::ecs {
    template<typename T>
    concept PureEventType = mytho::core::PureValueType<T>;

    template<PureEventType EventT>
    class basic_events final {
    public:
        using event_type = EventT;
        using events_type = std::vector<event_type>;

    public:
        auto& write() noexcept {
            return _write_events;
        }

        auto& mutate() noexcept {
            return _read_events;
        }

        const auto& read() const noexcept {
            return _read_events;
        }

        void swap() noexcept {
            _read_events.swap(_write_events);
            _write_events.clear();
        }

    private:
        events_type _read_events;
        events_type _write_events;
    };

    template<PureEventType EventT>
    class basic_event_writer final {
    public:
        using event_type = EventT;
        using events_type = typename basic_events<event_type>::events_type;

        basic_event_writer(events_type& events) noexcept : _events(events) {}

    public:
        template<typename... Ts>
        void write(Ts&&... ts) {
            _events.emplace_back(std::forward<Ts>(ts)...);
        }

    private:
        events_type& _events;
    };

    template<PureEventType EventT>
    class basic_event_mutator final {
    public:
        using event_type = EventT;
        using events_type = typename basic_events<event_type>::events_type;

        basic_event_mutator(events_type& events) noexcept : _events(events) {}

    public:
        events_type& mutate() noexcept {
            return _events;
        }

    private:
        events_type& _events;
    };

    template<PureEventType EventT>
    class basic_event_reader final {
    public:
        using event_type = EventT;
        using events_type = typename basic_events<event_type>::events_type;

        basic_event_reader(const events_type& events) noexcept : _events(events) {}

    public:
        const events_type& read() const noexcept {
            return _events;
        }

    private:
        const events_type& _events;
    };

    template<typename T>
    inline constexpr bool is_event_writer_v = mytho::core::is_template_v<T, basic_event_writer>;

    template<typename T>
    inline constexpr bool is_event_mutator_v = mytho::core::is_template_v<T, basic_event_mutator>;

    template<typename T>
    inline constexpr bool is_event_reader_v = mytho::core::is_template_v<T, basic_event_reader>;
}