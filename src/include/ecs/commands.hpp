#pragma once

namespace mytho::ecs {
    template<typename RegistryT>
    class basic_commands final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;

    public:
        basic_commands(registry_type& reg) : _reg(reg) {}

    public:
        template<mytho::utils::PureValueType... Ts>
        entity_type spawn(Ts&&... ts) noexcept {
            return _reg.spawn(std::forward<Ts>(ts)...);
        }

        void despawn(const entity_type& e) noexcept {
            _reg.despawn(e);
        }

    public:
        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) noexcept {
            _reg.insert(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) noexcept {
            _reg.template remove<Ts...>(e);
        }

        template<mytho::utils::PureValueType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) noexcept {
            _reg.replace(e, std::forward<Ts>(ts)...);
        }

    private:
        registry_type& _reg;        
    };
}