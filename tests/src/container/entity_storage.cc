#include <gtest/gtest.h>
#include <container/entity_storage.hpp>

TEST(EntityStorageTest, BasicEntitySpawnAndDespawn) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    EXPECT_EQ(s.size(), 0);

    entity e = s.emplace();
    EXPECT_EQ(e.id(), 0);
    EXPECT_EQ(e.version(), 0);
    EXPECT_EQ(s.contain(e), true);
    EXPECT_EQ(s.size(), 1);

    s.pop(e);
    EXPECT_EQ(s.contain(e), false);
    EXPECT_EQ(s.size(), 0);
}

TEST(EntityStorageTest, MultipleEntitiesSpawnAndDespawn) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    EXPECT_EQ(s.size(), 0);
    std::vector<entity> v;

    for (int i = 0; i < 100; i++) {
        entity e = s.emplace();
        EXPECT_EQ(e.id(), i);
        EXPECT_EQ(e.version(), 0);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.size(), i + 1);

        v.push_back(e);
    }

    for (int i = 0; i < 100; i++) {
        s.pop(v[i]);
        EXPECT_EQ(s.contain(v[i]), false);
        EXPECT_EQ(s.size(), 100 - i - 1);
    }
}

TEST(EntityStorageTest, FrequentlyEntitiesSpawnAndDespawn) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    for (int i = 0; i < 100; i++) {
        entity e = s.emplace();
        EXPECT_EQ(e.id(), 0);
        EXPECT_EQ(e.version(), i);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.size(), 1);

        s.pop(e);
        EXPECT_EQ(s.contain(e), false);
        EXPECT_EQ(s.size(), 0);
    }
}

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

TEST(EntityStorageTest, BasicComponentsAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    entity e = s.emplace<Position>();
    EXPECT_EQ(e.id(), 0);
    EXPECT_EQ(e.version(), 0);
    EXPECT_EQ(s.contain(e), true);
    EXPECT_EQ(s.has<Position>(e), true);
    EXPECT_EQ(s.has<Vectory>(e), false);
    EXPECT_EQ(s.has<Direction>(e), false);

    s.add<Vectory>(e);
    EXPECT_EQ((s.has<Position, Vectory>(e)), true);
    EXPECT_EQ(s.has<Direction>(e), false);

    s.add<Direction>(e);
    EXPECT_EQ((s.has<Position, Vectory, Direction>(e)), true);

    s.remove<Position>(e);
    EXPECT_EQ((s.has<Vectory, Direction>(e)), true);
    EXPECT_EQ(s.has<Position>(e), false);

    s.remove<Vectory>(e);
    EXPECT_EQ(s.has<Direction>(e), true);
    EXPECT_EQ(s.has<Position>(e), false);
    EXPECT_EQ(s.has<Vectory>(e), false);

    s.remove<Direction>(e);
    EXPECT_EQ(s.has<Position>(e), false);
    EXPECT_EQ(s.has<Vectory>(e), false);
    EXPECT_EQ(s.has<Direction>(e), false);
}

TEST(EntityStorageTest, FrequentlyComponentsAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    entity e = s.emplace();
    EXPECT_EQ(s.has<Position>(e), false);
    EXPECT_EQ(s.has<Vectory>(e), false);
    EXPECT_EQ(s.has<Direction>(e), false);

    for (int i = 0; i < 100; i++) {
        s.add<Position>(e);
        EXPECT_EQ(s.has<Position>(e), true);
        EXPECT_EQ(s.has<Vectory>(e), false);
        EXPECT_EQ(s.has<Direction>(e), false);

        s.add<Vectory>(e);
        EXPECT_EQ((s.has<Position, Vectory>(e)), true);
        EXPECT_EQ(s.has<Direction>(e), false);

        s.add<Direction>(e);
        EXPECT_EQ((s.has<Position, Vectory, Direction>(e)), true);

        s.remove<Position>(e);
        EXPECT_EQ((s.has<Vectory, Direction>(e)), true);
        EXPECT_EQ(s.has<Position>(e), false);

        s.remove<Vectory>(e);
        EXPECT_EQ(s.has<Direction>(e), true);
        EXPECT_EQ(s.has<Position>(e), false);
        EXPECT_EQ(s.has<Vectory>(e), false);

        s.remove<Direction>(e);
        EXPECT_EQ(s.has<Position>(e), false);
        EXPECT_EQ(s.has<Vectory>(e), false);
        EXPECT_EQ(s.has<Direction>(e), false);

        s.add<Position, Vectory, Direction>(e);
        EXPECT_EQ((s.has<Position, Vectory, Direction>(e)), true);

        s.remove<Position, Vectory, Direction>(e);
        EXPECT_EQ((s.has<Position, Vectory, Direction>(e)), false);
    }
}

TEST(EntityStorageTest, RepeatComponentsAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    entity e = s.emplace();
    EXPECT_EQ(s.has<Position>(e), false);

    s.add<Position>(e);
    EXPECT_EQ(s.has<Position>(e), true);

    s.add<Position>(e);
    EXPECT_EQ(s.has<Position>(e), true);

    s.remove<Position>(e);
    EXPECT_EQ(s.has<Position>(e), false);

    s.remove<Position>(e);
    EXPECT_EQ(s.has<Position>(e), false);
}