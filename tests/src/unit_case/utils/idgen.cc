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
#include <utils/idgen.hpp>

using namespace mytho::utils;

/*
 * ======================================== Test Cases ========================================
 */

// Test basic ID generation functionality
TEST(IdGeneratorTest, BasicGeneration) {
    using ComponentGen = basic_id_generator<GeneratorType::COMPONENT_GENOR, uint32_t>;
    using ResourceGen = basic_id_generator<GeneratorType::RESOURCE_GENOR, uint32_t>;
    using EventGen = basic_id_generator<GeneratorType::EVENT_GENOR, uint32_t>;

    auto id1 = ComponentGen::gen<int>();
    auto id2 = ResourceGen::gen<int>();
    auto id3 = EventGen::gen<int>();

    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 0);
    EXPECT_EQ(id3, 0);
}

// Test sequential ID generation
TEST(IdGeneratorTest, SequentialGeneration) {
    using ComponentGen = basic_id_generator<GeneratorType::COMPONENT_GENOR, uint32_t>;

    auto id1 = ComponentGen::gen<int>();
    auto id2 = ComponentGen::gen<float>();
    auto id3 = ComponentGen::gen<double>();

    EXPECT_EQ(id1, 0);
    EXPECT_EQ(id2, 1);
    EXPECT_EQ(id3, 2);
}

// Test independence of different generator types
TEST(IdGeneratorTest, DifferentGeneratorTypes) {
    using ComponentGen = basic_id_generator<GeneratorType::COMPONENT_GENOR, uint32_t>;
    using ResourceGen = basic_id_generator<GeneratorType::RESOURCE_GENOR, uint32_t>;

    auto comp_id1 = ComponentGen::gen<int>();
    auto comp_id2 = ComponentGen::gen<int>();
    auto res_id1 = ResourceGen::gen<int>();
    auto res_id2 = ResourceGen::gen<int>();

    EXPECT_EQ(comp_id1, comp_id2);
    EXPECT_EQ(res_id1, res_id2);
    EXPECT_EQ(comp_id1, 0);
    EXPECT_EQ(res_id1, 0);
}

// Test value_type definition
TEST(IdGeneratorTest, ValueType) {
    using ComponentGen = basic_id_generator<GeneratorType::COMPONENT_GENOR, uint32_t>;

    EXPECT_TRUE((std::is_same_v<ComponentGen::value_type, uint32_t>));
}