#pragma once

#include "utils/assert.hpp"
#include "utils/concept.hpp"

#include <cassert>
#include <utility>
#include <array>
#include <string_view>
#include <limits>
#include <cstddef>

// mini-magic_enum impletation, waiting for c++26 reflection

namespace mytho::utils {
    namespace internal {
        template<auto E>
        consteval bool v() {
#if defined(__clang__) || defined(__GNUC__)
            constexpr std::string_view s = __PRETTY_FUNCTION__;
            /*
             * clang 13.0.1/14.0.0/15.0.7/16.0.6/17.0.6/18.1.3/19.1.1/20.1.2 example:
             *      bool mytho::utils::internal::v() [E = sbo::GameState::Menu]
             *      bool mytho::utils::internal::v() [E = (sbo::GameState)3]
             * 
             * g++ 11.4.0/12.3.0/13.1.0/14.2.0 example:
             *      consteval bool mytho::utils::internal::v() [with auto E = sbo::GameState::Menu]
             *      consteval bool mytho::utils::internal::v() [with auto E = (sbo::GameState)3]
            */
            auto start = s.find("E = ") + 4;
            return s.find("::", start) != std::string_view::npos && s.find("(", start) == std::string_view::npos;
#elif defined(_MSC_VER)
            constexpr std::string_view s = __FUNCSIG__;
            /*
             *msvc v143/v145 example:
             *      class bool __cdecl mytho::utils::internal::v<sbo::GameState::Menu>(void)
             *      class bool __cdecl mytho::utils::internal::v<(enum sbo::GameState)0x3>(void)
             */
            //static_assert(false, __FUNCSIG__);

            auto start = s.find("v<") + 2;
            return s.find("0x", start) == std::string_view::npos;
#else
    #error Unsupported Compiler
#endif
        }

        template<typename EnumT, auto Min, auto Max, EnumT... EnumValues>
        requires(Min <= Max)
        class enum_holder {
        private:
            template<typename IndexT, IndexT IndexDefault, std::size_t... I>
            static consteval std::array<IndexT, sizeof...(I)> make_filled_array(std::index_sequence<I...>) {
                return { ((void)I, IndexDefault)... };
            }

            template<typename IndexT, IndexT IndexDefault, size_t Size>
            static consteval auto make_indices() {
                using U = std::underlying_type_t<EnumT>;

                auto array = make_filled_array<IndexT, IndexDefault>(std::make_index_sequence<Size>());

                size_t i = 0;
                ((array[((Size + static_cast<U>(EnumValues)) % Size)] = static_cast<IndexT>(i++)), ...);

                return array;
            }

        public:
            static std::optional<size_t> index(EnumT e) {
                using U = std::underlying_type_t<EnumT>;
                U u = static_cast<U>(e);
                size_t idx = (_indices_size + u) % _indices_size;
                return _indices[idx] == _index_null ? std::nullopt : std::make_optional(idx);
            }

            static constexpr size_t _values_size = sizeof...(EnumValues);
            static constexpr std::array<EnumT, _values_size> _values{ EnumValues... };

            static constexpr size_t _indices_size = Max - Min + 1;
            static constexpr size_t _index_null = std::numeric_limits<size_t>::max();
            static constexpr std::array<size_t, _indices_size> _indices = make_indices<size_t, _index_null, _indices_size>();
        };

        template<typename T, auto RealMin, auto RealMax, auto I, auto Max, T... Es>
        consteval auto enum_holder_query() {
            if constexpr (I < Max) {
                constexpr T e = static_cast<T>(I);
                if constexpr (v<e>()) {
                    constexpr auto real_min = I < RealMin ? I : RealMin;
                    constexpr auto real_max = I > RealMax ? I : RealMax;
                    return enum_holder_query<T, real_min, real_max, I + 1, Max, Es..., e>();
                } else {
                    return enum_holder_query<T, RealMin, RealMax, I + 1, Max, Es...>();
                }
            } else {
                return enum_holder<T, RealMin, RealMax, Es...>{};
            }
        }
    }

    template<PureEnumClassType T, auto Min, auto Max>
    consteval const auto& enum_values() {
        using result_type = decltype(internal::enum_holder_query<T, Max, Min, Min, Max>());
        return std::decay_t<result_type>::_values;
    }

    template<PureEnumClassType T, auto Min, auto Max>
    consteval const auto& enum_indices() {
        using result_type = decltype(internal::enum_holder_query<T, Max, Min, Min, Max>());
        return std::decay_t<result_type>::_indices;
    }

    template<PureEnumClassType T, auto Min, auto Max>
    constexpr std::optional<size_t> enum_index(T e) {
        using result_type = decltype(internal::enum_holder_query<T, Max, Min, Min, Max>());
        return std::decay_t<result_type>::index(e);
    }

    namespace internal {
        template<PureEnumClassType T, size_t I, typename HolderT, typename F>
        void enum_switch_imple(F&& f, size_t idx) {
            if constexpr (I < HolderT::_values_size) {
                if (idx == I) {
                    f.template operator()<HolderT::_values[I]>();
                } else {
                    enum_switch_imple<T, I + 1, HolderT, F>(std::forward<F>(f), idx);
                }
            }
        }
    }

    template<PureEnumClassType T, auto Min, auto Max, typename F>
    void enum_switch(F&& f, T e) {
        using result_type = decltype(internal::enum_holder_query<T, Max, Min, Min, Max>());
        using holder_type = std::decay_t<result_type>;

        auto idx = holder_type::index(e);
        if (!idx) return;

        internal::enum_switch_imple<T, 0, holder_type, F>(std::forward<F>(f), idx.value());
    }

    namespace internal {
        template<PureEnumClassType T, size_t I, typename HolderT, typename F>
        void enum_foreach_imple(F&& f) {
            if constexpr (I < HolderT::_values_size) {
                f.template operator()<HolderT::_values[I]>();
                enum_foreach_imple<T, I + 1, HolderT, F>(std::forward<F>(f));
            }
        }
    }

    template<PureEnumClassType T, auto Min, auto Max, typename F>
    void enum_foreach(F&& f) {
        using result_type = decltype(internal::enum_holder_query<T, Max, Min, Min, Max>());
        using holder_type = std::decay_t<result_type>;

        internal::enum_foreach_imple<T, 0, holder_type, F>(std::forward<F>(f));
    }
}