/*
 * Unit Tests for tick_set.hpp - Tick Set Testing
 * 
 * This test suite validates the basic_tick_set functionality in tick_set.hpp:
 * - get_added_tick/set_added_tick: Get and set added tick values by index
 * - get_changed_tick/set_changed_tick: Get and set changed tick values by index
 * - resize: Resize tick arrays with default values
 * - clear: Clear all tick data
 * - size/empty: Get size and check empty state
 * 
 * Test Cases:
 * 1. BasicOperations - Tests basic tick operations and state
 * 2. ResizeAndModify - Tests resize and modification operations
 * 3. LargeCapacity - Tests large capacity and performance
 * 4. EdgeCases - Tests edge cases and boundary conditions
 * 5. RandomOperations - Tests random operations and data integrity
 */

#include <gtest/gtest.h>
#include <container/tick_set.hpp>
#include <random>
#include <vector>

using namespace mytho::container;

/*
 * ======================================== Test Cases ========================================
 */

// Test basic tick operations and state
TEST(TickSetTest, BasicOperations) {
    basic_tick_set tick_set;

    EXPECT_EQ(tick_set.size(), 0);
    EXPECT_TRUE(tick_set.empty());

    tick_set.resize(3, 10);
    EXPECT_EQ(tick_set.size(), 3);
    EXPECT_FALSE(tick_set.empty());

    EXPECT_EQ(tick_set.get_added_tick(0), 10);
    EXPECT_EQ(tick_set.get_changed_tick(1), 10);

    tick_set.set_added_tick(0, 20);
    tick_set.set_changed_tick(1, 30);
    EXPECT_EQ(tick_set.get_added_tick(0), 20);
    EXPECT_EQ(tick_set.get_changed_tick(1), 30);

    tick_set.get_added_tick(2) = 40;
    tick_set.get_changed_tick(2) = 50;
    EXPECT_EQ(tick_set.get_added_tick(2), 40);
    EXPECT_EQ(tick_set.get_changed_tick(2), 50);

    tick_set.clear();
    EXPECT_EQ(tick_set.size(), 0);
    EXPECT_TRUE(tick_set.empty());
}

// Test resize and modification operations
TEST(TickSetTest, ResizeAndModify) {
    basic_tick_set tick_set;

    tick_set.resize(2, 5);
    tick_set.set_added_tick(0, 10);
    tick_set.set_changed_tick(1, 15);

    EXPECT_EQ(tick_set.size(), 2);
    EXPECT_EQ(tick_set.get_added_tick(0), 10);
    EXPECT_EQ(tick_set.get_changed_tick(1), 15);

    tick_set.resize(4, 20);
    EXPECT_EQ(tick_set.size(), 4);
    EXPECT_EQ(tick_set.get_added_tick(0), 10);  // Preserved
    EXPECT_EQ(tick_set.get_changed_tick(1), 15); // Preserved
    EXPECT_EQ(tick_set.get_added_tick(2), 20);  // New value
    EXPECT_EQ(tick_set.get_changed_tick(2), 20); // New value

    tick_set.resize(1, 25);
    EXPECT_EQ(tick_set.size(), 1);
    EXPECT_EQ(tick_set.get_added_tick(0), 10);  // Preserved
    EXPECT_EQ(tick_set.get_changed_tick(0), 5); // Preserved

    tick_set.resize(6, 30);
    tick_set.resize(3, 40);
    EXPECT_EQ(tick_set.size(), 3);
    EXPECT_EQ(tick_set.get_added_tick(0), 10);  // Still preserved
    EXPECT_EQ(tick_set.get_changed_tick(0), 5); // Still preserved
}

// Test large capacity and performance
TEST(TickSetTest, LargeCapacity) {
    basic_tick_set tick_set;

    tick_set.resize(1000, 42);
    EXPECT_EQ(tick_set.size(), 1000);
    for (size_t i = 0; i < 1000; ++i) {
        EXPECT_EQ(tick_set.get_added_tick(i), 42);
        EXPECT_EQ(tick_set.get_changed_tick(i), 42);
    }

    tick_set.set_added_tick(0, 100);
    tick_set.set_changed_tick(500, 200);
    tick_set.set_added_tick(999, 300);
    tick_set.set_changed_tick(999, 400);
    EXPECT_EQ(tick_set.get_added_tick(0), 100);
    EXPECT_EQ(tick_set.get_changed_tick(500), 200);
    EXPECT_EQ(tick_set.get_added_tick(999), 300);
    EXPECT_EQ(tick_set.get_changed_tick(999), 400);

    tick_set.get_added_tick(100) = 500;
    tick_set.get_changed_tick(200) = 600;
    EXPECT_EQ(tick_set.get_added_tick(100), 500);
    EXPECT_EQ(tick_set.get_changed_tick(200), 600);
}

