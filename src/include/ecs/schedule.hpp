#pragma once

#include "ecs/system.hpp"

namespace mytho::ecs::internal {
    template<typename RegistryT>
    class basic_schedule final {
    public:
        using registry_type = RegistryT;
        using stage_id_generator = typename registry_type::stage_id_generator;
        using stage_id_type = typename stage_id_generator::value_type;
        using self_type = basic_schedule<registry_type>;
        using system_stage_type = basic_system_stage<registry_type>;
        using system_type = typename system_stage_type::system_type;

    private:
        struct basic_stage {
            basic_stage() = default;
            basic_stage(stage_id_type id, system_stage_type&& stage)
                : _key(id), _stage(std::move(stage)) {}

            stage_id_type _key;
            system_stage_type _stage;
        };

    public:
        using stage_type = basic_stage;
        using stages_type = std::vector<stage_type>;
        using stage_index_type = typename stages_type::size_type;

        inline static constexpr stage_index_type stage_index_null = std::numeric_limits<stage_index_type>::max();

    public:
        template<auto StageE>
        self_type& add_stage() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");
            _stages.emplace_back(id, system_stage_type());

            return *this;
        }

        template<auto StageE, auto BeforeStageE>
        self_type& add_stage_before() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto before_id = stage_id_generator::template gen<BeforeStageE>();
            auto idx = _index(before_id);

            ASSURE(idx != stage_index_null, "before-stage not exist!");
            _stages.insert(_stages.begin() + idx, stage_type(id, system_stage_type()));

            return *this;
        }

        template<auto StageE, auto AfterStageE>
        self_type& add_stage_after() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto after_id = stage_id_generator::template gen<AfterStageE>();
            auto idx = _index(after_id);

            ASSURE(idx != stage_index_null, "after-stage not exist!");
            _stages.insert(_stages.begin() + idx + 1, stage_type(id, system_stage_type()));

            return *this;
        }

        template<auto StageE, auto InsertStageE>
        self_type& insert_stage() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto insert_id = stage_id_generator::template gen<InsertStageE>();
            auto idx = _index(insert_id);

            if (idx == stage_index_null) {
                _stages.emplace_back(id, system_stage_type());
            } else {
                _stages[idx]._key = id;
                _stages[idx]._stage.clear();

                if (_default == insert_id) {
                    _default = id;
                }
            }

            return *this;
        }

        template<auto StageE>
        self_type& set_default_stage() {
            auto id = stage_id_generator::template gen<StageE>();
            _default = id;

            return *this;
        }

    public:
        template<mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            ASSURE(_index(_default) < _stages.size(), "no available default stage!");

            _stages[_index(_default)]._stage.add(std::forward<Func>(func));

            return *this;
        }

        template<auto StageE, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(_contain(id), "stage not exist!");
            _stages[_index(id)]._stage.add(std::forward<Func>(func));

            return *this;
        }

        self_type& add_system(system_type& system) {
            ASSURE(_index(_default) < _stages.size(), "no available default stage!");

            _stages[_index(_default)]._stage.add(system);

            return *this;
        }

        template<auto StageE>
        self_type& add_system(system_type& system) {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(_contain(id), "stage not exist!");
            _stages[_index(id)]._stage.add(system);

            return *this;
        }

    public:
        void run(registry_type& reg, uint64_t& tick) {
            auto size = _stages.size();
            for (auto i = 0; i < size; ++i) {
                _stages[i]._stage.run(reg, tick);
            }
        }

    private:
        stages_type _stages;
        stage_id_type _default = 0;

    private:
        bool _contain(stage_id_type id) const noexcept {
            auto size = _stages.size();
            for (auto i = 0; i < size; ++i) {
                if (_stages[i]._key == id) {
                    return true;
                }
            }

            return false;
        }

        stage_index_type _index(stage_id_type id) const noexcept {
            auto size = _stages.size();
            for (auto i = 0; i < size; ++i) {
                if (_stages[i]._key == id) {
                    return i;
                }
            }

            return stage_index_null;
        }
    };
}