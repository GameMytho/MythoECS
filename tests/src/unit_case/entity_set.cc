#include <random>
#include <gtest/gtest.h>
#include <container/entity_set.hpp>

TEST(EntitySetTest, BasicAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    EXPECT_EQ(s.contain(0), false);
    EXPECT_EQ(s.size(), 0);

    s.add(0);
    EXPECT_EQ(s.contain(0), true);
    EXPECT_EQ(s.size(), 1);
    EXPECT_EQ(s.index(0), 0);

    s.remove(0);
    EXPECT_EQ(s.contain(0), false);
    EXPECT_EQ(s.size(), 0);
}

TEST(EntitySetTest, MultipleAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(s.contain(i * 10), false);
    }
    EXPECT_EQ(s.size(), 0);

    for (int i = 0; i < 1024; i++) {
        s.add(i * 10);
        EXPECT_EQ(s.contain(i * 10), true);
        EXPECT_EQ(s.index(i * 10), i);
    }
    EXPECT_EQ(s.size(), 1024);

    for (int i = 0; i < 1024; i++) {
        s.remove(i * 10);
        EXPECT_EQ(s.contain(i * 10), false);
        EXPECT_EQ(s.size(), 1024 - i - 1);
    }
}

TEST(EntitySetTest, FrequentlyAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    for (int i = 0; i < 1024; i++) {
        EXPECT_EQ(s.contain(i * 10), false);
    }
    EXPECT_EQ(s.size(), 0);

    for (int i = 0; i < 1024; i++) {
        s.add(i * 10);
        EXPECT_EQ(s.contain(i * 10), true);
        EXPECT_EQ(s.index(i * 10), 0);

        s.remove(i * 10);
        EXPECT_EQ(s.contain(i * 10), false);
        EXPECT_EQ(s.size(), 0);
    }
}

TEST(EntitySetTest, RandomAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    EXPECT_EQ(s.size(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 102400);

    for (int i = 0; i < 100; i++) {
        int r = dist(gen);
        s.add(r);
        EXPECT_EQ(s.contain(r), true);
        EXPECT_EQ(s.index(r), 0);

        s.remove(r);
        EXPECT_EQ(s.contain(r), false);
        EXPECT_EQ(s.size(), 0);
    }
}

TEST(EntitySetTest, RepeatAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    EXPECT_EQ(s.size(), 0);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 102400);
    int r = dist(gen);

    for (int i = 0; i < 100; i++) {
        s.add(r);
        EXPECT_EQ(s.contain(r), true);
        EXPECT_EQ(s.index(r), 0);

        s.remove(r);
        EXPECT_EQ(s.contain(r), false);
        EXPECT_EQ(s.size(), 0);
    }
}
