/*
 * Unit Tests for idgen.hpp - ID Generator Testing
 * 
 * This test suite validates the ID generator functionality in idgen.hpp:
 * - basic_id_generator: Template class for generating unique IDs
 * - GeneratorType: Enum for different generator types
 * 
 * Test Cases:
 * 1. BasicGeneration - Tests basic ID generation functionality
 * 2. SequentialGeneration - Tests sequential ID generation
 * 3. DifferentGeneratorTypes - Tests independence of different generator types
 * 4. ValueType - Tests value_type definition
 */

#include <gtest/gtest.h>

#include "utils/idgen.hpp"

using namespace mytho::utils;

/*
 * ======================================== Helper Structures/Functions ==================================
 */

struct component_genor final {};
struct resource_genor final {};
struct event_genor final {};
struct stage_genor final {};

enum class StageKind { A, B, C };

/*
 * ======================================== Test Cases ========================================
 */

// Test basic ID generation functionality
TEST(IdGeneratorTest, BasicGeneration) {
    using ComponentGen = basic_id_generator<component_genor, uint32_t>;
    using ResourceGen = basic_id_generator<resource_genor, uint32_t>;
    using EventGen = basic_id_generator<event_genor, uint32_t>;
    using StageGen = basic_id_generator<stage_genor, uint32_t>;

    auto id1 = ComponentGen::gen<int>();
    auto id2 = ResourceGen::gen<int>();
    auto id3 = EventGen::gen<int>();
    auto s_a = StageGen::gen<StageKind::A>();

    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 0);
    EXPECT_EQ(id3, 0);
    EXPECT_EQ(s_a, 0u);
}

// Test sequential ID generation
TEST(IdGeneratorTest, SequentialGeneration) {
    using ComponentGen = basic_id_generator<component_genor, uint32_t>;
    using ResourceGen = basic_id_generator<resource_genor, uint32_t>;
    using EventGen = basic_id_generator<event_genor, uint32_t>;
    using StageGen = basic_id_generator<stage_genor, uint32_t>;

    auto id1 = ComponentGen::gen<int>();
    auto id2 = ComponentGen::gen<float>();
    auto id3 = ComponentGen::gen<double>();

    auto r1 = ResourceGen::gen<int>();
    auto r2 = ResourceGen::gen<float>();
    auto r3 = ResourceGen::gen<double>();

    auto e1 = EventGen::gen<int>();
    auto e2 = EventGen::gen<float>();
    auto e3 = EventGen::gen<double>();

    auto s_a1 = StageGen::gen<StageKind::A>();
    auto s_b1 = StageGen::gen<StageKind::B>();
    auto s_a2 = StageGen::gen<StageKind::A>();

    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(id3, 2);
    EXPECT_EQ(r1, 0u);
    EXPECT_EQ(r2, 1u);
    EXPECT_EQ(r3, 2u);
    EXPECT_EQ(e1, 0u);
    EXPECT_EQ(e2, 1u);
    EXPECT_EQ(e3, 2u);
    EXPECT_EQ(s_a1, 0u);
    EXPECT_EQ(s_b1, 1u);
    EXPECT_EQ(s_a1, s_a2);
}

// Test value_type definition
TEST(IdGeneratorTest, ValueType) {
    using ComponentGen = basic_id_generator<component_genor, uint8_t>;
    using ResourceGen = basic_id_generator<resource_genor, uint16_t>;
    using EventGen = basic_id_generator<event_genor, uint32_t>;
    using StageGen = basic_id_generator<stage_genor, uint64_t>;

    EXPECT_TRUE((std::is_same_v<ComponentGen::value_type, uint8_t>));
    EXPECT_TRUE((std::is_same_v<ResourceGen::value_type, uint16_t>));
    EXPECT_TRUE((std::is_same_v<EventGen::value_type, uint32_t>));
    EXPECT_TRUE((std::is_same_v<StageGen::value_type, uint64_t>));
}