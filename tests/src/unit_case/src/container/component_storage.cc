/*
 * Unit Tests for component_storage.hpp - Component Storage Testing
 * 
 * This test suite validates the basic_component_storage functionality in component_storage.hpp:
 * - add: Add multiple components for entity
 * - remove: Remove specific components or all components of entity
 * - get: Get multiple components of entity
 * - replace: Replace multiple components of entity
 * - contain: Check if entity has specific components
 * - not_contain: Check if entity doesn't have specific components
 * - is_added: Check if components were added after or at specific tick
 * - is_changed: Check if components were changed after or at specific tick
 * - removed_entities: Get list of entities that had specific component type removed
 * - clear: Clear all components
 * - removed_entities_clear: Clear all removed entities lists
 * - size/empty: Get size and check empty state
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic component storage operations
 * 2. MultiComponentOperations - Tests multiple component operations
 * 3. ComponentQueries - Tests component query operations
 * 4. TickManagement - Tests tick-based component tracking
 * 5. ReplaceOperations - Tests component replacement functionality
 * 6. ComplexComponentTypes - Tests complex component types
 * 7. RemovedEntities - Tests removed entities functionality and clear operations
 * 8. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <cmath>
#include <container/component_storage.hpp>
#include <ecs/entity.hpp>

using namespace mytho::container;
using namespace mytho::ecs;

/*
 * =============================== Helper Structures/Functions ===============================
 */
#include "components.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using component_storage = basic_component_storage<entity, component_genor>;

