#pragma once

#include "ecs/system.hpp"

namespace mytho::ecs::internal {
    template<typename RegistryT, typename StageIdT = uint8_t>
    class basic_schedule {
    public:
        using registry_type = RegistryT;
        using stage_id_type = StageIdT;
        using self_type = basic_schedule<registry_type, stage_id_type>;
        using stage_id_generator = mytho::utils::basic_id_generator<mytho::utils::GeneratorType::STAGE_GENOR, stage_id_type>;
        using system_storage_type = basic_system_storage<registry_type>;
        using system_config_type = typename system_storage_type::system_config_type;

    private:
        struct basic_stage {
            basic_stage() = default;
            basic_stage(stage_id_type id, system_storage_type&& storage)
                : _key(id), _storage(std::move(storage)) {}

            stage_id_type _key;
            system_storage_type _storage;
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
            _stages.emplace_back(id, system_storage_type());

            return *this;
        }

        template<auto StageE, auto BeforeStageE>
        self_type& add_stage_before() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto before_id = stage_id_generator::template gen<BeforeStageE>();
            auto idx = _index(before_id);

            ASSURE(idx != stage_index_null, "before-stage not exist!");
            _stages.insert(_stages.begin() + idx, stage_type(id, system_storage_type()));

            return *this;
        }

        template<auto StageE, auto AfterStageE>
        self_type& add_stage_after() {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto after_id = stage_id_generator::template gen<AfterStageE>();
            auto idx = _index(after_id);

            ASSURE(idx != stage_index_null, "after-stage not exist!");
            _stages.insert(_stages.begin() + idx + 1, stage_type(id, system_storage_type()));

            return *this;
        }

        template<auto StageE, auto InsertStageE>
        self_type& insert_stage() noexcept {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(!_contain(id), "new stage already exists!");

            auto insert_id = stage_id_generator::template gen<InsertStageE>();
            auto idx = _index(insert_id);

            if (idx == stage_index_null) {
                _stages.push_back(stage_type(id, system_storage_type()));
            } else {
                _stages.emplace_back(id, system_storage_type());
                std::swap(_stages[idx], _stages.back());
            }

            return *this;
        }

    public:
        template<auto StageE, mytho::utils::FunctionType Func>
        self_type& add_system(Func&& func) noexcept {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(_contain(id), "stage not exist!");
            _stages[_index(id)]._storage.add(std::forward<Func>(func));

            return *this;
        }

        template<auto StageE>
        self_type& add_system(system_config_type& config) noexcept {
            auto id = stage_id_generator::template gen<StageE>();

            ASSURE(_contain(id), "stage not exist!");
            _stages[_index(id)]._storage.add(config);

            return *this;
        }

    public:
        void ready() {
            for (auto& [id, storage] : _stages) {
                storage.ready();
            }
        }

        void run(registry_type& reg, uint64_t& tick) noexcept {
            for (auto& [id, storage] : _stages) {
                storage.run(reg, tick);
            }
        }

    private:
        stages_type _stages;

    private:
        bool _contain(stage_id_type id) const noexcept {
            for (auto& stage : _stages) {
                if (stage._key == id) {
                    return true;
                }
            }

            return false;
        }

        stage_index_type _index(stage_id_type id) const noexcept {
            for (stage_index_type i = 0; i < _stages.size(); ++i) {
                if (_stages[i]._key == id) {
                    return i;
                }
            }

            return stage_index_null;
        }
    };
}