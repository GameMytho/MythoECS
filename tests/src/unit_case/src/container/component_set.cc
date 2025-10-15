/*
 * Unit Tests for component_set.hpp - Component Set Testing
 * 
 * This test suite validates the basic_component_set functionality in component_set.hpp:
 * - add: Add component to entity
 * - remove: Remove component from entity
 * - replace: Replace component data
 * - get: Get component data by entity
 * - contain: Check if entity has component
 * - is_added: Check if component was added after or at specific tick
 * - is_changed: Check if component was changed after or at specific tick
 * - changed_tick: Get/set component changed tick
 * - clear: Clear all components
 * - size/empty: Get size and check empty state
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic component operations and lifecycle
 * 2. ComponentData - Tests component data access and modification
 * 3. ReplaceOperations - Tests component replacement functionality
 * 4. TickManagement - Tests tick-based component tracking
 * 5. ComplexComponentTypes - Tests complex component types
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <container/component_set.hpp>
#include <ecs/entity.hpp>

using namespace mytho::container;
using namespace mytho::ecs;

/*
 * =============================== Helper Structures/Functions ===============================
 */
#include "components.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using component_set = basic_component_set<entity, int, std::allocator<int>>;
using complex_component_set = basic_component_set<entity, PlayerAttribute, std::allocator<PlayerAttribute>>;

enum class Operation {
    ADD = 0,
    REMOVE = 1,
    REPLACE = 2,
    MODIFY = 3,
    TICK_UPDATE = 4,
    VERIFY = 5,
    MAX_OPERATIONS
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic component operations and lifecycle
TEST(ComponentSetTest, BasicOperations) {
    component_set cs;

    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.empty());

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    cs.add(e1, 100, 42);
    cs.add(e2, 200, 84);
    cs.add(e3, 300, 126);

    EXPECT_EQ(cs.size(), 3);
    EXPECT_FALSE(cs.empty());

    EXPECT_TRUE(cs.contain(e1));
    EXPECT_TRUE(cs.contain(e2));
    EXPECT_TRUE(cs.contain(e3));

    EXPECT_EQ(cs.get(e1), 42);
    EXPECT_EQ(cs.get(e2), 84);
    EXPECT_EQ(cs.get(e3), 126);

    cs.remove(e2);
    EXPECT_EQ(cs.size(), 2);
    EXPECT_FALSE(cs.contain(e2));
    EXPECT_TRUE(cs.contain(e1));
    EXPECT_TRUE(cs.contain(e3));

    cs.clear();
    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.empty());
    EXPECT_FALSE(cs.contain(e1));
    EXPECT_FALSE(cs.contain(e3));
}

// Test tick-based component tracking
TEST(ComponentSetTest, TickManagement) {
    component_set cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add(e1, 100, 42);
    cs.add(e2, 200, 84);

    EXPECT_TRUE(cs.is_added(e1, 100));  // Same tick
    EXPECT_TRUE(cs.is_added(e1, 99));   // Before add tick
    EXPECT_FALSE(cs.is_added(e1, 101)); // After add tick

    EXPECT_TRUE(cs.is_added(e2, 200));
    EXPECT_TRUE(cs.is_added(e2, 199));
    EXPECT_FALSE(cs.is_added(e2, 201));

    EXPECT_TRUE(cs.is_changed(e1, 100));
    EXPECT_TRUE(cs.is_changed(e1, 99));
    EXPECT_FALSE(cs.is_changed(e1, 101));

    cs.changed_tick(e1) = 150;
    EXPECT_TRUE(cs.is_changed(e1, 150));
    EXPECT_TRUE(cs.is_changed(e1, 149));
    EXPECT_FALSE(cs.is_changed(e1, 151));

    EXPECT_TRUE(cs.is_added(e1, 100));
    EXPECT_FALSE(cs.is_added(e1, 101));
}

// Test component data access and modification
TEST(ComponentSetTest, ComponentData) {
    component_set cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add(e1, 100, 50);
    cs.add(e2, 200, 100);

    EXPECT_EQ(cs.get(e1), 50);
    EXPECT_EQ(cs.get(e2), 100);

    const auto& const_cs = cs;
    EXPECT_EQ(const_cs.get(e1), 50);
    EXPECT_EQ(const_cs.get(e2), 100);

    cs.get(e1) = 75;
    cs.get(e2) = 150;

    EXPECT_EQ(cs.get(e1), 75);
    EXPECT_EQ(cs.get(e2), 150);

    cs.changed_tick(e1) = 150;
    cs.changed_tick(e2) = 250;

    EXPECT_EQ(cs.changed_tick(e1), 150);
    EXPECT_EQ(cs.changed_tick(e2), 250);
}

