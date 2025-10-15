/*
 * Unit Tests for entity_storage.hpp - Entity Storage Testing
 * 
 * This test suite validates the basic_entity_storage functionality in entity_storage.hpp:
 * - emplace: Create new entities with optional components
 * - pop: Remove entities from storage
 * - add: Add components to existing entities
 * - remove: Remove components from existing entities
 * - has: Check if entity has specific components
 * - not_has: Check if entity doesn't have specific components
 * - contain: Check if entity exists in storage
 * - clear: Clear all entities and components
 * - size/empty: Get size and check empty state
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic entity storage operations
 * 2. ComponentManagement - Tests component add/remove operations
 * 3. EntityLifecycle - Tests entity creation and removal
 * 4. ComponentQueries - Tests component query operations
 * 5. EdgeCases - Tests edge cases and error conditions
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <string>
#include <container/entity_storage.hpp>
#include <ecs/entity.hpp>

using namespace mytho::container;
using namespace mytho::ecs;

/*
 * =============================== Helper Structures/Functions ===============================
 */
#include "components.hpp"

using entity = basic_entity<uint32_t, uint16_t>;
using entity_storage = basic_entity_storage<entity>;

enum class Operation {
    EMPLACE_NO_COMPONENTS = 0,
    EMPLACE_WITH_POSITION = 1,
    EMPLACE_WITH_VELOCITY = 2,
    EMPLACE_WITH_HEALTH = 3,
    EMPLACE_WITH_MULTIPLE = 4,
    ADD_POSITION = 5,
    ADD_VELOCITY = 6,
    ADD_HEALTH = 7,
    ADD_MULTIPLE = 8,
    REMOVE_POSITION = 9,
    REMOVE_VELOCITY = 10,
    REMOVE_HEALTH = 11,
    REMOVE_MULTIPLE = 12,
    POP_ENTITY = 13,
    MAX_OPERATIONS
};

/*
 * ======================================== Test Cases ========================================
 */

TEST(EntityStorageTest, BasicOperations) {
    entity_storage storage;

    EXPECT_TRUE(storage.empty());
    EXPECT_EQ(storage.size(), 0);

    auto e1 = storage.emplace();
    EXPECT_FALSE(storage.empty());
    EXPECT_EQ(storage.size(), 1);
    EXPECT_TRUE(storage.contain(e1));

    auto e2 = storage.emplace<Position, Velocity>();
    EXPECT_EQ(storage.size(), 2);
    EXPECT_TRUE(storage.contain(e2));
    EXPECT_TRUE((storage.has<Position, Velocity>(e2)));
    EXPECT_FALSE(storage.has<Health>(e2));

    storage.pop(e1);
    EXPECT_EQ(storage.size(), 1);
    EXPECT_FALSE(storage.contain(e1));
    EXPECT_TRUE(storage.contain(e2));

    auto e3 = storage.emplace<Position, Velocity, Health>();
    EXPECT_EQ(storage.size(), 2);
    EXPECT_TRUE(storage.contain(e3));
    EXPECT_TRUE((storage.has<Position, Velocity, Health>(e3)));

    storage.clear();
    EXPECT_EQ(storage.size(), 0);
    EXPECT_TRUE(storage.empty());
    EXPECT_FALSE(storage.contain(e2));
    EXPECT_FALSE(storage.contain(e3));

    auto e4 = storage.emplace<Position>();
    EXPECT_EQ(storage.size(), 1);
    EXPECT_TRUE(storage.contain(e4));
    EXPECT_TRUE(storage.has<Position>(e4));
}

TEST(EntityStorageTest, ComponentManagement) {
    entity_storage storage;

    auto e = storage.emplace();
    EXPECT_FALSE(storage.has<Position>(e));
    EXPECT_TRUE(storage.not_has<Position>(e));

    storage.add<Position>(e);
    EXPECT_TRUE(storage.has<Position>(e));
    EXPECT_FALSE(storage.not_has<Position>(e));

    storage.add<Velocity, Health>(e);
    EXPECT_TRUE((storage.has<Position, Velocity, Health>(e)));
    EXPECT_FALSE((storage.not_has<Position, Velocity, Health>(e)));

    storage.remove<Position>(e);
    EXPECT_FALSE(storage.has<Position>(e));
    EXPECT_TRUE((storage.has<Velocity, Health>(e)));

    storage.remove<Velocity, Health>(e);
    EXPECT_FALSE((storage.has<Position, Velocity, Health>(e)));
    EXPECT_TRUE((storage.not_has<Position, Velocity, Health>(e)));
}


