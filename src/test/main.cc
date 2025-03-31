#include <gtest/gtest.h>
#include "ecs.hpp"

TEST(AdditionTest, BasicTest) {
    EXPECT_EQ(mytho::ecs::add(2, 3), 5);
}