enum class Operation {
    ADD_POSITION = 0,
    ADD_VELOCITY = 1,
    ADD_HEALTH = 2,
    REMOVE_ONE_COMPONENT = 3,
    REMOVE_ALL = 4,
    REPLACE_ONE = 5,
    VERIFY = 6,
    MAX_OPERATIONS
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic component storage operations
TEST(ComponentStorageTest, BasicOperations) {
    component_storage cs;

    EXPECT_EQ(cs.size(), 0);
    EXPECT_TRUE(cs.empty());

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add<int>(e1, 100, 42);
    cs.add<float>(e2, 200, 3.14f);

    EXPECT_EQ(cs.size(), 2);
    EXPECT_FALSE(cs.empty());

    EXPECT_TRUE(cs.contain<int>(e1));
    EXPECT_FALSE(cs.contain<int>(e2));
    EXPECT_FALSE(cs.contain<float>(e1));
    EXPECT_TRUE(cs.contain<float>(e2));

    EXPECT_TRUE(cs.not_contain<float>(e1));
    EXPECT_TRUE(cs.not_contain<int>(e2));

    auto [int_val] = cs.get<int>(e1);
    auto [float_val] = cs.get<float>(e2);
    EXPECT_EQ(int_val, 42);
    EXPECT_EQ(float_val, 3.14f);

    cs.remove<int>(e1);
    cs.remove<float>(e2);

    EXPECT_EQ(cs.size(), 2); // Size remains the same, because the component set is still exist, we just remove the components from component set
    EXPECT_FALSE(cs.empty());
    EXPECT_FALSE(cs.contain<int>(e1));
    EXPECT_FALSE(cs.contain<float>(e2));
}

// Test multiple component operations
TEST(ComponentStorageTest, MultiComponentOperations) {
    component_storage cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add<Position, Velocity, Health>(e1, 100, Position(10, 20), Velocity(5, -2), Health(80, 100));
    cs.add<Position, int>(e2, 200, Position(30, 40), 42);

    EXPECT_TRUE((cs.contain<Position, Velocity, Health>(e1)));
    EXPECT_TRUE((cs.contain<Position, int>(e2)));
    EXPECT_FALSE((cs.contain<Velocity, Health>(e2)));
    EXPECT_FALSE(cs.contain<int>(e1));

    auto [pos1, vel1, health1] = cs.get<Position, Velocity, Health>(e1);
    auto [pos2, int_val2] = cs.get<Position, int>(e2);

    EXPECT_EQ(pos1, Position(10, 20));
    EXPECT_EQ(vel1, Velocity(5, -2));
    EXPECT_EQ(health1, Health(80, 100));
    EXPECT_EQ(pos2, Position(30, 40));
    EXPECT_EQ(int_val2, 42);

    cs.remove<Velocity, Health>(e1);
    EXPECT_FALSE((cs.contain<Velocity, Health>(e1)));
    EXPECT_TRUE(cs.contain<Position>(e1));

    cs.remove<>(e1);
    EXPECT_FALSE(cs.contain<Position>(e1));
    EXPECT_TRUE((cs.contain<Position, int>(e2)));
}

// Test tick-based component tracking
TEST(ComponentStorageTest, TickManagement) {
    component_storage cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add<Position, Velocity>(e1, 100, Position(1, 2), Velocity(3, 4));
    cs.add<Position, Health>(e2, 200, Position(5.0f, 6.0f), Health(50, 100));

    EXPECT_TRUE((cs.is_added<Position, Velocity>(e1, 100)));
    EXPECT_TRUE((cs.is_added<Position, Velocity>(e1, 99)));
    EXPECT_FALSE((cs.is_added<Position, Velocity>(e1, 101)));

    EXPECT_TRUE((cs.is_added<Position, Health>(e2, 200)));
    EXPECT_TRUE((cs.is_added<Position, Health>(e2, 199)));
    EXPECT_FALSE((cs.is_added<Position, Health>(e2, 201)));

    EXPECT_TRUE((cs.is_changed<Position, Velocity>(e1, 100)));
    EXPECT_TRUE((cs.is_changed<Position, Velocity>(e1, 99)));
    EXPECT_FALSE((cs.is_changed<Position, Velocity>(e1, 101)));

    EXPECT_TRUE(cs.is_added<Position>(e1, 100));
    EXPECT_TRUE(cs.is_added<Velocity>(e1, 100));
    EXPECT_FALSE(cs.is_added<Health>(e1, 100));
}

// Test component replacement functionality
TEST(ComponentStorageTest, ReplaceOperations) {
    component_storage cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add<Position, Velocity>(e1, 100, Position(1, 2), Velocity(3, 4));
    cs.add<Position, Health>(e2, 200, Position(5, 6), Health(50, 100));

    cs.replace<Position, Velocity>(e1, 150, Position(10, 20), Velocity(30, 40));
    cs.replace<Position, Health>(e2, 250, Position(50.0f, 60.0f), Health(80, 100));

    auto [pos1, vel1] = cs.get<Position, Velocity>(e1);
    auto [pos2, health2] = cs.get<Position, Health>(e2);

    EXPECT_EQ(pos1, Position(10, 20));
    EXPECT_EQ(vel1, Velocity(30, 40));
    EXPECT_EQ(pos2, Position(50, 60));
    EXPECT_EQ(health2, Health(80, 100));

    EXPECT_TRUE((cs.is_added<Position, Velocity>(e1, 100)));   // Original add tick
    EXPECT_FALSE((cs.is_added<Position, Velocity>(e1, 101)));  // After original add tick
    EXPECT_TRUE((cs.is_changed<Position, Velocity>(e1, 150))); // New change tick
    EXPECT_FALSE((cs.is_changed<Position, Velocity>(e1, 151))); // After new change tick
}

// Test complex component types
TEST(ComponentStorageTest, ComplexComponentTypes) {
    component_storage cs;

    entity e1(1, 10);
    entity e2(2, 20);

    cs.add<std::string, int>(e1, 100, "entity1", 42);
    cs.add<std::string, float>(e2, 200, "entity2", 3.14f);

    auto [str1, int_val1] = cs.get<std::string, int>(e1);
    auto [str2, float_val2] = cs.get<std::string, float>(e2);

    EXPECT_EQ(str1, "entity1");
    EXPECT_EQ(int_val1, 42);
    EXPECT_EQ(str2, "entity2");
    EXPECT_EQ(float_val2, 3.14f);

    cs.replace<std::string, int>(e1, 150, "modified_entity1", 100);

    auto [str1_modified, int_val1_modified] = cs.get<std::string, int>(e1);
    EXPECT_EQ(str1_modified, "modified_entity1");
    EXPECT_EQ(int_val1_modified, 100);
}

// Test removed entities functionality
TEST(ComponentStorageTest, RemovedEntities) {
    component_storage cs;

    entity e1(1, 10);
    entity e2(2, 20);

    EXPECT_TRUE(cs.removed_entities<Position>().empty());

    cs.add<Position, Velocity>(e1, 100, Position(10, 20), Velocity(5, -2));
    cs.add<Position>(e2, 200, Position(30, 40));

    cs.remove<Position>(e1);
    cs.remove<Velocity>(e1);

    EXPECT_EQ(cs.removed_entities<Position>().size(), 1);
    EXPECT_EQ(cs.removed_entities<Position>()[0], e1);
    EXPECT_EQ(cs.removed_entities<Velocity>().size(), 1);
    EXPECT_EQ(cs.removed_entities<Velocity>()[0], e1);

    cs.removed_entities_clear();
    EXPECT_TRUE(cs.removed_entities<Position>().empty());
    EXPECT_TRUE(cs.removed_entities<Velocity>().empty());

    cs.add<Position, Velocity>(e1, 300, Position(1, 2), Velocity(3, 4));
    cs.remove<>(e1);

    EXPECT_EQ(cs.removed_entities<Position>().size(), 1);
    EXPECT_EQ(cs.removed_entities<Velocity>().size(), 1);
}

// Test random operations and data integrity
TEST(ComponentStorageTest, RandomOperations) {
    component_storage cs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> id_dist(1, 100);
    std::uniform_int_distribution<int> value_dist(1, 100);
    std::uniform_int_distribution<uint64_t> tick_dist(1, 1000);
    std::uniform_int_distribution<size_t> size_dist(1, 20);
    std::uniform_int_distribution<int> operation_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);
    std::uniform_int_distribution<int> component_type_dist(0, 2); // 0=Position, 1=Velocity, 2=Health