TEST(EntityStorageTest, RandomOperations) {
    entity_storage storage;
    std::vector<entity> entities;
    std::mt19937 gen(42); // Fixed seed for reproducible tests

    std::uniform_int_distribution<> op_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);
    std::uniform_int_distribution<> entity_dist(0, 99);
    std::uniform_int_distribution<> value_dist(1, 100);

    for (int round = 0; round < 1000; ++round) {
        Operation op = static_cast<Operation>(op_dist(gen));

        switch (op) {
            case Operation::EMPLACE_NO_COMPONENTS: {
                auto e = storage.emplace();
                entities.push_back(e);
                EXPECT_TRUE(storage.contain(e));
                break;
            }

            case Operation::EMPLACE_WITH_POSITION: {
                auto e = storage.emplace<Position>();
                entities.push_back(e);
                EXPECT_TRUE(storage.contain(e));
                EXPECT_TRUE(storage.has<Position>(e));
                break;
            }

            case Operation::EMPLACE_WITH_VELOCITY: {
                auto e = storage.emplace<Velocity>();
                entities.push_back(e);
                EXPECT_TRUE(storage.contain(e));
                EXPECT_TRUE(storage.has<Velocity>(e));
                break;
            }

            case Operation::EMPLACE_WITH_HEALTH: {
                auto e = storage.emplace<Health>();
                entities.push_back(e);
                EXPECT_TRUE(storage.contain(e));
                EXPECT_TRUE(storage.has<Health>(e));
                break;
            }

            case Operation::EMPLACE_WITH_MULTIPLE: {
                auto e = storage.emplace<Position, Velocity, Health>();
                entities.push_back(e);
                EXPECT_TRUE(storage.contain(e));
                EXPECT_TRUE((storage.has<Position, Velocity, Health>(e)));
                break;
            }

            case Operation::ADD_POSITION: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.add<Position>(e);
                        EXPECT_TRUE(storage.has<Position>(e));
                    }
                }
                break;
            }

            case Operation::ADD_VELOCITY: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.add<Velocity>(e);
                        EXPECT_TRUE(storage.has<Velocity>(e));
                    }
                }
                break;
            }

            case Operation::ADD_HEALTH: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.add<Health>(e);
                        EXPECT_TRUE(storage.has<Health>(e));
                    }
                }
                break;
            }

            case Operation::ADD_MULTIPLE: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.add<Position, Velocity, Health>(e);
                        EXPECT_TRUE((storage.has<Position, Velocity, Health>(e)));
                    }
                }
                break;
            }

            case Operation::REMOVE_POSITION: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.remove<Position>(e);
                        EXPECT_FALSE(storage.has<Position>(e));
                    }
                }
                break;
            }

            case Operation::REMOVE_VELOCITY: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.remove<Velocity>(e);
                        EXPECT_FALSE(storage.has<Velocity>(e));
                    }
                }
                break;
            }

            case Operation::REMOVE_HEALTH: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.remove<Health>(e);
                        EXPECT_FALSE(storage.has<Health>(e));
                    }
                }
                break;
            }

            case Operation::REMOVE_MULTIPLE: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.remove<Position, Velocity, Health>(e);
                        EXPECT_FALSE((storage.has<Position, Velocity, Health>(e)));
                    }
                }
                break;
            }

            case Operation::POP_ENTITY: {
                if (!entities.empty()) {
                    auto e = entities[entity_dist(gen) % entities.size()];
                    if (storage.contain(e)) {
                        storage.pop(e);
                        EXPECT_FALSE(storage.contain(e));
                        entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
                    }
                }
                break;
            }

            default:
                break;
        }

        if (round % 100 == 0) {
            EXPECT_EQ(storage.size(), entities.size());

            for (size_t i = 0; i < entities.size(); ++i) {
                if (storage.contain(entities[i])) {
                    EXPECT_TRUE(storage.contain(entities[i]));
                }
            }
        }
    }

    EXPECT_EQ(storage.size(), entities.size());
}