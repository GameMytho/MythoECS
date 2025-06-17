#pragma once
#include "ecs/querier.hpp"

namespace mytho::ecs {
    template<typename RegistryT>
    class basic_commands final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;

    public:
        basic_commands(registry_type& reg) : _reg(reg) {}

    public:
        template<mytho::utils::PureComponentType... Ts>
        entity_type spawn(Ts&&... ts) noexcept {
            return _reg.spawn(std::forward<Ts>(ts)...);
        }

        void despawn(const entity_type& e) noexcept {
            _reg.despawn(e);
        }

    public:
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) noexcept {
            _reg.insert(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            _reg.template remove<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            _reg.replace(e, std::forward<Ts>(ts)...);
        }

    public:
        template<typename T, typename... Rs>
        void init_resource(Rs&&... rs) noexcept {
            _reg.template init_resource<T>(std::forward<Rs>(rs)...);
        }

        template<typename T>
        void remove_resource() noexcept {
            _reg.template remove_resource<T>();
        }

    private:
        registry_type& _reg;
    };
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_commands_v = internal::is_template_v<T, mytho::ecs::basic_commands>;
}