#include <gtest/gtest.h>
#include <container/entity_storage.hpp>

struct Position {
    float x;
    float y;
};

struct Vectory {
    float x;
    float y;
};

TEST(EntityStorageTest, BasicTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    EXPECT_EQ(s.size(), 0);

    entity e0 = s.emplace();
    EXPECT_EQ(e0.id(), 0);
    EXPECT_EQ(e0.version(), 0);
    EXPECT_EQ(s.contain(e0), true);
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.has<Position>(e0), false);

    s.add<Position>(e0);
    EXPECT_EQ(s.has<Position>(e0), true);
    EXPECT_EQ((s.has<Position, Vectory>(e0)), false);

    s.add<Vectory>(e0);
    EXPECT_EQ((s.has<Position, Vectory>(e0)), true);

    s.remove<Position>(e0);
    EXPECT_EQ(s.has<Position>(e0), false);
    EXPECT_EQ(s.has<Vectory>(e0), true);

    s.remove<Vectory>(e0);
    EXPECT_EQ(s.has<Position>(e0), false);
    EXPECT_EQ(s.has<Vectory>(e0), false);

    entity e1 = s.emplace<Position, Vectory>();
    EXPECT_EQ(e1.id(), 1);
    EXPECT_EQ(e1.version(), 0);
    EXPECT_EQ(s.contain(e1), true);
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ((s.has<Position, Vectory>(e1)), true);
    EXPECT_EQ(s.has<Position>(e1), true);
    EXPECT_EQ(s.has<Vectory>(e1), true);

    s.remove<Vectory>(e1);
    EXPECT_EQ((s.has<Position, Vectory>(e1)), false);
    EXPECT_EQ(s.has<Position>(e1), true);
    EXPECT_EQ(s.has<Vectory>(e1), false);

    s.remove<Position>(e1);
    EXPECT_EQ((s.has<Position, Vectory>(e1)), false);
    EXPECT_EQ(s.has<Position>(e1), false);
    EXPECT_EQ(s.has<Vectory>(e1), false);

    s.remove(e0);
    EXPECT_EQ(s.contain(e0), false);
    EXPECT_EQ(s.size(), 1);

    entity e3 = s.emplace();
    EXPECT_EQ(e3.id(), 0);
    EXPECT_EQ(e3.version(), 1);
    EXPECT_EQ(s.contain(e3), true);
    EXPECT_EQ(s.size(), 2);
    EXPECT_EQ((s.has<Position, Vectory>(e3)), false);
    EXPECT_EQ(s.has<Position>(e3), false);
    EXPECT_EQ(s.has<Vectory>(e3), false);

    s.remove(e1);
    EXPECT_EQ(s.contain(e1), false);
    EXPECT_EQ(s.size(), 1);

    s.remove(e3);
    EXPECT_EQ(s.contain(e3), false);
    EXPECT_EQ(s.size(), 0);
}