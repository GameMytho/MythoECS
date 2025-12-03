#pragma once

#include "utils/type_list.hpp"
#include "container/event_storage.hpp"

namespace mytho::utils {
    template<typename T>
    concept PureEventType = PureValueType<T>;
}

namespace mytho::ecs {
    template<
        typename EventIdGeneratorT,
        mytho::utils::UnsignedIntegralType EventIdT = size_t,
        template<typename> typename AllocatorT = std::allocator
    >
    class basic_events final {
    public:
        using event_id_generator_type = EventIdGeneratorT;
        using event_id_type = EventIdT;
        using event_storage_type = mytho::container::basic_event_storage<event_id_generator_type, event_id_type, AllocatorT>;

    public:
        template<mytho::utils::PureValueType T>
        void init() {
            _write_events.template init<T>();
            _read_events.template init<T>();
        }

        template<mytho::utils::PureValueType T>
        void deinit() {
            _write_events.template deinit<T>();
            _read_events.template deinit<T>();
        }

        template<mytho::utils::PureValueType T, typename... Rs>
        void write(Rs&&... rs) {
            _write_events.template write<T>(std::forward<Rs>(rs)...);
        }

        template<mytho::utils::PureValueType T>
        auto mutate() noexcept {
            return _read_events.template mutate<T>();
        }

        template<mytho::utils::PureValueType T>
        auto read() const noexcept {
            return _read_events.template read<T>();
        }

        void swap() {
            _read_events.swap(_write_events);
            _write_events.clear();
        }

    private:
        event_storage_type _read_events;
        event_storage_type _write_events;
    };

    template<typename RegistryT, mytho::utils::PureEventType T>
    class basic_event_writer final {
    public:
        using registry_type = RegistryT;
        using event_type = T;

        basic_event_writer(registry_type& reg) noexcept : _reg(reg) {}

    public:
        template<typename... Ts>
        void write(Ts&&... ts) {
            _reg.template event_write<event_type>(std::forward<Ts>(ts)...);
        }

    private:
        registry_type& _reg;
    };

    template<mytho::utils::PureEventType T>
    class basic_event_mutator final {
    public:
        using event_type = T;
        using events_type = mytho::container::basic_event_set<T>;

        basic_event_mutator(events_type&& events) noexcept : _events(std::move(events)) {}

    public:
        events_type& mutate() noexcept {
            return _events;
        }

    private:
        events_type _events;
    };

    template<mytho::utils::PureEventType T>
    class basic_event_reader final {
    public:
        using event_type = T;
        using events_type = mytho::container::basic_event_set<T>;

        basic_event_reader(events_type&& events) noexcept : _events(std::move(events)) {}

    public:
        const events_type& read() const noexcept {
            return _events;
        }

    private:
        events_type _events;
    };
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_event_writer_v = internal::is_template_v<T, mytho::ecs::basic_event_writer>;

    template<typename T>
    inline constexpr bool is_event_mutator_v = internal::is_template_v<T, mytho::ecs::basic_event_mutator>;

    template<typename T>
    inline constexpr bool is_event_reader_v = internal::is_template_v<T, mytho::ecs::basic_event_reader>;
}