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
#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <container/resource_storage.hpp>

using namespace mytho::container;

/*
 * =============================== Helper Structures ===============================
 */
#include "resources.hpp"

enum class Operation {
    INIT = 0,
    DEINIT = 1,
    GET = 2,
    TICK = 3,
    MAX_OPERATIONS
};

/*
 * =============================== Test Cases ===============================
 */

// Test basic operations and resource management
TEST(ResourceStorageTest, BasicOperations) {
    basic_resource_storage<> resource_storage;

    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());

    resource_storage.init<GameConfig>(100, 42, "test_resource");
    EXPECT_EQ(resource_storage.size(), 1);
    EXPECT_FALSE(resource_storage.empty());
    EXPECT_TRUE(resource_storage.contain<GameConfig>());

    const auto& resource = resource_storage.get<GameConfig>();
    EXPECT_EQ(resource.value, 42);
    EXPECT_EQ(resource.name, "test_resource");

    auto& mutable_resource = resource_storage.get<GameConfig>();
    mutable_resource.value = 100;
    mutable_resource.name = "modified";

    EXPECT_EQ(resource_storage.get<GameConfig>().value, 100);
    EXPECT_EQ(resource_storage.get<GameConfig>().name, "modified");

    resource_storage.init<PhysicsSettings>(200, 3.14f, true);
    EXPECT_EQ(resource_storage.size(), 2);
    EXPECT_TRUE(resource_storage.contain<PhysicsSettings>());

    const auto& another_resource = resource_storage.get<PhysicsSettings>();
    EXPECT_EQ(another_resource.data, 3.14f);
    EXPECT_TRUE(another_resource.active);

    resource_storage.deinit<GameConfig>();
    EXPECT_EQ(resource_storage.size(), 2); // Size doesn't change, but resource is nullified
    EXPECT_FALSE(resource_storage.contain<GameConfig>());
    EXPECT_TRUE(resource_storage.contain<PhysicsSettings>());

    resource_storage.clear();
    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());
    EXPECT_FALSE(resource_storage.contain<GameConfig>());
    EXPECT_FALSE(resource_storage.contain<PhysicsSettings>());
}

// Test tick management functionality and resource state tracking
TEST(ResourceStorageTest, TickManagement) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<GameConfig>(100, 10, "resource1");
    resource_storage.init<PhysicsSettings>(200, 2.5f, false);

    EXPECT_TRUE(resource_storage.is_added<GameConfig>(100));
    EXPECT_TRUE(resource_storage.is_added<GameConfig>(50));
    EXPECT_FALSE(resource_storage.is_added<GameConfig>(150));

    EXPECT_TRUE(resource_storage.is_added<PhysicsSettings>(200));
    EXPECT_TRUE(resource_storage.is_added<PhysicsSettings>(100));
    EXPECT_FALSE(resource_storage.is_added<PhysicsSettings>(250));

    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(100));
    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(50));

    resource_storage.get_changed_tick_ref<GameConfig>() = 150;
    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(150));
    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(100));
    EXPECT_FALSE(resource_storage.is_changed<GameConfig>(200));

    resource_storage.deinit<GameConfig>();
    resource_storage.init<GameConfig>(300, 20, "resource2");
    EXPECT_TRUE(resource_storage.is_added<GameConfig>(300));
    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(300));
}

// Test resource lifecycle, memory management, and reinitialization
TEST(ResourceStorageTest, ResourceLifecycle) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<GameConfig>(100, 10, "first");
    resource_storage.init<GameConfig>(200, 20, "second"); // Should be ignored

    const auto& resource = resource_storage.get<GameConfig>();
    EXPECT_EQ(resource.value, 10);
    EXPECT_EQ(resource.name, "first");

    resource_storage.deinit<GameConfig>();
    resource_storage.init<GameConfig>(300, 30, "third");

    const auto& new_resource = resource_storage.get<GameConfig>();
    EXPECT_EQ(new_resource.value, 30);
    EXPECT_EQ(new_resource.name, "third");
    EXPECT_TRUE(resource_storage.is_added<GameConfig>(300));
    EXPECT_TRUE(resource_storage.is_changed<GameConfig>(300));
}

// Test complex resource types with containers and custom types
TEST(ResourceStorageTest, ComplexResourceTypes) {
    basic_resource_storage<> resource_storage;

    std::vector<int> numbers = {1, 2, 3, 4, 5};
    resource_storage.init<LevelData>(100, numbers, "complex_resource");
    EXPECT_TRUE(resource_storage.contain<LevelData>());

    const auto& complex_resource = resource_storage.get<LevelData>();
    EXPECT_EQ(complex_resource.numbers, numbers);
    EXPECT_EQ(complex_resource.description, "complex_resource");

    auto& mutable_complex = resource_storage.get<LevelData>();
    mutable_complex.numbers.push_back(6);
    mutable_complex.description = "modified_complex";

    EXPECT_EQ(resource_storage.get<LevelData>().numbers.size(), 6);
    EXPECT_EQ(resource_storage.get<LevelData>().description, "modified_complex");
}

