#pragma once

#include "ecs/system.hpp"

namespace mytho::ecs::internal {
    template<typename RegistryT>
    class basic_system_schedule final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_system_schedule<registry_type>;

        using system_graph_type = basic_system_graph<registry_type>;
        using meta_systems_pool_type = typename system_graph_type::meta_systems_pool_type;

        using system_type = typename system_graph_type::system_type;

        basic_system_schedule() noexcept = default;
        basic_system_schedule(basic_system_schedule& ss) noexcept = delete;

        basic_system_schedule(basic_system_schedule&& ss) noexcept
            : _system_graph(std::move(ss).system_graph()),
            _meta_systems_pool(std::move(ss).meta_systems_pool()) {}

        basic_system_schedule& operator=(basic_system_schedule&& ss) noexcept {
            _system_graph = std::move(ss).system_graph();
            _meta_systems_pool = std::move(ss).meta_systems_pool();

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        void add(Func&& func) {
            _system_graph.add(std::forward<Func>(func));
        }

        void add(system_type& system) {
            _system_graph.add(system);
        }

    public:
        void ready() {
            _meta_systems_pool = _system_graph.sort();
        }

        void run(registry_type& reg, uint64_t& tick) {
            auto size = _meta_systems_pool.size();
            for (auto i = 0; i < size; ++i) {
                auto& systems = _meta_systems_pool[i];
                auto in_size = systems.size();

                for (auto j = 0; j < in_size; ++j) {
                    auto& system = systems[j];

                    system(reg, tick++);
                }
            }
        }

    public:
        auto system_graph() && noexcept { return std::move(_system_graph); }

        auto meta_systems_pool() && noexcept { return std::move(_meta_systems_pool); }

        auto size() const noexcept { return _meta_systems_pool.size(); }

        void clear() noexcept {
            _system_graph.clear();
            _meta_systems_pool.clear();
        }

    private:
        system_graph_type _system_graph;
        meta_systems_pool_type _meta_systems_pool;
    };

    template<typename RegistryT>
    class basic_schedules final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_schedules<registry_type>;

        using system_schedule_type = basic_system_schedule<registry_type>;

        using schedule_id_generator = typename registry_type::schedule_id_generator;
        using schedule_id_type = typename schedule_id_generator::value_type;
        using system_type = typename system_schedule_type::system_type;

    private:
        struct basic_schedule {
            basic_schedule() noexcept = default;
            basic_schedule(schedule_id_type id, system_schedule_type&& schedule) noexcept
                : _key(id), _schedule(std::move(schedule)) {}

            schedule_id_type _key;
            system_schedule_type _schedule;
        };

    public:
        using schedule_type = basic_schedule;
        using schedules_type = std::vector<schedule_type>;
        using schedule_index_type = typename schedules_type::size_type;

        inline static constexpr schedule_index_type schedule_index_null = std::numeric_limits<schedule_index_type>::max();

    public:
        template<auto ScheduleE>
        self_type& add_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(!_contain(id), "new schedule already exists!");
            _schedules.emplace_back(id, system_schedule_type());

            return *this;
        }

        template<auto ScheduleE, auto BeforeScheduleE>
        self_type& add_schedule_before() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(!_contain(id), "new schedule already exists!");

            auto before_id = schedule_id_generator::template gen<BeforeScheduleE>();
            auto idx = _index(before_id);

            ASSURE(idx != schedule_index_null, "before-schedule not exist!");
            _schedules.insert(_schedules.begin() + idx, schedule_type(id, system_schedule_type()));

            return *this;
        }

        template<auto ScheduleE, auto AfterScheduleE>
        self_type& add_schedule_after() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(!_contain(id), "new schedule already exists!");

            auto after_id = schedule_id_generator::template gen<AfterScheduleE>();
            auto idx = _index(after_id);

            ASSURE(idx != schedule_index_null, "after-schedule not exist!");
            _schedules.insert(_schedules.begin() + idx + 1, schedule_type(id, system_schedule_type()));

            return *this;
        }

        template<auto ScheduleE, auto InsertScheduleE>
        self_type& insert_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(!_contain(id), "new schedule already exists!");

            auto insert_id = schedule_id_generator::template gen<InsertScheduleE>();
            auto idx = _index(insert_id);

            if (idx == schedule_index_null) {
                _schedules.emplace_back(id, system_schedule_type());
            } else {
                _schedules[idx]._key = id;
                _schedules[idx]._schedule.clear();

                if (_default == insert_id) {
                    _default = id;
                }
            }

            return *this;
        }

        template<auto ScheduleE>
        self_type& set_default_schedule() noexcept {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            _default = id;

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            ASSURE(_index(_default) < _schedules.size(), "no available default schedule!");

            _schedules[_index(_default)]._schedule.add(std::forward<Func>(func));

            return *this;
        }

        template<auto ScheduleE, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_contain(id), "schedule not exist!");
            _schedules[_index(id)]._schedule.add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_system(system_type& system) {
            ASSURE(_index(_default) < _schedules.size(), "no available default schedule!");

            _schedules[_index(_default)]._schedule.add(system);

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_system(system_type& system) {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_contain(id), "schedule not exist!");
            _schedules[_index(id)]._schedule.add(system);

            return *this;
        }

    public:
        void ready() {
            auto size = _schedules.size();
            for (auto i = 0; i < size; ++i) {
                _schedules[i]._schedule.ready();
            }
        }

        void run(registry_type& reg, uint64_t& tick) {
            auto size = _schedules.size();
            for (auto i = 0; i < size; ++i) {
                _schedules[i]._schedule.run(reg, tick);
            }
        }

    private:
        schedules_type _schedules;
        schedule_id_type _default = 0;

    private:
        bool _contain(schedule_id_type id) const noexcept {
            auto size = _schedules.size();
            for (auto i = 0; i < size; ++i) {
                if (_schedules[i]._key == id) {
                    return true;
                }
            }

            return false;
        }

        schedule_index_type _index(schedule_id_type id) const noexcept {
            auto size = _schedules.size();
            for (auto i = 0; i < size; ++i) {
                if (_schedules[i]._key == id) {
                    return i;
                }
            }

            return schedule_index_null;
        }
    };
}