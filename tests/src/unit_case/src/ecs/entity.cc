/*
 * Unit Tests for entity.hpp - Entity & Data Wrapper Testing
 * 
 * This test suite validates:
 * - basic_entity: construction, accessors, equality/inequality
 * - utils::is_entity_v: entity trait detection
 * - data_wrapper<T>: general template read/write and tick update semantics
 * - data_wrapper<basic_entity<...>>: specialization read-only access
 * 
 * Entity Test Cases:
 * 1. BasicConstructionAndAccessors - Tests entity construction and accessor methods
 * 2. EqualityAndInequality - Tests equality and inequality operators
 * 3. EntityConceptTrait - Tests entity concept trait detection
 * 
 * DataWrapper Test Cases:
 * 1. DataWrapperGeneralType - Tests data wrapper general template read/write and tick update semantics
 * 2. DataWrapperEntitySpecialization - Tests data wrapper specialization for entity
 * 
 * Entity Trait Test Cases:
 * 1. EntityConceptTrait - Tests entity concept trait detection
 * 
 */

#include <gtest/gtest.h>
#include <type_traits>
#include <ecs/entity.hpp>

using namespace mytho::ecs;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

using entity = basic_entity<uint32_t, uint16_t>;

/*
 * ======================================== Entity Test Cases ============================================
 */

TEST(EntityTest, BasicConstructionAndAccessors) {
    entity e1{123u, 7u};
    EXPECT_EQ(e1.id(), 123u);
    EXPECT_EQ(e1.version(), 7u);

    entity e2{0u};
    EXPECT_EQ(e2.id(), 0u);
    EXPECT_EQ(e2.version(), 0u);
}

TEST(EntityTest, EqualityAndInequality) {
    entity a{10u, 2u};
    entity b{10u, 2u};
    entity c{10u, 3u};
    entity d{11u, 2u};

    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a != b);

    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);

    EXPECT_FALSE(a == d);
    EXPECT_TRUE(a != d);
}

/*
 * ======================================== Trait Test Cases =============================================
 */
TEST(EntityTest, EntityConceptTrait) {
    static_assert(mytho::utils::is_entity_v<entity>, "entity should satisfy is_entity_v");
    EXPECT_TRUE(mytho::utils::is_entity_v<entity>);
    EXPECT_FALSE(mytho::utils::is_entity_v<int>);
}

/*
 * ======================================== DataWrapper Specialization Test Cases ========================
 */

TEST(EntityTest, DataWrapperGeneralType) {
    using mytho::utils::internal::data_wrapper;

    struct S { int x; };

    uint64_t changed_tick = 0;
    uint64_t current_tick = 100;
    S s{5};
    data_wrapper<S> w(&s, changed_tick, current_tick);

    EXPECT_EQ(w->x, 5);
    EXPECT_EQ(changed_tick, 100u); // operator-> updates changed_tick

    w->x = 9;
    EXPECT_EQ(changed_tick, current_tick);
    EXPECT_EQ((*w).x, 9);
}

TEST(EntityTest, DataWrapperEntitySpecialization) {
    using mytho::utils::internal::data_wrapper;

    entity e{42u, 1u};
    data_wrapper<entity> wrap(e);

    EXPECT_EQ(wrap->id(), 42u);
    EXPECT_EQ(wrap->version(), 1u);
    EXPECT_EQ((*wrap).id(), 42u);
}
