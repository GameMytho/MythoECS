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

using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
using registry = mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024>;
using commands = mytho::ecs::basic_commands<registry>;

template<typename... Ts>
using querier = mytho::ecs::basic_querier<registry, Ts...>;

void startup(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    cmds.spawn(Position{1.0f, 2.0f}, Vectory{3.0f, 4.0f}, Direction{5.0f, 6.0f});

    EXPECT_EQ(q.size(), 0);
}

void update(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    cmds.spawn(Position{10.0f, 20.0f}, Vectory{30.0f, 40.0f}, Direction{50.0f, 60.0f});

    EXPECT_EQ(q.size(), 1);

    for (auto [e, pos, vec, dir] : q) {
        EXPECT_EQ(e.id(), 0);
        EXPECT_EQ(e.version(), 0);
        EXPECT_EQ(pos.x, 1.0f);
        EXPECT_EQ(pos.y, 2.0f);
        EXPECT_EQ(vec.x, 3.0f);
        EXPECT_EQ(vec.y, 4.0f);
        EXPECT_EQ(dir.x, 5.0f);
        EXPECT_EQ(dir.y, 6.0f);
    }
}

void shutdown(commands cmds, querier<entity, Position, Vectory, Direction> q) {
    EXPECT_EQ(q.size(), 2);

    for (auto [e, pos, vec, dir] : q) {
        if (e.id() == 0) {
            EXPECT_EQ(e.version(), 0);
            EXPECT_EQ(pos.x, 1.0f);
            EXPECT_EQ(pos.y, 2.0f);
            EXPECT_EQ(vec.x, 3.0f);
            EXPECT_EQ(vec.y, 4.0f);
            EXPECT_EQ(dir.x, 5.0f);
            EXPECT_EQ(dir.y, 6.0f);
        } else if (e.id() == 1) {
            EXPECT_EQ(e.version(), 0);
            EXPECT_EQ(pos.x, 10.0f);
            EXPECT_EQ(pos.y, 20.0f);
            EXPECT_EQ(vec.x, 30.0f);
            EXPECT_EQ(vec.y, 40.0f);
            EXPECT_EQ(dir.x, 50.0f);
            EXPECT_EQ(dir.y, 60.0f);
        } else {
            FAIL() << "This should not be reached";
        }
    }
}

TEST(CommandsTest, CommandsQueueTest) {
    registry reg;

    reg.add_startup_system<startup>();
    reg.add_update_system<update>();
    reg.add_shutdown_system<shutdown>();

    reg.startup();
    reg.update();
    reg.shutdown();
}