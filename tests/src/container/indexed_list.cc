#include <gtest/gtest.h>
#include <container/indexed_list.hpp>

TEST(IndexedListTest, BasicTest) {
    mytho::container::indexed_list<int, uint8_t> list;

    list.emplace_front(10);
    list.emplace_front(20);
    list.emplace_back(30);
    list.emplace_back(40);
    list.sort();
    ASSERT_EQ(list.size(), 4);
    EXPECT_EQ(list[0], 20);
    EXPECT_EQ(list[1], 10);
    EXPECT_EQ(list[2], 30);
    EXPECT_EQ(list[3], 40);

    list.emplace(2, 99);
    list.sort();
    ASSERT_EQ(list.size(), 5);
    EXPECT_EQ(list[0], 20);
    EXPECT_EQ(list[1], 10);
    EXPECT_EQ(list[2], 99);
    EXPECT_EQ(list[3], 30);
    EXPECT_EQ(list[4], 40);

    list.pop(1);
    list.sort();
    ASSERT_EQ(list.size(), 4);
    EXPECT_EQ(list[0], 20);
    EXPECT_EQ(list[1], 99);
    EXPECT_EQ(list[2], 30);
    EXPECT_EQ(list[3], 40);

    list.pop_front();
    list.pop_back();
    list.sort();
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0], 99);
    EXPECT_EQ(list[1], 30);

    list.emplace_front(5);
    list.emplace_back(50);
    list.sort();
    ASSERT_EQ(list.size(), 4);
    EXPECT_EQ(list[0], 5);
    EXPECT_EQ(list[1], 99);
    EXPECT_EQ(list[2], 30);
    EXPECT_EQ(list[3], 50);
}