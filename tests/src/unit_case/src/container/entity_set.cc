/*
 * Unit Tests for es.hpp - Entity Set Testing
 * 
 * This test suite validates the basic_entity_set functionality in es.hpp:
 * - add: Add entities to the set
 * - remove: Remove entities from the set
 * - contain: Check if entity exists with correct version
 * - swap: Swap positions of two entities
 * - index: Get index of entity in dense array
 * - entity: Get entity by index
 * - clear: Clear all entities
 * - size/empty: Get size and check empty state
 * - iterators: Test iterator functionality for entities
 * - version_next: Test version increment functionality
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic entity operations and lifecycle
 * 2. SwapAndIndex - Tests swap and index operations
 * 3. IteratorOperations - Tests iterator functionality
 * 4. ConstIteratorOperations - Tests const iterator functionality
 * 5. EntityVersioning - Tests entity version management
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <container/entity_set.hpp>
#include <ecs/entity.hpp>

using namespace mytho::container;
using namespace mytho::ecs;

/*
 * =============================== Helper Structures/Functions ===============================
 */

using entity = basic_entity<uint32_t, uint16_t>;
using entity_set = basic_entity_set<entity>;

// Test helper to expose protected version_next for testing
struct test_entity_set : entity_set {
    using entity_set::version_next;
};

enum class Operation {
    ADD = 0,
    REMOVE = 1,
    VERSION_INC = 2,
    SWAP = 3,
    VERIFY = 4,
    MAX_OPERATIONS
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic entity operations and lifecycle
TEST(EntitySetTest, BasicOperations) {
    entity_set es;

    EXPECT_EQ(es.size(), 0);
    EXPECT_TRUE(es.empty());

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    es.add(e1);
    es.add(e2);
    es.add(e3);

    EXPECT_EQ(es.size(), 3);
    EXPECT_FALSE(es.empty());

    EXPECT_TRUE(es.contain(e1));
    EXPECT_TRUE(es.contain(e2));
    EXPECT_TRUE(es.contain(e3));

    entity e1_different_version(1, 11);
    EXPECT_FALSE(es.contain(e1_different_version));

    EXPECT_EQ(es.index(e1), 0);
    EXPECT_EQ(es.index(e2), 1);
    EXPECT_EQ(es.index(e3), 2);

    EXPECT_EQ(es.entity(0), e1);
    EXPECT_EQ(es.entity(1), e2);
    EXPECT_EQ(es.entity(2), e3);

    es.remove(e2);
    EXPECT_EQ(es.size(), 2);
    EXPECT_FALSE(es.contain(e2));
    EXPECT_TRUE(es.contain(e1));
    EXPECT_TRUE(es.contain(e3));

    EXPECT_EQ(es.index(e1), 0);
    EXPECT_EQ(es.index(e3), 1);

    es.clear();
    EXPECT_EQ(es.size(), 0);
    EXPECT_TRUE(es.empty());
    EXPECT_FALSE(es.contain(e1));
    EXPECT_FALSE(es.contain(e3));
}

// Test swap and index operations
TEST(EntitySetTest, SwapAndIndex) {
    entity_set es;

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    es.add(e1);
    es.add(e2);
    es.add(e3);

    EXPECT_EQ(es.index(e1), 0);
    EXPECT_EQ(es.index(e2), 1);
    EXPECT_EQ(es.index(e3), 2);

    es.swap(e1, e3);
    EXPECT_EQ(es.index(e1), 2);
    EXPECT_EQ(es.index(e2), 1);
    EXPECT_EQ(es.index(e3), 0);

    EXPECT_EQ(es.entity(0), e3);
    EXPECT_EQ(es.entity(1), e2);
    EXPECT_EQ(es.entity(2), e1);

    es.swap(e2, e2);
    EXPECT_EQ(es.index(e2), 1);
    EXPECT_EQ(es.entity(1), e2);

    es.swap(e3, e2);
    EXPECT_EQ(es.index(e2), 0);
    EXPECT_EQ(es.index(e3), 1);
    EXPECT_EQ(es.entity(0), e2);
    EXPECT_EQ(es.entity(1), e3);
}

// Test iterator functionality
TEST(EntitySetTest, IteratorOperations) {
    entity_set es;

    EXPECT_EQ(es.begin(), es.end());

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    es.add(e1);
    es.add(e2);
    es.add(e3);

    auto it = es.begin();
    EXPECT_EQ(*it, e1);
    ++it;
    EXPECT_EQ(*it, e2);
    ++it;
    EXPECT_EQ(*it, e3);
    ++it;
    EXPECT_EQ(it, es.end());

    it = es.begin();
    it += 2;
    EXPECT_EQ(*it, e3);
    it -= 1;
    EXPECT_EQ(*it, e2);

    auto it1 = es.begin();
    auto it2 = es.begin() + 1;
    auto it3 = es.begin() + 1;
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it2 == it3);
    EXPECT_TRUE(it1 != it2);
    EXPECT_EQ(it2 - it1, 1);

    EXPECT_EQ(es.begin()[0], e1);
    EXPECT_EQ(es.begin()[1], e2);
    EXPECT_EQ(es.begin()[2], e3);

    std::vector<entity> entities;
    for (const auto& e : es) {
        entities.push_back(e);
    }

    EXPECT_EQ(entities.size(), 3);
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e1) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e2) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e3) != entities.end());

    es.remove(e2);
    entities.clear();
    for (const auto& e : es) {
        entities.push_back(e);
    }

    EXPECT_EQ(entities.size(), 2);
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e1) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e3) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e2) == entities.end());
}

