#include <gtest/gtest.h>
#include <container/component_storage.hpp>

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

TEST(ComponentStorageTest, BasicAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_component_storage<entity> s;

    entity e(10);

    s.add(e, Position{0.5f, 5.0f});
    EXPECT_EQ(s.contain<Position>(e), true);
    {
        auto [pos] = s.get<Position>(e);
        EXPECT_EQ((pos.x), 0.5f);
        EXPECT_EQ((pos.y), 5.0f);
    }

    s.add(e, Vectory{0.6f, 6.0f}, Direction{1.5f, 2.5f});
    EXPECT_EQ((s.contain<Position, Vectory, Direction>(e)), true);
    {
        auto [pos, vec, dir] = s.get<Position, Vectory, Direction>(e);
        EXPECT_EQ((pos.x), 0.5f);
        EXPECT_EQ((pos.y), 5.0f);
        EXPECT_EQ((vec.x), 0.6f);
        EXPECT_EQ((vec.y), 6.0f);
        EXPECT_EQ((dir.x), 1.5f);
        EXPECT_EQ((dir.y), 2.5f);
    }

    s.replace(e, 0, Position{1.0f, 1.0f}, Vectory{1.2f, 12.0f}, Direction{3.0f, 5.2f});
    EXPECT_EQ((s.contain<Position, Vectory, Direction>(e)), true);
    {
        auto [pos, vec, dir] = s.get<Position, Vectory, Direction>(e);
        EXPECT_EQ((pos.x), 1.0f);
        EXPECT_EQ((pos.y), 1.0f);
        EXPECT_EQ((vec.x), 1.2f);
        EXPECT_EQ((vec.y), 12.0f);
        EXPECT_EQ((dir.x), 3.0f);
        EXPECT_EQ((dir.y), 5.2f);
    }

    s.remove<Position>(e);
    EXPECT_EQ((s.contain<Position, Vectory, Direction>(e)), false);
    EXPECT_EQ((s.contain<Vectory, Direction>(e)), true);
    {
        auto [vec, dir] = s.get<Vectory, Direction>(e);
        EXPECT_EQ((vec.x), 1.2f);
        EXPECT_EQ((vec.y), 12.0f);
        EXPECT_EQ((dir.x), 3.0f);
        EXPECT_EQ((dir.y), 5.2f);
    }

    s.remove<Vectory, Direction>(e);
    EXPECT_EQ(s.contain<Position>(e), false);
    EXPECT_EQ(s.contain<Vectory>(e), false);
    EXPECT_EQ(s.contain<Direction>(e), false);
}