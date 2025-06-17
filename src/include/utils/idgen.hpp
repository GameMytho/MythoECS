#pragma once

namespace mytho::utils {
    template<typename IdT>
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