#pragma once

#include "ecs/system.hpp"

namespace mytho::ecs::internal {
    template<typename RegistryT>
    class basic_system_schedule final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_system_schedule<registry_type>;

        using meta_system_type = basic_meta_system<registry_type>;
        using meta_systems_type = std::vector<meta_system_type>;

        using meta_systems_pool_type = std::vector<meta_systems_type>;

        basic_system_schedule() noexcept = default;
        basic_system_schedule(basic_system_schedule& ss) noexcept = delete;

        basic_system_schedule(basic_system_schedule&& ss) noexcept
            : _meta_systems_pool(std::move(ss).meta_systems_pool()) {}

        basic_system_schedule(meta_systems_pool_type&& pool) noexcept
            : _meta_systems_pool(pool) {}

        basic_system_schedule& operator=(basic_system_schedule&& ss) noexcept {
            _meta_systems_pool = std::move(ss).meta_systems_pool();

            return *this;
        }

        basic_system_schedule& operator=(meta_systems_pool_type&& pool) noexcept {
            _meta_systems_pool = pool;

            return *this;
        }

    public:
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
        auto meta_systems_pool() && noexcept { return std::move(_meta_systems_pool); }

        auto size() const noexcept { return _meta_systems_pool.size(); }

        void clear() noexcept { _meta_systems_pool.clear(); }

