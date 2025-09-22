/*
 * Unit Tests for concept.hpp - C++20 Concepts Testing
 * 
 * This test suite validates the concept definitions in concept.hpp:
 * - UnsignedIntegralType: Checks if a type is an unsigned integer
 * - PureValueType: Checks if a type is a pure value type (not const, ref, or pointer)
 * 
 * Test Cases:
 * 1. UnsignedIntegralType - Tests unsigned integer type detection
 * 2. PureValueType - Tests pure value type detection
 * 3. TemplateUsage - Tests template constraints
 * 4. SFINAEUsage - Tests SFINAE
 * 5. ConditionalCompilation - Tests if constexpr
 * 6. TypeTraitsIntegration - Tests type traits integration
 * 7. SpecialTypes - Tests special types
 */

#include <gtest/gtest.h>
#include <type_traits>
#include <utils/concept.hpp>

using namespace mytho::utils;

/*
 * ======================================== Helper Structures/Functions ========================================
 */

struct TestStruct { int x; };

enum class TestEnum { A, B, C };

template<UnsignedIntegralType T>
bool is_positive_unsigned(T value) {
    return value > 0;
}

template<PureValueType T>
T identity(T value) {
    return value;
}

template<typename T>
std::enable_if_t<UnsignedIntegralType<T>, bool> is_even(T value) {
    return value % 2 == 0;
}

template<typename T>
std::enable_if_t<PureValueType<T>, T> copy_value(T value) {
    return value;
}

template<typename T>
void process_type(T value) {
    if constexpr (UnsignedIntegralType<T>) {
        EXPECT_GE(value, 0);
    } else if constexpr (PureValueType<T>) {
        T copy = value;
        EXPECT_EQ(copy, value);
    }
}

template<typename T>
void test_type_properties() {
    if constexpr (UnsignedIntegralType<T>) {
        EXPECT_TRUE(std::is_integral_v<T>);
        EXPECT_TRUE(std::is_unsigned_v<T>);
    }

    if constexpr (PureValueType<T>) {
        EXPECT_FALSE(std::is_const_v<T>);
        EXPECT_FALSE(std::is_reference_v<T>);
        EXPECT_FALSE(std::is_pointer_v<T>);
    }
}

/*
 * ======================================== Test Cases ========================================
 */

// Test UnsignedIntegralType concept
TEST(ConceptTest, UnsignedIntegralType) {
    // Test unsigned integer types
    EXPECT_TRUE(UnsignedIntegralType<bool>);
    EXPECT_TRUE(UnsignedIntegralType<uint8_t>);
    EXPECT_TRUE(UnsignedIntegralType<uint16_t>);
    EXPECT_TRUE(UnsignedIntegralType<uint32_t>);
    EXPECT_TRUE(UnsignedIntegralType<uint64_t>);
    EXPECT_TRUE(UnsignedIntegralType<unsigned char>);
    EXPECT_TRUE(UnsignedIntegralType<unsigned short>);
    EXPECT_TRUE(UnsignedIntegralType<unsigned int>);
    EXPECT_TRUE(UnsignedIntegralType<unsigned long>);
    EXPECT_TRUE(UnsignedIntegralType<unsigned long long>);

    // Test signed integer types (should be false)
    EXPECT_FALSE(UnsignedIntegralType<int8_t>);
    EXPECT_FALSE(UnsignedIntegralType<int16_t>);
    EXPECT_FALSE(UnsignedIntegralType<int32_t>);
    EXPECT_FALSE(UnsignedIntegralType<int64_t>);
    EXPECT_FALSE(UnsignedIntegralType<char>);
    EXPECT_FALSE(UnsignedIntegralType<short>);
    EXPECT_FALSE(UnsignedIntegralType<int>);
    EXPECT_FALSE(UnsignedIntegralType<long>);
    EXPECT_FALSE(UnsignedIntegralType<long long>);

    // Test non-integer types (should be false)
    EXPECT_FALSE(UnsignedIntegralType<float>);
    EXPECT_FALSE(UnsignedIntegralType<double>);
    EXPECT_FALSE(UnsignedIntegralType<long double>);
    EXPECT_FALSE(UnsignedIntegralType<void>);
    EXPECT_FALSE(UnsignedIntegralType<std::string>);
    EXPECT_FALSE(UnsignedIntegralType<int*>);
    EXPECT_FALSE(UnsignedIntegralType<const int>);
    EXPECT_FALSE(UnsignedIntegralType<volatile int>);
}

