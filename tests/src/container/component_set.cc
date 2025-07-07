#include <gtest/gtest.h>
#include <container/component_set.hpp>

struct Position {
    float x;
    float y;
};

TEST(ComponentSetTest, BasicAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_component_set<entity, Position, std::allocator<Position>> s;

    entity e(10);

    s.add(e, 0.5f, 5.0f);
    EXPECT_EQ(s.contain(e), true);
    EXPECT_EQ(s.get(e).x, 0.5f);
    EXPECT_EQ(s.get(e).y, 5.0f);

    s.replace(e, 0, 0.6f, 6.0f);
    EXPECT_EQ(s.contain(e), true);
    EXPECT_EQ(s.get(e).x, 0.6f);
    EXPECT_EQ(s.get(e).y, 6.0f);

    s.remove(e);
    EXPECT_EQ(s.contain(e), false);
}

TEST(ComponentSetTest, MultipleAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_component_set<entity, Position, std::allocator<Position>> s;

    for (int i = 0; i < 100; i++) {
        entity e(i * 10);
        s.add(e, i * 0.5f, i * 5.0f);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.get(e).x, i * 0.5f);
        EXPECT_EQ(s.get(e).y, i * 5.0f);
    }

    for (int i = 0; i < 100; i++) {
        entity e(i * 10);
        s.replace(e, i, i * 0.6f, i * 6.0f);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.get(e).x, i * 0.6f);
        EXPECT_EQ(s.get(e).y, i * 6.0f);
    }

    for (int i = 0; i < 100; i++) {
        entity e(i * 10);
        s.remove(e);
        EXPECT_EQ(s.contain(e), false);
    }
}

TEST(ComponentSetTest, FrequentlyAddAndRemove) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_component_set<entity, Position, std::allocator<Position>> s;

    for (int i = 0; i < 100; i++) {
        entity e(i * 10);
        s.add(e, i * 0.5f, i * 5.0f);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.get(e).x, i * 0.5f);
        EXPECT_EQ(s.get(e).y, i * 5.0f);

        s.replace(e, i, i * 0.6f, i * 6.0f);
        EXPECT_EQ(s.contain(e), true);
        EXPECT_EQ(s.get(e).x, i * 0.6f);
        EXPECT_EQ(s.get(e).y, i * 6.0f);
        
        s.remove(e);
        EXPECT_EQ(s.contain(e), false);
    }
}