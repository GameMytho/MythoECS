/*
 * Unit Tests for sparse_set.hpp - Sparse Set Testing
 * 
 * This test suite validates the basic_sparse_set functionality in sparse_set.hpp:
 * - add: Add data to sparse set
 * - remove: Remove data from sparse set
 * - contain: Check if data exists in sparse set
 * - index: Get index of data in dense array
 * - data: Get data by index
 * - swap: Swap positions of two data elements
 * - clear: Clear all data
 * - size/empty: Get size and check empty state
 * - iterators: Test iterator functionality
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic operations
 * 2. SwapAndIndex - Tests swap and index operations
 * 3. LargeCapacity - Tests large capacity and performance
 * 4. IteratorOperations - Tests iterator functionality
 * 5. ConstIteratorOperations - Tests const iterator functionality
 * 6. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <algorithm>
#include <container/sparse_set.hpp>

using namespace mytho::container;

/*
 * =============================== Helper Structures/Functions ===============================
 */

enum class Operation {
    ADD = 0,
    REMOVE = 1,
    SWAP = 2,
    MAX_OPERATIONS
};

/*
 * ======================================== Test Cases ========================================
 */

// Test basic operations
TEST(SparseSetTest, BasicOperations) {
    basic_sparse_set<uint32_t> sparse_set;

    EXPECT_EQ(sparse_set.size(), 0);
    EXPECT_TRUE(sparse_set.empty());

    sparse_set.add(10);
    sparse_set.add(20);
    sparse_set.add(30);
    sparse_set.add(40);

    EXPECT_EQ(sparse_set.size(), 4);
    EXPECT_FALSE(sparse_set.empty());

    EXPECT_TRUE(sparse_set.contain(10));
    EXPECT_TRUE(sparse_set.contain(20));
    EXPECT_TRUE(sparse_set.contain(30));
    EXPECT_TRUE(sparse_set.contain(40));
    EXPECT_FALSE(sparse_set.contain(50));

    EXPECT_EQ(sparse_set.index(10), 0);
    EXPECT_EQ(sparse_set.index(20), 1);
    EXPECT_EQ(sparse_set.index(30), 2);
    EXPECT_EQ(sparse_set.index(40), 3);

    EXPECT_EQ(sparse_set.data(0), 10);
    EXPECT_EQ(sparse_set.data(1), 20);
    EXPECT_EQ(sparse_set.data(2), 30);
    EXPECT_EQ(sparse_set.data(3), 40);

    sparse_set.remove(20);
    EXPECT_EQ(sparse_set.size(), 3);
    EXPECT_FALSE(sparse_set.contain(20));
    EXPECT_TRUE(sparse_set.contain(10));
    EXPECT_TRUE(sparse_set.contain(40));
    EXPECT_TRUE(sparse_set.contain(30));

    EXPECT_EQ(sparse_set.index(10), 0);
    EXPECT_EQ(sparse_set.index(40), 1);
    EXPECT_EQ(sparse_set.index(30), 2);

    sparse_set.remove(40);
    EXPECT_EQ(sparse_set.size(), 2);
    EXPECT_FALSE(sparse_set.contain(40));
    EXPECT_TRUE(sparse_set.contain(10));
    EXPECT_TRUE(sparse_set.contain(30));

    sparse_set.remove(10);
    EXPECT_EQ(sparse_set.size(), 1);
    EXPECT_FALSE(sparse_set.contain(10));
    EXPECT_TRUE(sparse_set.contain(30));
    EXPECT_EQ(sparse_set.index(30), 0);

    sparse_set.remove(30);
    EXPECT_EQ(sparse_set.size(), 0);
    EXPECT_TRUE(sparse_set.empty());
    EXPECT_FALSE(sparse_set.contain(30));

    sparse_set.add(100);
    sparse_set.add(200);
    EXPECT_EQ(sparse_set.size(), 2);
    
    sparse_set.clear();
    EXPECT_EQ(sparse_set.size(), 0);
    EXPECT_TRUE(sparse_set.empty());
}

