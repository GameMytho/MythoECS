#include <gtest/gtest.h>
#include <container/entity_storage.hpp>

TEST(EntityStorageTest, BasicTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_storage<entity> s;

    EXPECT_EQ(s.size(), 0);

    entity e0 = s.emplace();
    EXPECT_EQ(e0.id(), 0);
    EXPECT_EQ(e0.version(), 0);
    EXPECT_EQ(s.contain(e0), true);
    EXPECT_EQ(s.size(), 1);

    entity e1 = s.emplace();
    EXPECT_EQ(e1.id(), 1);
    EXPECT_EQ(e1.version(), 0);
    EXPECT_EQ(s.contain(e1), true);
    EXPECT_EQ(s.size(), 2);

    s.remove(e0);
    EXPECT_EQ(s.contain(e0), false);
    EXPECT_EQ(s.size(), 1);

    entity e3 = s.emplace();
    EXPECT_EQ(e3.id(), 0);
    EXPECT_EQ(e3.version(), 1);
    EXPECT_EQ(s.contain(e3), true);
    EXPECT_EQ(s.size(), 2);

    s.remove(e1);
    EXPECT_EQ(s.contain(e1), false);
    EXPECT_EQ(s.size(), 1);

    s.remove(e3);
    EXPECT_EQ(s.contain(e3), false);
    EXPECT_EQ(s.size(), 0);
}