// Test PureValueType concept
TEST(ConceptTest, PureValueType) {
    // Test pure value types (should be true)
    EXPECT_TRUE(PureValueType<int>);
    EXPECT_TRUE(PureValueType<float>);
    EXPECT_TRUE(PureValueType<double>);
    EXPECT_TRUE(PureValueType<bool>);
    EXPECT_TRUE(PureValueType<char>);
    EXPECT_TRUE(PureValueType<std::string>);
    EXPECT_TRUE(PureValueType<std::vector<int>>);
    EXPECT_TRUE(PureValueType<TestStruct>);

    // Test const types (should be false)
    EXPECT_FALSE(PureValueType<const int>);
    EXPECT_FALSE(PureValueType<const float>);
    EXPECT_FALSE(PureValueType<const std::string>);
    EXPECT_FALSE(PureValueType<const volatile int>);

    // Test reference types (should be false)
    EXPECT_FALSE(PureValueType<int&>);
    EXPECT_FALSE(PureValueType<const int&>);
    EXPECT_FALSE(PureValueType<int&&>);
    EXPECT_FALSE(PureValueType<const int&&>);

    // Test pointer types (should be false)
    EXPECT_FALSE(PureValueType<int*>);
    EXPECT_FALSE(PureValueType<const int*>);
    EXPECT_FALSE(PureValueType<int**>);
    EXPECT_FALSE(PureValueType<const int* const>);

    // Test array types
    EXPECT_TRUE(PureValueType<int[10]>);
    EXPECT_FALSE(PureValueType<const int[10]>);
    EXPECT_TRUE(PureValueType<int[][10]>);
}

// Test templates usage
TEST(ConceptTest, TemplateUsage) {
    // Test UnsignedIntegralType usage in templates
    EXPECT_TRUE(is_positive_unsigned(5u));
    EXPECT_TRUE(is_positive_unsigned(10u));
    EXPECT_FALSE(is_positive_unsigned(0u));

    // Test PureValueType usage in templates
    EXPECT_EQ(identity(42), 42);
    EXPECT_EQ(identity(3.14f), 3.14f);
    EXPECT_EQ(identity(true), true);
    EXPECT_EQ(identity(std::string("hello")), std::string("hello"));
}

// Test SFINAE usage
TEST(ConceptTest, SFINAEUsage) {
    EXPECT_TRUE(is_even(4u));
    EXPECT_FALSE(is_even(5u));

    EXPECT_EQ(copy_value(100), 100);
    EXPECT_EQ(copy_value(2.5), 2.5);
}

// Test conditional compilation
TEST(ConceptTest, ConditionalCompilation) {
    process_type(5u);        // UnsignedIntegralType
    process_type(3.14);      // PureValueType
    process_type(std::string("test")); // PureValueType
}

// Test type traits integration
TEST(ConceptTest, TypeTraitsIntegration) {
    test_type_properties<uint32_t>();
    test_type_properties<int>();
    test_type_properties<float>();
    test_type_properties<std::string>();
    test_type_properties<const int>();
    test_type_properties<int&>();
    test_type_properties<int*>();
}

// Test special types
TEST(ConceptTest, SpecialTypes) {
    EXPECT_FALSE(UnsignedIntegralType<TestEnum>);
    EXPECT_TRUE(PureValueType<TestEnum>);

    union TestUnion { int i; float f; };
    EXPECT_FALSE(UnsignedIntegralType<TestUnion>);
    EXPECT_TRUE(PureValueType<TestUnion>);

    EXPECT_FALSE(UnsignedIntegralType<void()>);
    EXPECT_TRUE(PureValueType<void()>);  // Function types are pure value types

    EXPECT_FALSE(UnsignedIntegralType<void(TestUnion::*)()>);
    EXPECT_TRUE(PureValueType<void(TestUnion::*)()>);  // Member function pointers are pure value types
}