// Test component replacement functionality
TEST(ComponentSetTest, ReplaceOperations) {
    component_set cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add(e1, 100, 42);
    cs.add(e2, 200, 84);

    cs.replace(e1, 150, 100);
    cs.replace(e2, 250, 200);

    EXPECT_EQ(cs.get(e1), 100);
    EXPECT_EQ(cs.get(e2), 200);

    EXPECT_TRUE(cs.is_added(e1, 100));   // Original add tick
    EXPECT_FALSE(cs.is_added(e1, 101));  // After original add tick
    EXPECT_TRUE(cs.is_changed(e1, 150)); // New change tick
    EXPECT_FALSE(cs.is_changed(e1, 151)); // After new change tick

    EXPECT_TRUE(cs.is_added(e2, 200));
    EXPECT_FALSE(cs.is_added(e2, 201));
    EXPECT_TRUE(cs.is_changed(e2, 250));
    EXPECT_FALSE(cs.is_changed(e2, 251));
}

// Test complex component types
TEST(ComponentSetTest, ComplexComponentTypes) {
    complex_component_set ccs;

    entity e1(1, 10);
    entity e2(2, 20);

    ccs.add(e1, 100, 42, "entity1");
    ccs.add(e2, 200, 84, "entity2");

    EXPECT_EQ(ccs.get(e1), PlayerAttribute(42, "entity1"));
    EXPECT_EQ(ccs.get(e2), PlayerAttribute(84, "entity2"));

    ccs.get(e1).value = 100;
    ccs.get(e1).name = "updated_entity1";

    EXPECT_EQ(ccs.get(e1), PlayerAttribute(100, "updated_entity1"));

    ccs.replace(e2, 300, 200, "replaced_entity2");
    EXPECT_EQ(ccs.get(e2), PlayerAttribute(200, "replaced_entity2"));
}

// Test random operations and data integrity
TEST(ComponentSetTest, RandomOperations) {
    component_set cs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> id_dist(1, 1000);
    std::uniform_int_distribution<int> value_dist(1, 1000);
    std::uniform_int_distribution<uint64_t> tick_dist(1, 10000);
    std::uniform_int_distribution<size_t> size_dist(1, 50);
    std::uniform_int_distribution<int> operation_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);

    std::vector<std::pair<entity, int>> entities;
    entities.reserve(100);

    for (int round = 0; round < 1000; ++round) {
        Operation operation = static_cast<Operation>(operation_dist(gen));
        size_t num_operations = size_dist(gen);

        switch (operation) {
            case Operation::ADD: { // Add operation
                for (size_t i = 0; i < num_operations; ++i) {
                    uint32_t id = id_dist(gen);
                    entity e(id, 0);
                    int value = value_dist(gen);
                    uint64_t tick = tick_dist(gen);

                    if (!cs.contain(e)) {
                        cs.add(e, tick, value);
                        entities.push_back({e, value});
                    }
                }
                break;
            }
            case Operation::REMOVE: { // Remove operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, value] = entities[random_index];
                    cs.remove(e);
                    entities.erase(entities.begin() + random_index);
                }
                break;
            }
            case Operation::REPLACE: { // Replace operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, old_value] = entities[random_index];
                    int new_value = value_dist(gen);
                    uint64_t tick = tick_dist(gen);

                    cs.replace(e, tick, new_value);
                    entities[random_index] = {e, new_value};
                }
                break;
            }
            case Operation::MODIFY: { // Modify operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, old_value] = entities[random_index];
                    int new_value = value_dist(gen);

                    cs.get(e) = new_value;
                    entities[random_index] = {e, new_value};
                }
                break;
            }
            case Operation::TICK_UPDATE: { // Tick update operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, value] = entities[random_index];
                    uint64_t new_tick = tick_dist(gen);

                    cs.changed_tick(e) = new_tick;
                }
                break;
            }
            case Operation::VERIFY: { // Verification operation
                EXPECT_EQ(cs.size(), entities.size());

                for (const auto& [e, value] : entities) {
                    EXPECT_TRUE(cs.contain(e));
                    EXPECT_EQ(cs.get(e), value);
                }
                break;
            }

            default:
                break;
        }
    }

    EXPECT_EQ(cs.size(), entities.size());
    for (const auto& [e, value] : entities) {
        EXPECT_TRUE(cs.contain(e));
        EXPECT_EQ(cs.get(e), value);
    }
}
