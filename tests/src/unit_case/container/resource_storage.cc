/*
 * Unit Tests for resource_storage.hpp - Resource Storage Testing
 * 
 * This test suite validates the basic_resource_storage functionality in resource_storage.hpp:
 * - init: Initialize resources with tick and constructor arguments
 * - deinit: Deinitialize resources and clean up memory
 * - get: Get const and mutable references to resources
 * - contain: Check if resource exists
 * - is_added/is_changed: Check tick-based resource state
 * - get_changed_tick_ref: Get mutable reference to changed tick
 * - clear: Clear all resources and free memory
 * - size/empty: Get size and check empty state
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic resource operations and lifecycle
 * 2. TickManagement - Tests tick-based resource state management
 * 3. ResourceLifecycle - Tests resource lifecycle and reinitialization
 * 4. ComplexResourceTypes - Tests complex resource types with containers
 * 5. MultipleResourceTypes - Tests multiple resource types management
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include "container/resource_storage.hpp"
#include <random>
#include <vector>
#include <string>
#include <algorithm>

using namespace mytho::container;

/*
 * =============================== Helper Structures ===============================
 */
struct TestResource {
    int value;
    std::string name;
    
    TestResource(int v, const std::string& n) : value(v), name(n) {}
    TestResource() : value(0), name("default") {}
    
    bool operator==(const TestResource& other) const {
        return value == other.value && name == other.name;
    }
};

struct AnotherResource {
    float data;
    bool active;
    
    AnotherResource(float d, bool a) : data(d), active(a) {}
    AnotherResource() : data(0.0f), active(false) {}
    
    bool operator==(const AnotherResource& other) const {
        return data == other.data && active == other.active;
    }
};

struct ComplexResource {
    std::vector<int> numbers;
    std::string description;
    
    ComplexResource(const std::vector<int>& nums, const std::string& desc) 
        : numbers(nums), description(desc) {}
    ComplexResource() : numbers(), description("") {}
    
    bool operator==(const ComplexResource& other) const {
        return numbers == other.numbers && description == other.description;
    }
};

/*
 * =============================== Test Cases ===============================
 */

// Test basic operations and resource management
TEST(ResourceStorageTest, BasicOperations) {
    basic_resource_storage<> resource_storage;

    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());

    resource_storage.init<TestResource>(100, 42, "test_resource");
    EXPECT_EQ(resource_storage.size(), 1);
    EXPECT_FALSE(resource_storage.empty());
    EXPECT_TRUE(resource_storage.contain<TestResource>());

    const auto& resource = resource_storage.get<TestResource>();
    EXPECT_EQ(resource.value, 42);
    EXPECT_EQ(resource.name, "test_resource");

    auto& mutable_resource = resource_storage.get<TestResource>();
    mutable_resource.value = 100;
    mutable_resource.name = "modified";

    EXPECT_EQ(resource_storage.get<TestResource>().value, 100);
    EXPECT_EQ(resource_storage.get<TestResource>().name, "modified");

    resource_storage.init<AnotherResource>(200, 3.14f, true);
    EXPECT_EQ(resource_storage.size(), 2);
    EXPECT_TRUE(resource_storage.contain<AnotherResource>());

    const auto& another_resource = resource_storage.get<AnotherResource>();
    EXPECT_EQ(another_resource.data, 3.14f);
    EXPECT_TRUE(another_resource.active);

    resource_storage.deinit<TestResource>();
    EXPECT_EQ(resource_storage.size(), 2); // Size doesn't change, but resource is nullified
    EXPECT_FALSE(resource_storage.contain<TestResource>());
    EXPECT_TRUE(resource_storage.contain<AnotherResource>());

    resource_storage.clear();
    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());
    EXPECT_FALSE(resource_storage.contain<TestResource>());
    EXPECT_FALSE(resource_storage.contain<AnotherResource>());
}

// Test tick management functionality and resource state tracking
TEST(ResourceStorageTest, TickManagement) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<TestResource>(100, 10, "resource1");
    resource_storage.init<AnotherResource>(200, 2.5f, false);

    EXPECT_TRUE(resource_storage.is_added<TestResource>(100));
    EXPECT_TRUE(resource_storage.is_added<TestResource>(50));
    EXPECT_FALSE(resource_storage.is_added<TestResource>(150));

    EXPECT_TRUE(resource_storage.is_added<AnotherResource>(200));
    EXPECT_TRUE(resource_storage.is_added<AnotherResource>(100));
    EXPECT_FALSE(resource_storage.is_added<AnotherResource>(250));

    EXPECT_TRUE(resource_storage.is_changed<TestResource>(100));
    EXPECT_TRUE(resource_storage.is_changed<TestResource>(50));

    resource_storage.get_changed_tick_ref<TestResource>() = 150;
    EXPECT_TRUE(resource_storage.is_changed<TestResource>(150));
    EXPECT_TRUE(resource_storage.is_changed<TestResource>(100));
    EXPECT_FALSE(resource_storage.is_changed<TestResource>(200));

    resource_storage.deinit<TestResource>();
    resource_storage.init<TestResource>(300, 20, "resource2");
    EXPECT_TRUE(resource_storage.is_added<TestResource>(300));
    EXPECT_TRUE(resource_storage.is_changed<TestResource>(300));
}

