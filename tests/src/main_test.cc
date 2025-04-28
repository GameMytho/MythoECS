#include <ecs/ecs.hpp>
#include <cstdint>
#include <gtest/gtest.h>

TEST(BasicTest, AddAndRemove) {
    mytho::container::basic_sparse_set<uint32_t, 1024> s;

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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}