    std::vector<std::tuple<entity, Position, Velocity, Health>> entities;
    entities.reserve(100);

    for (int round = 0; round < 100; ++round) {
        Operation operation = static_cast<Operation>(operation_dist(gen));
        size_t num_operations = size_dist(gen);

        switch (operation) {
            case Operation::ADD_POSITION: { // Add Position components
                for (size_t i = 0; i < num_operations; ++i) {
                    uint32_t id = id_dist(gen);
                    entity e(id, 0);
                    int x = value_dist(gen) / 10 + 1;
                    int y = value_dist(gen) / 10 + 1;
                    uint64_t tick = tick_dist(gen);

                    if (cs.not_contain<Position>(e)) {
                        cs.add<Position>(e, tick, Position(x, y));

                        auto it = std::find_if(entities.begin(), entities.end(),
                            [&e](const auto& tuple) { return std::get<0>(tuple) == e; });
                        if (it == entities.end()) {
                            entities.push_back({e, Position(x, y), Velocity(), Health()});
                        } else {
                            std::get<1>(*it) = Position(x, y);
                        }
                    }
                }
                break;
            }

            case Operation::ADD_VELOCITY: { // Add Velocity components
                for (size_t i = 0; i < num_operations; ++i) {
                    uint32_t id = id_dist(gen);
                    entity e(id, 0);
                    int vx = value_dist(gen) / 10 + 1;
                    int vy = value_dist(gen) / 10 + 1;
                    uint64_t tick = tick_dist(gen);

                    if (cs.not_contain<Velocity>(e)) {
                        cs.add<Velocity>(e, tick, Velocity(vx, vy));

                        auto it = std::find_if(entities.begin(), entities.end(),
                            [&e](const auto& tuple) { return std::get<0>(tuple) == e; });
                        if (it == entities.end()) {
                            entities.push_back({e, Position(), Velocity(vx, vy), Health()});
                        } else {
                            std::get<2>(*it) = Velocity(vx, vy);
                        }
                    }
                }
                break;
            }

            case Operation::ADD_HEALTH: { // Add Health components
                for (size_t i = 0; i < num_operations; ++i) {
                    uint32_t id = id_dist(gen);
                    entity e(id, 0);
                    int current = value_dist(gen) % 100 + 1;
                    int max = current + (value_dist(gen) % 50) + 1;
                    uint64_t tick = tick_dist(gen);

                    if (cs.not_contain<Health>(e)) {
                        cs.add<Health>(e, tick, Health(current, max));

                        auto it = std::find_if(entities.begin(), entities.end(),
                            [&e](const auto& tuple) { return std::get<0>(tuple) == e; });
                        if (it == entities.end()) {
                            entities.push_back({e, Position(), Velocity(), Health(current, max)});
                        } else {
                            std::get<3>(*it) = Health(current, max);
                        }
                    }
                }
                break;
            }

            case Operation::REMOVE_ONE_COMPONENT: { // Remove specific components
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, pos, vel, health] = entities[random_index];
                    int component_type = component_type_dist(gen);

                    switch (component_type) {
                        case 0: // Remove Position
                            if (cs.contain<Position>(e)) {
                                cs.remove<Position>(e);
                                std::get<1>(entities[random_index]) = Position();
                            }
                            break;
                        case 1: // Remove Velocity
                            if (cs.contain<Velocity>(e)) {
                                cs.remove<Velocity>(e);
                                std::get<2>(entities[random_index]) = Velocity();
                            }
                            break;
                        case 2: // Remove Health
                            if (cs.contain<Health>(e)) {
                                cs.remove<Health>(e);
                                std::get<3>(entities[random_index]) = Health();
                            }
                            break;
                    }
                }
                break;
            }