// Test swap and index operations
TEST(SparseSetTest, SwapAndIndex) {
    basic_sparse_set<uint32_t> sparse_set;

    sparse_set.add(10);
    sparse_set.add(20);
    sparse_set.add(30);

    EXPECT_EQ(sparse_set.index(10), 0);
    EXPECT_EQ(sparse_set.index(20), 1);
    EXPECT_EQ(sparse_set.index(30), 2);

    sparse_set.swap(10, 30);
    EXPECT_EQ(sparse_set.index(10), 2);
    EXPECT_EQ(sparse_set.index(20), 1);
    EXPECT_EQ(sparse_set.index(30), 0);

    EXPECT_EQ(sparse_set.data(0), 30);
    EXPECT_EQ(sparse_set.data(1), 20);
    EXPECT_EQ(sparse_set.data(2), 10);

    sparse_set.swap(20, 20);
    EXPECT_EQ(sparse_set.index(20), 1);
    EXPECT_EQ(sparse_set.data(1), 20);

    sparse_set.swap(30, 20);
    EXPECT_EQ(sparse_set.index(30), 1);
    EXPECT_EQ(sparse_set.index(20), 0);
    EXPECT_EQ(sparse_set.data(0), 20);
    EXPECT_EQ(sparse_set.data(1), 30);
}

// Test large capacity and performance
TEST(SparseSetTest, LargeCapacity) {
    basic_sparse_set<uint32_t> sparse_set;

    for (uint32_t i = 0; i < 1000; ++i) {
        sparse_set.add(i);
    }

    EXPECT_EQ(sparse_set.size(), 1000);

    for (uint32_t i = 0; i < 1000; ++i) {
        EXPECT_TRUE(sparse_set.contain(i));
        EXPECT_EQ(sparse_set.index(i), i);
        EXPECT_EQ(sparse_set.data(i), i);
    }

    sparse_set.remove(0);
    sparse_set.remove(500);
    sparse_set.remove(999);

    EXPECT_EQ(sparse_set.size(), 997);
    EXPECT_FALSE(sparse_set.contain(0));
    EXPECT_FALSE(sparse_set.contain(500));
    EXPECT_FALSE(sparse_set.contain(999));

    EXPECT_TRUE(sparse_set.contain(1));
    EXPECT_TRUE(sparse_set.contain(501));
    EXPECT_TRUE(sparse_set.contain(998));
}

// Test iterator functionality
TEST(SparseSetTest, IteratorOperations) {
    basic_sparse_set<uint32_t> sparse_set;
    EXPECT_EQ(sparse_set.begin(), sparse_set.end());

    sparse_set.add(10);
    sparse_set.add(20);
    sparse_set.add(30);

    auto it = sparse_set.begin();
    EXPECT_EQ(*it, 10);
    ++it;
    EXPECT_EQ(*it, 20);
    ++it;
    EXPECT_EQ(*it, 30);
    ++it;
    EXPECT_EQ(it, sparse_set.end());

    it = sparse_set.begin();
    it += 2;
    EXPECT_EQ(*it, 30);
    it -= 1;
    EXPECT_EQ(*it, 20);

    auto it1 = sparse_set.begin();
    auto it2 = sparse_set.begin() + 1;
    auto it3 = sparse_set.begin() + 1;
    EXPECT_TRUE(it1 < it2);
    EXPECT_TRUE(it2 > it1);
    EXPECT_TRUE(it2 == it3);
    EXPECT_TRUE(it1 != it2);
    EXPECT_EQ(it2 - it1, 1);
    EXPECT_EQ(sparse_set.begin()[0], 10);
    EXPECT_EQ(sparse_set.begin()[1], 20);
    EXPECT_EQ(sparse_set.begin()[2], 30);

    std::vector<uint32_t> elements;
    for (const auto& element : sparse_set) {
        elements.push_back(element);
    }
    EXPECT_EQ(elements.size(), 3);
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 10) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 20) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 30) != elements.end());

    sparse_set.remove(20);
    elements.clear();
    for (const auto& element : sparse_set) {
        elements.push_back(element);
    }

    EXPECT_EQ(elements.size(), 2);
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 10) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 30) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 20) == elements.end());
}

