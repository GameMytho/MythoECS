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

TEST(CommandsTest, BasicTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    using registry = mytho::ecs::basic_registry<entity, uint8_t, uint8_t, 1024>;
    using commands = mytho::ecs::basic_commands<registry>;

    registry reg;
    commands cmds(reg);

    auto e = cmds.spawn();
    EXPECT_EQ(e.id(), 0);
    EXPECT_EQ(e.version(), 0);

    cmds.insert(e, Position{0.1f, 1.1f});
    EXPECT_EQ(reg.contain<Position>(e), true);
    {
        auto [pos] = reg.get<Position>(e);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);
    }
    
    cmds.insert(e, Vectory{1.0f, 2.1f}, Direction{3.0f, 0.3f});
    EXPECT_EQ((reg.contain<Position, Vectory, Direction>(e)), true);
    {
        auto [pos, vec, dir] = reg.get<Position, Vectory, Direction>(e);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);
        EXPECT_EQ(vec.x, 1.0f);
        EXPECT_EQ(vec.y, 2.1f);
        EXPECT_EQ(dir.x, 3.0f);
        EXPECT_EQ(dir.y, 0.3f);
    }

    cmds.remove<Vectory>(e);
    EXPECT_EQ(reg.contain<Vectory>(e), false);
    EXPECT_EQ((reg.contain<Position, Direction>(e)), true);
    {
        auto [pos, dir] = reg.get<Position, Direction>(e);
        EXPECT_EQ(pos.x, 0.1f);
        EXPECT_EQ(pos.y, 1.1f);
        EXPECT_EQ(dir.x, 3.0f);
        EXPECT_EQ(dir.y, 0.3f);
    }

    cmds.replace(e, Position{0.2f, 1.2f}, Direction{3.1f, 0.4f});
    EXPECT_EQ((reg.contain<Position, Direction>(e)), true);
    {
        auto [pos, dir] = reg.get<Position, Direction>(e);
        EXPECT_EQ(pos.x, 0.2f);
        EXPECT_EQ(pos.y, 1.2f);
        EXPECT_EQ(dir.x, 3.1f);
        EXPECT_EQ(dir.y, 0.4f);
    }

    cmds.despawn(e);
}