#pragma once
#include <vector>
#include <tuple>
#include <functional>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstddef>

#include "utils/concept.hpp"
#include "ecs/querier.hpp"

namespace mytho::ecs {
    namespace internal {
        template<typename RegistryT>
        class basic_command_queue final {
        public:
            using registry_type = RegistryT;
            using entity_type = typename registry_type::entity_type;
            using executors_type = std::vector<void(*)(registry_type&, void*)>;
            using destroyers_type = std::vector<void(*)(void*)>;
            using buffers_type = std::vector<uint8_t>;
            using offsets_type = std::vector<size_t>;

        public:
            template<mytho::utils::PureComponentType... Ts>
            void spawn(Ts&&... ts) {
                using tuple_type = std::tuple<Ts...>;

                _executors.push_back([](registry_type& reg, void* ptr) {
                    auto* data = static_cast<tuple_type*>(ptr);
                    std::apply([&reg](auto&&... ts) {
                        reg.spawn(std::move(ts)...);
                    }, std::move(*data));
                });

                if constexpr (std::is_trivially_destructible_v<tuple_type>) {
                    _destroyers.push_back(nullptr);
                } else {
                    _destroyers.push_back([](void* ptr) {
                        static_cast<tuple_type*>(ptr)->~tuple_type();
                    });
                }

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(tuple_type) - 1) & ~(alignof(tuple_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(tuple_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) tuple_type(std::forward<Ts>(ts)...);
            }

            void despawn(const entity_type& e) {
                _executors.push_back([](registry_type& reg, void* ptr) {
                    reg.despawn(*static_cast<entity_type*>(ptr));
                });

                _destroyers.push_back(nullptr);

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(entity_type) - 1) & ~(alignof(entity_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(entity_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) entity_type(e);
            }

            template<mytho::utils::PureComponentType... Ts>
            void insert(const entity_type& e, Ts&&... ts) {
                using tuple_type = std::tuple<entity_type, Ts...>;

                _executors.push_back([](registry_type& reg, void* ptr) {
                    auto* data = static_cast<tuple_type*>(ptr);
                    std::apply([&reg](auto&&... args) {
                        reg.insert(std::move(args)...);
                    }, std::move(*data));
                });

                if constexpr (std::is_trivially_destructible_v<tuple_type>) {
                    _destroyers.push_back(nullptr);
                } else {
                    _destroyers.push_back([](void* ptr) {
                        static_cast<tuple_type*>(ptr)->~tuple_type();
                    });
                }

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(tuple_type) - 1) & ~(alignof(tuple_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(tuple_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) tuple_type(e, std::forward<Ts>(ts)...);
            }

            template<mytho::utils::PureComponentType... Ts>
            void remove(const entity_type& e) {
                _executors.push_back([](registry_type& reg, void* ptr) {
                    reg.template remove<Ts...>(*static_cast<entity_type*>(ptr));
                });

                _destroyers.push_back(nullptr);

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(entity_type) - 1) & ~(alignof(entity_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(entity_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) entity_type(e);
            }

            template<mytho::utils::PureComponentType... Ts>
            void replace(const entity_type& e, Ts&&... ts) {
                using tuple_type = std::tuple<entity_type, Ts...>;

                _executors.push_back([](registry_type& reg, void* ptr) {
                    auto* data = static_cast<tuple_type*>(ptr);
                    std::apply([&reg](auto&&... args) {
                        reg.replace(std::move(args)...);
                    }, std::move(*data));
                });

                if constexpr (std::is_trivially_destructible_v<tuple_type>) {
                    _destroyers.push_back(nullptr);
                } else {
                    _destroyers.push_back([](void* ptr) {
                        static_cast<tuple_type*>(ptr)->~tuple_type();
                    });
                }

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(tuple_type) - 1) & ~(alignof(tuple_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(tuple_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) tuple_type(e, std::forward<Ts>(ts)...);
            }

            template<typename T, typename... Rs>
            void init_resource(Rs&&... rs) {
                using tuple_type = std::tuple<Rs...>;

                _executors.push_back([](registry_type& reg, void* ptr) {
                    auto* data = static_cast<tuple_type*>(ptr);
                    std::apply([&reg](auto&&... rs) {
                        reg.template init_resource<T>(std::move(rs)...);
                    }, std::move(*data));
                });

                if constexpr (std::is_trivially_destructible_v<tuple_type>) {
                    _destroyers.push_back(nullptr);
                } else {
                    _destroyers.push_back([](void* ptr) {
                        static_cast<tuple_type*>(ptr)->~tuple_type();
                    });
                }

                size_t cur_buffer_size = _buffers.size();
                size_t cur_aligned_size = (cur_buffer_size + alignof(tuple_type) - 1) & ~(alignof(tuple_type) - 1);

                _buffers.resize(cur_aligned_size + sizeof(tuple_type));
                _offsets.push_back(cur_aligned_size);

                new (_buffers.data() + cur_aligned_size) tuple_type(std::forward<Rs>(rs)...);
            }

            template<typename T>
            void remove_resource() {
                _executors.push_back([](registry_type& reg, void* ptr) {
                    reg.template remove_resource<T>();
                });

                _destroyers.push_back(nullptr);

                _offsets.push_back(0);
            }

        public:
            void apply(registry_type& reg) {
                auto size = _executors.size();
                for (size_t i = 0; i < size; i++) {
                    _executors[i](reg, _buffers.data() + _offsets[i]);

                    if (_destroyers[i]) {
                        _destroyers[i](_buffers.data() + _offsets[i]);
                    }
                }

                clear();
            }

            void clear() noexcept {
                _executors.clear();
                _destroyers.clear();
                _buffers.clear();
                _offsets.clear();
            }

        private:
            executors_type _executors;
            destroyers_type _destroyers;
            buffers_type _buffers;
            offsets_type _offsets;
        };
    }

    template<typename RegistryT>
    class basic_commands final {
    public:
        using registry_type = RegistryT;
        using entity_type = typename registry_type::entity_type;

    public:
        basic_commands(registry_type& reg) : _reg(reg) {}

    public:
        template<mytho::utils::PureComponentType... Ts>
        void spawn(Ts&&... ts) {
            _reg.command_queue().spawn(std::forward<Ts>(ts)...);
        }

        void despawn(const entity_type& e) {
            _reg.command_queue().despawn(e);
        }

    public:
        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void insert(const entity_type& e, Ts&&... ts) {
            _reg.command_queue().insert(e, std::forward<Ts>(ts)...);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void remove(const entity_type& e) {
            _reg.command_queue().template remove<Ts...>(e);
        }

        template<mytho::utils::PureComponentType... Ts>
        requires (sizeof...(Ts) > 0)
        void replace(const entity_type& e, Ts&&... ts) {
            _reg.command_queue().replace(e, std::forward<Ts>(ts)...);
        }

    public:
        template<typename T, typename... Rs>
        void init_resource(Rs&&... rs) {
            _reg.command_queue().template init_resource<T>(std::forward<Rs>(rs)...);
        }

        template<typename T>
        void remove_resource() {
            _reg.command_queue().template remove_resource<T>();
        }

    private:
        registry_type& _reg;
    };
}

namespace mytho::utils {
    template<typename T>
    inline constexpr bool is_commands_v = internal::is_template_v<T, mytho::ecs::basic_commands>;
}