// Test const iterator functionality
TEST(EntitySetTest, ConstIteratorOperations) {
    entity_set es;

    entity e1(1, 10);
    entity e2(2, 20);
    entity e3(3, 30);

    es.add(e1);
    es.add(e2);
    es.add(e3);

    const auto& const_es = es;

    auto const_it = const_es.begin();
    EXPECT_EQ(*const_it, e1);
    ++const_it;
    EXPECT_EQ(*const_it, e2);
    ++const_it;
    EXPECT_EQ(*const_it, e3);
    ++const_it;
    EXPECT_EQ(const_it, const_es.end());

    const_it = const_es.begin();
    const_it += 2;
    EXPECT_EQ(*const_it, e3);
    const_it -= 1;
    EXPECT_EQ(*const_it, e2);

    auto const_it1 = const_es.begin();
    auto const_it2 = const_es.begin() + 1;
    auto const_it3 = const_es.begin() + 1;
    EXPECT_TRUE(const_it1 < const_it2);
    EXPECT_TRUE(const_it2 > const_it1);
    EXPECT_TRUE(const_it2 == const_it3);
    EXPECT_TRUE(const_it1 != const_it2);
    EXPECT_EQ(const_it2 - const_it1, 1);

    EXPECT_EQ(const_es.begin()[0], e1);
    EXPECT_EQ(const_es.begin()[1], e2);
    EXPECT_EQ(const_es.begin()[2], e3);

    std::vector<entity> entities;
    for (const auto& e : const_es) {
        entities.push_back(e);
    }

    EXPECT_EQ(entities.size(), 3);
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e1) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e2) != entities.end());
    EXPECT_TRUE(std::find(entities.begin(), entities.end(), e3) != entities.end());

    entity_set empty_entity_set;
    const auto& const_empty_entity_set = empty_entity_set;
    EXPECT_EQ(const_empty_entity_set.begin(), const_empty_entity_set.end());
}

// Test entity version management and version_next functionality
TEST(EntitySetTest, EntityVersioning) {
    test_entity_set es;

    entity e1(1, 10);
    entity e2(2, 20);

    es.add(e1);
    es.add(e2);

    es.version_next(e1);

    entity e1_updated(1, 11);
    EXPECT_TRUE(es.contain(e1_updated));
    EXPECT_FALSE(es.contain(e1)); // Old version should not be contained

    es.version_next(e1_updated);
    entity e1_updated2(1, 12);
    EXPECT_TRUE(es.contain(e1_updated2));
    EXPECT_FALSE(es.contain(e1_updated));

    es.version_next(e2);
    entity e2_updated(2, 21);
    EXPECT_TRUE(es.contain(e2_updated));
    EXPECT_FALSE(es.contain(e2));

    EXPECT_TRUE(es.contain(e1_updated2));
    EXPECT_TRUE(es.contain(e2_updated));
    EXPECT_EQ(es.size(), 2);
}

// Test random operations and data integrity
TEST(EntitySetTest, RandomOperations) {
    test_entity_set es;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> id_dist(1, 1000);
    std::uniform_int_distribution<size_t> size_dist(1, 50);
    std::uniform_int_distribution<int> operation_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);

    std::vector<entity> entities;
    entities.reserve(100); // Reserve space to avoid reallocation

    for (int round = 0; round < 100; ++round) {
        Operation operation = static_cast<Operation>(operation_dist(gen));
        size_t num_entities = size_dist(gen);

        switch (operation) {
            case Operation::ADD: { // Add operation
                for (size_t i = 0; i < num_entities; ++i) {
                    uint32_t id = id_dist(gen);
                    entity e(id, 0);

                    if (std::find_if(entities.begin(), entities.end(), [&](const entity& e) { return e.id() == id; }) == entities.end()) {
                        es.add(e);
                        entities.push_back(e);
                    }
                }
                break;
            }

            case Operation::REMOVE: { // Remove operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    entity entity_to_remove = entities[random_index];
                    es.remove(entity_to_remove);

                    std::swap(entities[random_index], entities.back());
                    entities.pop_back();
                }
                break;
            }

            case Operation::VERSION_INC: { // Version increment operation
                if (!entities.empty()) {
                    size_t random_index = gen() % entities.size();
                    entity entity_to_increment = entities[random_index];
                    es.version_next(entity_to_increment);

                    entity updated_entity(entity_to_increment.id(), entity_to_increment.version() + 1);
                    entities[random_index] = updated_entity;
                }
                break;
            }

            case Operation::SWAP: { // Swap operation
                if (entities.size() >= 2) {
                    size_t index1 = gen() % entities.size();
                    size_t index2 = gen() % entities.size();
                    if (index1 != index2) {
                        es.swap(entities[index1], entities[index2]);
                        std::swap(entities[index1], entities[index2]);
                    }
                }
                break;
            }

            case Operation::VERIFY: { // Verification operation
                EXPECT_EQ(es.size(), entities.size());
                
                for (const auto& e : entities) {
                    EXPECT_TRUE(es.contain(e));
                }

                std::vector<entity> iterator_entities;
                for (const auto& e : es) {
                    iterator_entities.push_back(e);
                }
                EXPECT_EQ(iterator_entities.size(), entities.size());

                for (size_t i = 0; i < entities.size(); ++i) {
                    EXPECT_EQ(es.entity(i), entities[i]);
                }
                break;
            }

            default:
                break;
        }
    }

    EXPECT_EQ(es.size(), entities.size());
    for (const auto& e : entities) {
        EXPECT_TRUE(es.contain(e));
    }
}
