#pragma once

namespace mytho::utils {
    enum class GeneratorType : uint8_t {
        COMPONENT_GENOR,
        RESOURCE_GENOR,
        EVENT_GENOR
    };

    template<GeneratorType GT, typename IdT>
    struct basic_id_generator {
        using value_type = IdT;

        template<typename T>
        static value_type gen() {
            static value_type id = _cur_id++;
            return id;
        }

        inline static value_type _cur_id = 0;
    };
}