#include <gtest/gtest.h>
#include "ecs/registry.hpp"

struct Position {
    float x;
    float y;
};

struct Vectory {
    float x;
    float y;
};

struct Direction {
    float x;
    float y;
};

TEST(RegistryTest, BasicTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024> reg;

    auto e = reg.spawn();
    EXPECT_EQ(e.id(), 0);
    EXPECT_EQ(e.version(), 0);

    reg.insert(e, Position{0.1f, 1.1f});
    EXPECT_EQ(reg.contain<Position>(e), true);
    {
        auto [pos] = reg.get<Position>(e);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const Position&>), true);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);
    }

    reg.insert(e, Vectory{1.0f, 2.1f}, Direction{3.0f, 0.3f});
    EXPECT_EQ((reg.contain<Position, Vectory, Direction>(e)), true);
    {
        auto [pos, vec, dir] = reg.get<Position, Vectory, Direction>(e);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const Position&>), true);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const Vectory&>), true);
        EXPECT_EQ(vec.x, 1.0f);
        EXPECT_EQ(vec.y, 2.1f);

        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, const Direction&>), true);
        EXPECT_EQ(dir.x, 3.0f);
        EXPECT_EQ(dir.y, 0.3f);
    }

    reg.remove<Vectory>(e);
    EXPECT_EQ(reg.contain<Vectory>(e), false);
    EXPECT_EQ((reg.contain<Position, Direction>(e)), true);
    {
        auto [pos, dir] = reg.get<Position, Direction>(e);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const Position&>), true);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);

        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, const Direction&>), true);
        EXPECT_EQ(dir.x, 3.0f);
        EXPECT_EQ(dir.y, 0.3f);
    }

    reg.replace(e, Position{0.2f, 1.2f}, Direction{3.1f, 0.4f});
    EXPECT_EQ((reg.contain<Position, Direction>(e)), true);
    {
        auto [pos, dir] = reg.get<Position, Direction>(e);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const Position&>), true);
        EXPECT_EQ(pos.x, 0.2f);
        EXPECT_EQ(pos.y, 1.2f);

        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, const Direction&>), true);
        EXPECT_EQ(dir.x, 3.1f);
        EXPECT_EQ(dir.y, 0.4f);
    }

    reg.despawn(e);
}

template<typename... Ts>
using mut = mytho::ecs::mut<Ts...>;

template<typename T>
using data_wrapper = mytho::utils::internal::data_wrapper<T>;

TEST(RegistryTest, QueryMutTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024> reg;

    for (int i = 0; i < 3; i++) {
        auto e = reg.spawn(Position{i * 0.1f, i * 0.1f}, Vectory{i * 0.2f, i * 0.2f}, Direction{i * 0.3f, i * 0.3f});
        EXPECT_EQ((reg.contain<Position, Vectory, Direction>(e)), true);
    }

    int i = 0;
    for (auto [pos, vec, dir] : reg.query<Position, mut<Vectory>, Direction>()) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, i * 0.1f);
        EXPECT_EQ(pos->y, i * 0.1f);

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, i * 0.2f);
        EXPECT_EQ(vec->y, i * 0.2f);
        vec->x *= 2;
        vec->y *= 2;

        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, const data_wrapper<Direction>>), true);
        EXPECT_EQ(dir->x, i * 0.3f);
        EXPECT_EQ(dir->y, i * 0.3f);

        i++;
    }

    i = 0;
    for (auto [vec] : reg.query<Vectory>()) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, i * 0.4f);
        EXPECT_EQ(vec->y, i * 0.4f);

        i++;
    }

    i = 0;
    for (auto [vec, dir] : reg.query<mut<Vectory, Direction>>()) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, i * 0.4f);
        EXPECT_EQ(vec->y, i * 0.4f);

        using dir_type = decltype(dir);
        EXPECT_EQ((std::is_same_v<dir_type, data_wrapper<Direction>>), true);
        EXPECT_EQ(dir->x, i * 0.3f);
        EXPECT_EQ(dir->y, i * 0.3f);

        i++;
    }
}

template<typename... Ts>
using with = mytho::ecs::with<Ts...>;

template<typename... Ts>
using without = mytho::ecs::without<Ts...>;

struct Name {
    std::string value;
};

struct Health {
    uint32_t value;
};

TEST(RegistryTest, QueryWithTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024> reg;

    auto e1 = reg.spawn(Position{0.1f, 0.1f});
    EXPECT_EQ((reg.contain<Position>(e1)), true);

    auto e2 = reg.spawn(Position{0.2f, 0.2f}, Vectory{0.2f, 0.2f});
    EXPECT_EQ((reg.contain<Position, Vectory>(e2)), true);

    auto e3 = reg.spawn(Position{0.3f, 0.3f}, Vectory{0.3f, 0.3f}, Direction{0.3f, 0.3f});
    EXPECT_EQ((reg.contain<Position, Vectory, Direction>(e3)), true);

    for (auto [pos] : reg.query<Position, with<Vectory>, without<Direction>>()) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.2f);
        EXPECT_EQ(pos->y, 0.2f);
    }

    for (auto [pos, vec] : reg.query<mut<Position, Vectory>, with<Direction>, without<Name, Health>>()) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.3f);
        EXPECT_EQ(pos->y, 0.3f);

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.3f);
        EXPECT_EQ(vec->y, 0.3f);
    }
}