            case Operation::REMOVE_ALL: { // Remove all components from entity
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, pos, vel, health] = entities[random_index];
                    cs.remove<>(e);
                    entities.erase(entities.begin() + random_index);
                }
                break;
            }

            case Operation::REPLACE_ONE: { // Replace components
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    auto [e, pos, vel, health] = entities[random_index];
                    int component_type = component_type_dist(gen);
                    uint64_t tick = tick_dist(gen);

                    switch (component_type) {
                        case 0: // Replace Position
                            if (cs.contain<Position>(e)) {
                                int x = value_dist(gen) / 10 + 1;
                                int y = value_dist(gen) / 10 + 1;
                                cs.replace<Position>(e, tick, Position(x, y));
                                std::get<1>(entities[random_index]) = Position(x, y);
                            }

                            break;
                        case 1: // Replace Velocity
                            if (cs.contain<Velocity>(e)) {
                                int vx = value_dist(gen) / 10 + 1;
                                int vy = value_dist(gen) / 10 + 1;
                                cs.replace<Velocity>(e, tick, Velocity(vx, vy));
                                std::get<2>(entities[random_index]) = Velocity(vx, vy);
                            }
                            break;
                        case 2: // Replace Health
                            if (cs.contain<Health>(e)) {
                                int current = value_dist(gen) % 100 + 1;
                                int max = current + (value_dist(gen) % 50) + 1;
                                cs.replace<Health>(e, tick, Health(current, max));
                                std::get<3>(entities[random_index]) = Health(current, max);
                            }
                            break;
                    }
                }
                break;
            }

            case Operation::VERIFY: { // Verification operation
                for (const auto& [e, pos, vel, health] : entities) {
                    if (pos.x != 0 || pos.y != 0) {
                        EXPECT_TRUE(cs.contain<Position>(e));
                        auto [actual_pos] = cs.get<Position>(e);
                         EXPECT_EQ(actual_pos, pos);
                    } else {
                        EXPECT_FALSE(cs.contain<Position>(e));
                    }

                    if (vel.vx != 0 || vel.vy != 0) {
                        EXPECT_TRUE(cs.contain<Velocity>(e));
                        auto [actual_vel] = cs.get<Velocity>(e);
                        EXPECT_EQ(actual_vel, vel);
                    } else {
                        EXPECT_FALSE(cs.contain<Velocity>(e));
                    }

                    if (health.current != 0 || health.max != 0) {
                        EXPECT_TRUE(cs.contain<Health>(e));
                        auto [actual_health] = cs.get<Health>(e);
                        EXPECT_EQ(actual_health, health);
                    } else {
                        EXPECT_FALSE(cs.contain<Health>(e));
                    }
                }
                break;
            }

            default:
                break;
        }
    }

    for (const auto& [e, pos, vel, health] : entities) {
        if (pos.x != 0 || pos.y != 0) {
            EXPECT_TRUE(cs.contain<Position>(e));
            auto [actual_pos] = cs.get<Position>(e);
            EXPECT_EQ(actual_pos, pos);
        }

        if (vel.vx != 0 || vel.vy != 0) {
            EXPECT_TRUE(cs.contain<Velocity>(e));
            auto [actual_vel] = cs.get<Velocity>(e);
            EXPECT_EQ(actual_vel, vel);
        }

        if (health.current != 0 || health.max != 0) {
            EXPECT_TRUE(cs.contain<Health>(e));
            auto [actual_health] = cs.get<Health>(e);
            EXPECT_EQ(actual_health, health);
        }
    }
}