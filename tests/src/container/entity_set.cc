#include <gtest/gtest.h>
#include <container/entity_set.hpp>

TEST(EntitySetTest, BasicTest) {
    using entity = mytho::ecs::basic_entity<uint32_t, uint8_t>;
    mytho::container::basic_entity_set<entity> s;

    s.add(100000);
    EXPECT_EQ(s.contain(100000), true);
    EXPECT_EQ(s.size(), 1);

    s.add(923424);
    EXPECT_EQ(s.contain(923424), true);
    EXPECT_EQ(s.size(), 2);

    s.add(8456);
    EXPECT_EQ(s.contain(8456), true);
    EXPECT_EQ(s.size(), 3);
    
    s.add(73);
    EXPECT_EQ(s.contain(73), true);
    EXPECT_EQ(s.size(), 4);

    s.remove(100000);
    EXPECT_EQ(s.contain(100000), false);
    EXPECT_EQ(s.size(), 3);

    s.remove(923424);
    EXPECT_EQ(s.contain(923424), false);
    EXPECT_EQ(s.size(), 2);

    s.remove(8456);
    EXPECT_EQ(s.contain(8456), false);
    EXPECT_EQ(s.size(), 1);

    s.remove(73);
    EXPECT_EQ(s.contain(73), false);
    EXPECT_EQ(s.size(), 0);
}