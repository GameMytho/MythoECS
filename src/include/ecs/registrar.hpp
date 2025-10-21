#pragma once
#include "ecs/querier.hpp"
#include "ecs/resources.hpp"

namespace mytho::ecs {
    template<typename RegistryT>
    class basic_registrar {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;
        using size_type = typename registry_type::size_type;

        basic_registrar(registry_type& reg, uint64_t tick) : _reg(reg), _last_run_tick(tick) {}

    public:
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        auto get(const entity_type& e) const noexcept {
            return _reg.template get<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool contain(const entity_type& e) const noexcept {
            return _reg.template contain<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_added() const noexcept {
            return _reg.template components_added<Ts...>(_last_run_tick);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_changed() const noexcept {
            return _reg.template components_changed<Ts...>(_last_run_tick);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        bool components_removed() noexcept {
            return _reg.template components_removed<Ts...>();
        }

        template<mytho::utils::QueryValueType... Ts>
        requires (sizeof...(Ts) > 0)
        size_type count() const noexcept {
            return _reg.template count<Ts...>(_last_run_tick);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_exist() const noexcept {
            return _reg.template resources_exist<Ts...>();
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_added() const noexcept {
            return _reg.template resources_added<Ts...>(_last_run_tick);
        }

        template<mytho::utils::PureResourceType... Ts>
        requires (sizeof...(Ts) > 0)
        bool resources_changed() const noexcept {
            return _reg.template resources_changed<Ts...>(_last_run_tick);
        }
    private:
        registry_type& _reg;
        uint64_t _last_run_tick;
    };
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_registrar_v = internal::is_template_v<T, mytho::ecs::basic_registrar>;
}