    private:
        meta_systems_pool_type _meta_systems_pool;
    };

    template<typename RegistryT>
    class basic_schedules final {
    public:
        using registry_type = RegistryT;
        using self_type = basic_schedules<registry_type>;

        using system_schedule_type = basic_system_schedule<registry_type>;
        using system_graph_type = basic_system_graph<registry_type>;

        using schedule_id_generator = typename registry_type::schedule_id_generator;
        using schedule_id_type = typename schedule_id_generator::value_type;
        using system_type = typename system_graph_type::system_type;

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
        using graphs_type = std::vector<system_graph_type>;
        using schedule_index_type = typename schedules_type::size_type;

        inline static constexpr schedule_index_type schedule_index_null = std::numeric_limits<schedule_index_type>::max();

    public:
        template<auto ScheduleE>
        self_type& add_startup_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            _schedules.insert(_schedules.begin() + _startup_end_index, schedule_type(id, system_schedule_type()));
            _graphs.insert(_graphs.begin() + _startup_end_index, system_graph_type());

            ++_startup_end_index;
            ++_update_end_index;

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_update_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            _schedules.insert(_schedules.begin() + _update_end_index, schedule_type(id, system_schedule_type()));
            _graphs.insert(_graphs.begin() + _update_end_index, system_graph_type());

            ++_update_end_index;

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            _schedules.emplace_back(schedule_type(id, system_schedule_type()));
            _graphs.emplace_back(system_graph_type());

            return *this;
        }

        template<typename ScheduleT>
        self_type& add_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleT>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            _schedules.emplace_back(schedule_type(id, system_schedule_type()));
            _graphs.emplace_back(system_graph_type());

            return *this;
        }

        template<auto ScheduleE, auto BeforeScheduleE>
        self_type& add_schedule_before() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            auto before_id = schedule_id_generator::template gen<BeforeScheduleE>();
            auto idx = _index(before_id);

            ASSURE(idx != schedule_index_null, "before-schedule not exist!");

            _schedules.insert(_schedules.begin() + idx, schedule_type(id, system_schedule_type()));
            _graphs.insert(_graphs.begin() + idx, system_graph_type());

            if (idx < _startup_end_index) {
                ++_startup_end_index;
                ++_update_end_index;
            } else if (idx < _update_end_index) {
                ++_update_end_index;
            }

            return *this;
        }

        template<auto ScheduleE, auto AfterScheduleE>
        self_type& add_schedule_after() {
            auto id = schedule_id_generator::template gen<ScheduleE>();

            ASSURE(_index(id) == schedule_index_null, "new schedule already exists!");

            auto after_id = schedule_id_generator::template gen<AfterScheduleE>();
            auto idx = _index(after_id);

            ASSURE(idx != schedule_index_null, "after-schedule not exist!");

            _schedules.insert(_schedules.begin() + idx + 1, schedule_type(id, system_schedule_type()));
            _graphs.insert(_graphs.begin() + idx + 1, system_graph_type());

            if (idx < _startup_end_index) {
                ++_startup_end_index;
                ++_update_end_index;
            } else if (idx < _update_end_index) {
                ++_update_end_index;
            }

            return *this;
        }

        template<auto ScheduleE, auto InsertScheduleE>
        self_type& insert_schedule() {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            auto idx = _index(id);

            ASSURE(idx == schedule_index_null, "new schedule already exists!");

            auto insert_id = schedule_id_generator::template gen<InsertScheduleE>();
            auto insert_idx = _index(insert_id);

            if (insert_idx == schedule_index_null) {
                _schedules.emplace_back(id, system_schedule_type());
                _graphs.emplace_back();
            } else {
                _schedules[insert_idx]._key = id;
                _schedules[insert_idx]._schedule.clear();
                _graphs[insert_idx].clear();

                if (_default_index == insert_idx) {
                    _default_index = idx;
                }
            }

            return *this;
        }

        template<auto ScheduleE>
        self_type& set_default_schedule() noexcept {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            _default_index = _index(id);

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            ASSURE(_default_index < _schedules.size(), "no available default schedule!");

            _graphs[_default_index].add(std::forward<Func>(func));

            return *this;
        }

        template<auto ScheduleE, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "schedule not exist!");

            _graphs[idx].add(std::forward<Func>(func));

            return *this;
        }

        template<typename ScheduleT, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            auto id = schedule_id_generator::template gen<ScheduleT>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "schedule not exist!");

            _graphs[idx].add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_system(system_type& system) {
            ASSURE(_default_index < _schedules.size(), "no available default schedule!");

            _graphs[_default_index].add(system);

            return *this;
        }

        template<auto ScheduleE>
        self_type& add_system(system_type& system) {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "schedule not exist!");

            _graphs[idx].add(system);

            return *this;
        }

        template<typename ScheduleT>
        self_type& add_system(system_type& system) {
            auto id = schedule_id_generator::template gen<ScheduleT>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "schedule not exist!");

            _graphs[idx].add(system);

            return *this;
        }

    public:
        void run(registry_type& reg, uint64_t& tick) {
            auto size = _graphs.size();
            for (auto i = 0; i < size; ++i) {
                _schedules[i]._schedule = std::move(_graphs[i].sort());
            }

            for (auto i = 0; i < _startup_end_index; ++i) {
                _schedules[i]._schedule.run(reg, tick);
            }

            while(_running) {
                for (auto i = _startup_end_index; i < _update_end_index; ++i) {
                    _schedules[i]._schedule.run(reg, tick);
                }
            }
        }

        void exit() {
            _running = false;
        }

        template<auto ScheduleE>
        void run_schedule(registry_type& reg, uint64_t& tick) {
            auto id = schedule_id_generator::template gen<ScheduleE>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "new schedule already exists!");

            _schedules[idx]._schedule.run(reg, tick);
        }

        template<typename ScheduleT>
        void run_schedule(registry_type& reg, uint64_t& tick) {
            auto id = schedule_id_generator::template gen<ScheduleT>();
            auto idx = _index(id);

            ASSURE(idx != schedule_index_null, "new schedule already exists!");

            _schedules[idx]._schedule.run(reg, tick);
        }

    private:
        bool _running = true;
        schedule_index_type _default_index = schedule_index_null;
        schedule_index_type _startup_end_index = 0;
        schedule_index_type _update_end_index = 0;
        schedules_type _schedules;
        graphs_type _graphs;

    private:
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