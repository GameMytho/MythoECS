/*
 * Unit Tests for assert.hpp - ASSURE Macro Testing
 * 
 * The ASSURE macro is a simple assertion macro that checks if a condition is false
 * and triggers assert(false) if so. Its behavior depends on MYTHO_ASSERTS_ENABLED.
 * 
 * Test Cases:
 * 1. BasicUsage - Tests basic macro functionality with simple conditions
 * 2. DifferentTypes - Tests macro with various data types
 * 
 * Note: We cannot test assert(false) behavior directly as it would terminate
 * the program. These tests focus on syntax correctness and compilation.
 */

#include <gtest/gtest.h>
#include <string>
#include <utils/assert.hpp>

/*
 * ======================================== Test Cases ========================================
 */

// Test basic ASSURE macro functionality
TEST(AssertTest, BasicUsage) {
    ASSURE(true, "Basic true condition");
    ASSURE(1 == 1, "Equality check");
    ASSURE(5 > 3, "Comparison check");

    // Test with variables
    int x = 10;
    int y = 20;
    ASSURE(x < y, "Variable comparison");
    ASSURE(x > 0, "Positive check");
}

// Test ASSURE macro with different data types
TEST(AssertTest, DifferentTypes) {
    // Boolean
    bool flag = true;
    ASSURE(flag, "Boolean test");

    // Integer
    int value = 42;
    ASSURE(value > 0, "Integer test");

    // Floating point
    double pi = 3.14159;
    ASSURE(pi > 3.0, "Floating point test");

    // Pointer
    int* ptr = &value;
    ASSURE(ptr != nullptr, "Pointer test");

    // Function Call
    std::string str = "hello";
    ASSURE(!str.empty(), "String test");
}