// Test multiple resource types and mixed resource management
TEST(ResourceStorageTest, MultipleResourceTypes) {
    basic_resource_storage<> resource_storage;

    resource_storage.init<GameConfig>(100, 1, "resource1");
    resource_storage.init<PhysicsSettings>(200, 1.5f, true);
    resource_storage.init<LevelData>(300, std::vector<int>{10, 20}, "complex1");

    EXPECT_EQ(resource_storage.size(), 3);
    EXPECT_TRUE(resource_storage.contain<GameConfig>());
    EXPECT_TRUE(resource_storage.contain<PhysicsSettings>());
    EXPECT_TRUE(resource_storage.contain<LevelData>());

    EXPECT_EQ(resource_storage.get<GameConfig>().value, 1);
    EXPECT_EQ(resource_storage.get<PhysicsSettings>().data, 1.5f);
    EXPECT_TRUE(resource_storage.get<PhysicsSettings>().active);
    EXPECT_EQ(resource_storage.get<LevelData>().numbers.size(), 2);

    resource_storage.deinit<PhysicsSettings>();
    EXPECT_TRUE(resource_storage.contain<GameConfig>());
    EXPECT_FALSE(resource_storage.contain<PhysicsSettings>());
    EXPECT_TRUE(resource_storage.contain<LevelData>());
    EXPECT_EQ(resource_storage.size(), 3); // Size remains the same

    resource_storage.clear();
    EXPECT_EQ(resource_storage.size(), 0);
    EXPECT_TRUE(resource_storage.empty());
    EXPECT_FALSE(resource_storage.contain<GameConfig>());
    EXPECT_FALSE(resource_storage.contain<PhysicsSettings>());
    EXPECT_FALSE(resource_storage.contain<LevelData>());
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
    std::uniform_int_distribution<int> operation_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);

    for (int round = 0; round < 50; ++round) {
        Operation operation = static_cast<Operation>(operation_dist(gen));
        int resource_type = type_dist(gen);

        switch (operation) {
            case Operation::INIT: { // Init operation
                switch (resource_type) {
                    case 0: {
                        int value = value_dist(gen);
                        std::string name = "test_" + std::to_string(value);
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<GameConfig>(tick, value, name);
                        break;
                    }
                    case 1: {
                        float data = float_dist(gen);
                        bool active = (value_dist(gen) % 2) == 0;
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<PhysicsSettings>(tick, data, active);
                        break;
                    }
                    case 2: {
                        std::vector<int> numbers = {value_dist(gen), value_dist(gen)};
                        std::string desc = "complex_" + std::to_string(value_dist(gen));
                        uint64_t tick = tick_dist(gen);
                        resource_storage.init<LevelData>(tick, numbers, desc);
                        break;
                    }
                }
                break;
            }

            case Operation::DEINIT: { // Deinit operation
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<GameConfig>()) {
                            resource_storage.deinit<GameConfig>();
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<PhysicsSettings>()) {
                            resource_storage.deinit<PhysicsSettings>();
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<LevelData>()) {
                            resource_storage.deinit<LevelData>();
                        }
                        break;
                }
                break;
            }

            case Operation::GET: { // Get operation
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<GameConfig>()) {
                            const auto& resource = resource_storage.get<GameConfig>();
                            EXPECT_GE(resource.value, 1);
                            EXPECT_LE(resource.value, 100);
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<PhysicsSettings>()) {
                            const auto& resource = resource_storage.get<PhysicsSettings>();
                            EXPECT_GE(resource.data, 0.0f);
                            EXPECT_LE(resource.data, 100.0f);
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<LevelData>()) {
                            const auto& resource = resource_storage.get<LevelData>();
                            EXPECT_GE(resource.numbers.size(), 0);
                        }
                        break;
                }
                break;
            }

            case Operation::TICK: { // Tick operations
                switch (resource_type) {
                    case 0:
                        if (resource_storage.contain<GameConfig>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<GameConfig>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<GameConfig>(tick));
                        }
                        break;
                    case 1:
                        if (resource_storage.contain<PhysicsSettings>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<PhysicsSettings>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<PhysicsSettings>(tick));
                        }
                        break;
                    case 2:
                        if (resource_storage.contain<LevelData>()) {
                            uint64_t tick = tick_dist(gen);
                            resource_storage.get_changed_tick_ref<LevelData>() = tick;
                            EXPECT_TRUE(resource_storage.is_changed<LevelData>(tick));
                        }
                        break;
                    default:
                        break;
                }
                break;
            }

            default:
                break;
        }
    }
}
