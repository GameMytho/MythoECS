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

struct Name {
    std::string value;
};

struct Health {
    uint32_t value;
};

template<typename... Ts>
using mut = mytho::ecs::mut<Ts...>;

template<typename... Ts>
using with = mytho::ecs::with<Ts...>;

template<typename... Ts>
using without = mytho::ecs::without<Ts...>;

template<typename... Ts>
using changed = mytho::ecs::changed<Ts...>;

using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
using registry = mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024>;
using commands = mytho::ecs::basic_commands<registry>;

template<typename... Ts>
using querier = mytho::ecs::basic_querier<registry, Ts...>;

template<typename T>
using data_wrapper = mytho::utils::internal::data_wrapper<T>;

void startup(commands cmds) {
    cmds.spawn(Position{0.1f, 0.1f});
    cmds.spawn(Position{0.2f, 0.2f}, Vectory{0.2f, 0.2f});
    cmds.spawn(Position{0.3f, 0.3f}, Vectory{0.3f, 0.3f}, Direction{0.3f, 0.3f});
}

void update1(querier<entity, Position, with<Vectory>, without<Direction>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [entt, pos] : q) {
        using entt_type = decltype(entt);
        EXPECT_EQ((std::is_same_v<entt_type, const data_wrapper<entity>>), true);
        EXPECT_EQ(entt->id(), 1);
        EXPECT_EQ(entt->version(), 0);

        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.2f);
        EXPECT_EQ(pos->y, 0.2f);
    }
}

void update2(querier<mut<Position>, Vectory, with<Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos, vec] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.3f);
        EXPECT_EQ(pos->y, 0.3f);

        pos->x = 0.4;
        pos->y = 0.4;

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.3f);
        EXPECT_EQ(vec->y, 0.3f);
    }
}

void update2_changed(querier<Position, changed<Position>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.4f);
        EXPECT_EQ(pos->y, 0.4f);
    }
}

void update3(querier<Position, mut<Vectory>, with<Vectory, Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [pos, vec] : q) {
        using pos_type = decltype(pos);
        EXPECT_EQ((std::is_same_v<pos_type, const data_wrapper<Position>>), true);
        EXPECT_EQ(pos->x, 0.4f);
        EXPECT_EQ(pos->y, 0.4f);

        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.3f);
        EXPECT_EQ(vec->y, 0.3f);

        vec->x *= 2;
        vec->y *= 2;
    }
}

void update3_changed(querier<Vectory, changed<Vectory>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [vec] : q) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.6f);
        EXPECT_EQ(vec->y, 0.6f);
    }
}

void update4(querier<Vectory, with<Position, Direction>, without<Name, Health>> q) {
    EXPECT_EQ(q.size(), 1);

    for (auto [vec] : q) {
        using vec_type = decltype(vec);
        EXPECT_EQ((std::is_same_v<vec_type, const data_wrapper<Vectory>>), true);
        EXPECT_EQ(vec->x, 0.6f);
        EXPECT_EQ(vec->y, 0.6f);
    }
}

void shutdown() {

}

TEST(SystemTest, AddAndRemoveTest) {
    registry reg;

    reg.add_startup_system<startup>()
       .add_update_system<update1>()
       .add_update_system<update2>()
       .add_shutdown_system<shutdown>();

    reg.startup();

    reg.update();

    reg.remove_update_system<update1>()
       .remove_update_system<update2>()
       .add_update_system<update3>();

    reg.update();

    reg.shutdown();
}

TEST(SystemTest, EnableAndDisableTest) {
    registry reg;

    reg.add_startup_system<startup>()
       .add_update_system<update1>()
       .add_update_system<update2>()
       .add_update_system<update3>()
       .add_shutdown_system<shutdown>();

    reg.enable_update_system<update1>()
       .enable_update_system<update2>()
       .disable_update_system<update3>();

    reg.startup();

    reg.update();

    reg.disable_update_system<update1>()
       .disable_update_system<update2>()
       .enable_update_system<update3>();

    reg.update();

    reg.shutdown();
}

template<auto... Funcs>
using before = mytho::ecs::before<Funcs...>;

template<auto... Funcs>
using after = mytho::ecs::after<Funcs...>;

TEST(SystemTest, SystemLocationTest) {
    registry reg;

    reg.add_startup_system<startup>()
       .add_update_system<update4>()
       .add_update_system<update1, before<update4>>()
       .add_update_system<update3, after<update1>, before<update4>>()
       .add_update_system<update2, before<update3, update4>, after<update1>>()
       .remove_update_system<update4>()
       .add_update_system<update4, after<update1, update2, update3>>()
       .remove_update_system<update1>()
       .add_update_system<update1, before<update2, update3, update4>>()
       .add_shutdown_system<shutdown>();

    reg.enable_update_system<update1>()
       .enable_update_system<update2>()
       .disable_update_system<update3>()
       .disable_update_system<update4>();

    reg.ready();

    reg.startup();

    reg.update();

    reg.disable_update_system<update1>()
       .disable_update_system<update2>()
       .enable_update_system<update3>()
       .enable_update_system<update4>();

    reg.update();

    reg.shutdown();
}

TEST(SystemTest, ComponentChangedTest) {
    registry reg;

    reg.add_startup_system<startup>()
       .add_update_system<update1>()
       .add_update_system<update2>()
       .add_update_system<update2_changed>()
       .add_update_system<update3>()
       .add_update_system<update3_changed>()
       .add_update_system<update4>()
       .add_shutdown_system<shutdown>();

    reg.ready();

    reg.startup();

    reg.update();

    reg.shutdown();
}