// Test edge cases and boundary conditions
TEST(TickSetTest, EdgeCases) {
    basic_tick_set tick_set;

    tick_set.resize(0, 10);
    EXPECT_EQ(tick_set.size(), 0);
    EXPECT_TRUE(tick_set.empty());

    tick_set.resize(1, 5);
    EXPECT_EQ(tick_set.size(), 1);
    EXPECT_FALSE(tick_set.empty());
    EXPECT_EQ(tick_set.get_added_tick(0), 5);
    EXPECT_EQ(tick_set.get_changed_tick(0), 5);

    tick_set.clear();
    EXPECT_EQ(tick_set.size(), 0);
    EXPECT_TRUE(tick_set.empty());

    tick_set.resize(2, 20);
    EXPECT_EQ(tick_set.size(), 2);
    EXPECT_FALSE(tick_set.empty());

    const auto& const_tick_set = tick_set;
    EXPECT_EQ(const_tick_set.size(), 2);
    EXPECT_EQ(const_tick_set.get_added_tick(0), 20);
    EXPECT_EQ(const_tick_set.get_changed_tick(1), 20);
    EXPECT_FALSE(const_tick_set.empty());
}

// Test random operations and data integrity
TEST(TickSetTest, RandomOperations) {
    basic_tick_set tick_set;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> size_dist(1, 100);
    std::uniform_int_distribution<uint64_t> value_dist(0, 10000);
    std::uniform_int_distribution<int> operation_dist(0, 3);

    for (int round = 0; round < 100; ++round) {
        int operation = operation_dist(gen);

        switch (operation) {
            case 0: { // Resize operation
                size_t old_size = tick_set.size();
                size_t new_size = size_dist(gen);
                uint64_t default_value = value_dist(gen);
                tick_set.resize(new_size, default_value);

                EXPECT_EQ(tick_set.size(), new_size);
                if (new_size > 0) {
                    EXPECT_FALSE(tick_set.empty());
                    for (size_t i = old_size; i < new_size; ++i) {
                        EXPECT_EQ(tick_set.get_added_tick(old_size), default_value);
                        EXPECT_EQ(tick_set.get_changed_tick(old_size), default_value);
                    }
                } else {
                    EXPECT_TRUE(tick_set.empty());
                }
                break;
            }
            case 1: { // Set operations
                if (tick_set.size() > 0) {
                    size_t index = size_dist(gen) % tick_set.size();
                    uint64_t added_value = value_dist(gen);
                    uint64_t changed_value = value_dist(gen);

                    tick_set.set_added_tick(index, added_value);
                    tick_set.set_changed_tick(index, changed_value);

                    EXPECT_EQ(tick_set.get_added_tick(index), added_value);
                    EXPECT_EQ(tick_set.get_changed_tick(index), changed_value);
                }
                break;
            }
            case 2: { // Reference modification
                if (tick_set.size() > 0) {
                    size_t index = size_dist(gen) % tick_set.size();
                    uint64_t added_value = value_dist(gen);
                    uint64_t changed_value = value_dist(gen);

                    tick_set.get_added_tick(index) = added_value;
                    tick_set.get_changed_tick(index) = changed_value;

                    EXPECT_EQ(tick_set.get_added_tick(index), added_value);
                    EXPECT_EQ(tick_set.get_changed_tick(index), changed_value);
                }
                break;
            }
            case 3: { // Clear operation
                tick_set.clear();
                EXPECT_EQ(tick_set.size(), 0);
                EXPECT_TRUE(tick_set.empty());
                break;
            }
        }
    }

    if (tick_set.size() > 0) {
        EXPECT_FALSE(tick_set.empty());

        for (size_t i = 0; i < tick_set.size(); ++i) {
            uint64_t added = tick_set.get_added_tick(i);
            uint64_t changed = tick_set.get_changed_tick(i);

            EXPECT_GE(added, 0);
            EXPECT_GE(changed, 0);
        }
    }
}