// Test const iterator functionality
TEST(SparseSetTest, ConstIteratorOperations) {
    basic_sparse_set<uint32_t> sparse_set;

    sparse_set.add(10);
    sparse_set.add(20);
    sparse_set.add(30);

    const auto& const_sparse_set = sparse_set;

    auto const_it = const_sparse_set.begin();
    EXPECT_EQ(*const_it, 10);
    ++const_it;
    EXPECT_EQ(*const_it, 20);
    ++const_it;
    EXPECT_EQ(*const_it, 30);
    ++const_it;
    EXPECT_EQ(const_it, const_sparse_set.end());

    const_it = const_sparse_set.begin();
    const_it += 2;
    EXPECT_EQ(*const_it, 30);
    const_it -= 1;
    EXPECT_EQ(*const_it, 20);

    auto const_it1 = const_sparse_set.begin();
    auto const_it2 = const_sparse_set.begin() + 1;
    auto const_it3 = const_sparse_set.begin() + 1;
    EXPECT_TRUE(const_it1 < const_it2);
    EXPECT_TRUE(const_it2 > const_it1);
    EXPECT_TRUE(const_it2 == const_it3);
    EXPECT_TRUE(const_it1 != const_it2);
    EXPECT_EQ(const_it2 - const_it1, 1);

    EXPECT_EQ(const_sparse_set.begin()[0], 10);
    EXPECT_EQ(const_sparse_set.begin()[1], 20);
    EXPECT_EQ(const_sparse_set.begin()[2], 30);

    std::vector<uint32_t> elements;
    for (const auto& element : const_sparse_set) {
        elements.push_back(element);
    }

    EXPECT_EQ(elements.size(), 3);
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 10) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 20) != elements.end());
    EXPECT_TRUE(std::find(elements.begin(), elements.end(), 30) != elements.end());

    basic_sparse_set<uint32_t> empty_sparse_set;
    const auto& const_empty_sparse_set = empty_sparse_set;
    EXPECT_EQ(const_empty_sparse_set.begin(), const_empty_sparse_set.end());
}

// Test random operations and data integrity
TEST(SparseSetTest, RandomOperations) {
    basic_sparse_set<uint32_t> sparse_set;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> value_dist(1, 1000);
    std::uniform_int_distribution<int> operation_dist(0, static_cast<int>(Operation::MAX_OPERATIONS) - 1);
    std::vector<uint32_t> added_values;

    for (int round = 0; round < 200; ++round) {
        Operation operation = static_cast<Operation>(operation_dist(gen));

        switch (operation) {
            case Operation::ADD: { // Add operation
                uint32_t value = value_dist(gen);
                if (!sparse_set.contain(value)) {
                    sparse_set.add(value);
                    added_values.push_back(value);
                }
                break;
            }

            case Operation::REMOVE: { // Remove operation
                if (!added_values.empty()) {
                    size_t index = value_dist(gen) % added_values.size();
                    uint32_t value = added_values[index];

                    if (sparse_set.contain(value)) {
                        sparse_set.remove(value);
                        added_values.erase(added_values.begin() + index);
                    }
                }
                break;
            }

            case Operation::SWAP: { // Swap operation
                if (added_values.size() >= 2) {
                    size_t index1 = value_dist(gen) % added_values.size();
                    size_t index2 = value_dist(gen) % added_values.size();
                    uint32_t value1 = added_values[index1];
                    uint32_t value2 = added_values[index2];

                    if (sparse_set.contain(value1) && sparse_set.contain(value2)) {
                        sparse_set.swap(value1, value2);
                    }
                }
                break;
            }

            default:
                break;
        }

        EXPECT_EQ(sparse_set.size(), added_values.size());

        for (uint32_t value : added_values) {
            EXPECT_TRUE(sparse_set.contain(value));
        }
    }

    EXPECT_EQ(sparse_set.size(), added_values.size());

    for (size_t i = 0; i < added_values.size(); ++i) {
        uint32_t value = added_values[i];
        EXPECT_TRUE(sparse_set.contain(value));

        size_t index = sparse_set.index(value);
        EXPECT_LT(index, sparse_set.size());
        EXPECT_EQ(sparse_set.data(index), value);
    }
}
