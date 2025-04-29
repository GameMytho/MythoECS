#include <gtest/gtest.h>
#include <ecs/entity.hpp>

TEST(EntityTest, BasicTest) {
    using entity = mytho::ecs::entity<uint32_t, uint8_t>;
    entity e0(0);
    
    EXPECT_EQ(e0.id(), 0);
    EXPECT_EQ(e0.version(), 0);
    
    entity e10(10, 255);
    
    EXPECT_EQ(e10.id(), 10);
    EXPECT_EQ(e10.version(), 255);
}