// Test resource lifecycle, memory management, and reinitialization
TEST(ResourceStorageTest, ResourceLifecycle) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<TestResource>(100, 10, "first");
    resource_storage.init<TestResource>(200, 20, "second"); // Should be ignored

    const auto& resource = resource_storage.get<TestResource>();
    EXPECT_EQ(resource.value, 10);
    EXPECT_EQ(resource.name, "first");

    resource_storage.deinit<TestResource>();
    resource_storage.init<TestResource>(300, 30, "third");

    const auto& new_resource = resource_storage.get<TestResource>();
    EXPECT_EQ(new_resource.value, 30);
    EXPECT_EQ(new_resource.name, "third");
    EXPECT_TRUE(resource_storage.is_added<TestResource>(300));
    EXPECT_TRUE(resource_storage.is_changed<TestResource>(300));
}

// Test complex resource types with containers and custom types
TEST(ResourceStorageTest, ComplexResourceTypes) {
    basic_resource_storage<> resource_storage;

    std::vector<int> numbers = {1, 2, 3, 4, 5};
    resource_storage.init<ComplexResource>(100, numbers, "complex_resource");
    EXPECT_TRUE(resource_storage.contain<ComplexResource>());

    const auto& complex_resource = resource_storage.get<ComplexResource>();
    EXPECT_EQ(complex_resource.numbers, numbers);
    EXPECT_EQ(complex_resource.description, "complex_resource");

    auto& mutable_complex = resource_storage.get<ComplexResource>();
    mutable_complex.numbers.push_back(6);
    mutable_complex.description = "modified_complex";

    EXPECT_EQ(resource_storage.get<ComplexResource>().numbers.size(), 6);
    EXPECT_EQ(resource_storage.get<ComplexResource>().description, "modified_complex");
}

// Test multiple resource types and mixed resource management
TEST(ResourceStorageTest, MultipleResourceTypes) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<TestResource>(100, 1, "resource1");
    resource_storage.init<AnotherResource>(200, 1.5f, true);
    resource_storage.init<ComplexResource>(300, std::vector<int>{10, 20}, "complex1");

    EXPECT_EQ(resource_storage.size(), 3);
    EXPECT_TRUE(resource_storage.contain<TestResource>());
    EXPECT_TRUE(resource_storage.contain<AnotherResource>());
    EXPECT_TRUE(resource_storage.contain<ComplexResource>());

    EXPECT_EQ(resource_storage.get<TestResource>().value, 1);
    EXPECT_EQ(resource_storage.get<AnotherResource>().data, 1.5f);
    EXPECT_TRUE(resource_storage.get<AnotherResource>().active);
    EXPECT_EQ(resource_storage.get<ComplexResource>().numbers.size(), 2);

    resource_storage.deinit<AnotherResource>();
    EXPECT_TRUE(resource_storage.contain<TestResource>());
    EXPECT_FALSE(resource_storage.contain<AnotherResource>());
    EXPECT_TRUE(resource_storage.contain<ComplexResource>());
    EXPECT_EQ(resource_storage.size(), 3); // Size remains the same

    resource_storage.clear();
    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());
    EXPECT_FALSE(resource_storage.contain<TestResource>());
    EXPECT_FALSE(resource_storage.contain<AnotherResource>());
    EXPECT_FALSE(resource_storage.contain<ComplexResource>());
}

// Test random operations and data integrity under various conditions
TEST(ResourceStorageTest, RandomOperations) {
    basic_resource_storage<> resource_storage;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> value_dist(1, 100);
    std::uniform_real_distribution<float> float_dist(0.0f, 100.0f);
    std::uniform_int_distribution<uint64_t> tick_dist(1, 100);
    std::uniform_int_distribution<int> type_dist(0, 2);
    std::uniform_int_distribution<int> operation_dist(0, 3);

    for (int round = 0; round < 50; ++round) {
        int operation = operation_dist(gen);
        int resource_type = type_dist(gen);

        switch (operation) {
            case 0: { // Init operation
                switch (resource_type) {
                    case 0: {
                        int value = value_dist(gen);
                        std::string name = "test_" + std::to_string(value);
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<TestResource>(tick, value, name);
                        break;
                    }
                    case 1: {
                        float data = float_dist(gen);
                        bool active = (value_dist(gen) % 2) == 0;
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<AnotherResource>(tick, data, active);
                        break;
                    }
                    case 2: {
                        std::vector<int> numbers = {value_dist(gen), value_dist(gen)};
                        std::string desc = "complex_" + std::to_string(value_dist(gen));
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<ComplexResource>(tick, numbers, desc);
                        break;
                    }
                }
                break;
            }
            case 1: { // Deinit operation
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<TestResource>()) {
                            resource_storage.deinit<TestResource>();
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<AnotherResource>()) {
                            resource_storage.deinit<AnotherResource>();
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<ComplexResource>()) {
                            resource_storage.deinit<ComplexResource>();
                        }
                        break;
                }
                break;
            }
            case 2: { // Get operation
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<TestResource>()) {
                            const auto& resource = resource_storage.get<TestResource>();
                            EXPECT_GE(resource.value, 1);
                            EXPECT_LE(resource.value, 100);
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<AnotherResource>()) {
                            const auto& resource = resource_storage.get<AnotherResource>();
                            EXPECT_GE(resource.data, 0.0f);
                            EXPECT_LE(resource.data, 100.0f);
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<ComplexResource>()) {
                            const auto& resource = resource_storage.get<ComplexResource>();
                            EXPECT_GE(resource.numbers.size(), 0);
                        }
                        break;
                }
                break;
            }
            case 3: { // Tick operations
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<TestResource>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<TestResource>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<TestResource>(tick));
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<AnotherResource>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<AnotherResource>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<AnotherResource>(tick));
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<ComplexResource>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<ComplexResource>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<ComplexResource>(tick));
                        }
                        break;
                }
                break;
            }
        }
    }
}
